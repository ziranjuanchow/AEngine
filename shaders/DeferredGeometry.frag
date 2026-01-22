#version 450 core

layout (location = 0) out vec4 gAlbedoRough;
layout (location = 1) out vec4 gNormalMetal;
layout (location = 2) out vec4 gEmissiveAO;

layout (location = 0) in vec3 WorldPos;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoords;
layout (location = 3) in mat3 TBN;

// Material parameters (Must match StandardPBR layout for compatibility)
layout (location = 20) uniform vec3 albedo;
layout (location = 21) uniform float metallic;
layout (location = 22) uniform float roughness;
layout (location = 23) uniform float ao;

// Texture parameters
layout (location = 30) uniform sampler2D albedoMap;
layout (location = 31) uniform sampler2D normalMap;
layout (location = 32) uniform int useAlbedoMap;
layout (location = 33) uniform int useNormalMap;

void main()
{
    // Albedo
    vec3 baseColor = albedo;
    if (useAlbedoMap == 1) {
        baseColor = texture(albedoMap, TexCoords).rgb;
        baseColor = pow(baseColor, vec3(2.2)); // sRGB to Linear
    }

    // Normal
    vec3 N = normalize(Normal);
    if (useNormalMap == 1) {
        vec3 normalMapValue = texture(normalMap, TexCoords).rgb;
        normalMapValue = normalMapValue * 2.0 - 1.0;
        N = normalize(TBN * normalMapValue);
    }

    // Roughness & Metallic (Assume uniform for now, extend to map if needed)
    float r = roughness;
    float m = metallic;

    // Pack G-Buffer
    gAlbedoRough = vec4(baseColor, r);
    gNormalMetal = vec4(N, m);
    gEmissiveAO  = vec4(0.0, 0.0, 0.0, ao); // Emissive placeholder
}
