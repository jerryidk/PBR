#version 330 core
/* PBS: 
 * Few key ingredients:
 *
 *      Microfacet model: 
 *      h (halfway vector) = norm(l + v)
 *      h represents the surface normals at microscopic level (in some sense).
 *      The more actual surface normal aligns with h, the less rough it is, thus we get 
 *      higher specular.
 *
 *      Energy conservation:
 *      Light hit surfaces. Light either get refracted (kd) or reflected (ks). Some of the refracted 
 *      lights get absorbed. For metallic objects, all refracted light gets absorbed. Thus
 *      ks + kd = 1  where ks = fresenel 
 *       
 *      Rendering Equation:
 *      L(p, w_o) = sum_i_to_inf (brdf * L_i * n * w_i) over dw_i  
 *      
 *      BRDF:
 *      Cook-Torrance BRDF: 
 *      brdf = kd * f_lambert + ks * f_cooktorrance
 *      
 *      ks = F
 *      kd = 1 - ks
 *      f_lambert = c / pi where c is alberto/surface color
 *      f_cooktorrance = DG / (4 * w_i * n * w_o * n)
 *      D is Distribution function - This approximate how many h is aligned with surface normal on micro level  
 *      F is Fresenel equation - This approximate ratio of reflected light over refracted light depending on view angle - ks
 *      G is Geometric function - This approximate how many micro surfaces are shadowed by other micro surfaces 
 */
out vec4 FragColor;

in vec3 Normal;
in vec3 WorldPos;
in vec4 PosInLight;

// material parameters
uniform vec3  albedo;    // diffuse/surface color
uniform float metallic;  // 0.0 - 1.0 higher means higher ks less kd.
uniform float roughness; // 0.0 - 1.0
uniform float ao;        // ambient occusion
uniform bool gammacorrect;

// lights
uniform vec3 lightPosition;
uniform vec3 lightColor;

uniform vec3 camPos;
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfMap;
uniform sampler2DShadow shadowMap;

const float PI = 3.14159265359;
  
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness); 

void main()
{		
    vec3 N = normalize(Normal);
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	           
    // direct lighting
    //------------------------
    //------------------------
    vec3 Lo = vec3(0.0);

    vec3 L = normalize(lightPosition - WorldPos);
    vec3 H = normalize(V + L);
    float distance    = length(lightPosition - WorldPos);
    float attenuation = 1.0 / pow(distance,2);
    vec3 radiance     = lightColor * attenuation;        
        
    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, roughness);        
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
        
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;  
    float NdotL = max(dot(N, L), 0.0);           
    Lo = (kD * albedo / PI + specular) * radiance * NdotL; 
     
    //indirect lighting (IBL)
    //------------------------
    //------------------------
    F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    kS = F;
    kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse    = irradiance * albedo;
  
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;   
    vec2 envBRDF  = texture(brdfMap, vec2(max(dot(N, V), 0.0), roughness)).rg;
    specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    vec3 ambient = (kD * diffuse + specular) * ao; 
    float shadow = textureProj(shadowMap, PosInLight);     
	vec3 color = ambient + shadow * Lo;

    //reinhard tone mapping
    color = color / (color + vec3(1.0));
    //gamma correction
    if(gammacorrect)
        color = pow(color, vec3(1.0/2.2));  
   
    FragColor = vec4(color, 1.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness; // some adoption
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0; // some adoption

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   