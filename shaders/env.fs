#version 330 core

//Sample the env cube map generated from hdr image

out vec4 FragColor;

in vec3 Pos;
uniform samplerCube envMap;
uniform bool gammacorrect;
void main()
{
    vec3 envColor = texture(envMap, Pos).rgb;
    //hdr -> lr
    envColor = envColor / (envColor + vec3(1.0));
    //gamma correction
    if(gammacorrect)
        envColor = pow(envColor, vec3(1.0/2.2)); 

    FragColor = vec4(envColor, 1.0);
}