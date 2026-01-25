#version 450 core
out vec4 FragColor;
layout (location = 0) in vec2 TexCoords;

layout (location = 0) uniform sampler2D sceneTexture;
layout (location = 1) uniform float exposure;

void main()
{
    const float gamma = 2.2;
    vec3 hdrColor = texture(sceneTexture, TexCoords).rgb;
    
    // Exposure
    vec3 mapped = hdrColor * exposure;
    
    // ACES Tone Mapping
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    mapped = clamp((mapped*(a*mapped+b))/(mapped*(c*mapped+d)+e), 0.0, 1.0);
    
    // Gamma correction
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);
}
