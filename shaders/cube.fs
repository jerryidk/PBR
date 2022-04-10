#version 330 core
//Convert from hdr image to env cube map

out vec4 FragColor;
in vec3 Pos;

uniform sampler2D textureMap;

const float PI = 3.14159265359;
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.x, v.z)/(2 * PI), v.y * 0.5);
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(Pos)); // make sure to normalize localPos
    vec3 color = texture(textureMap, uv).rgb;
    FragColor = vec4(color, 1.0);
}