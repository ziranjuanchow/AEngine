#version 450 core
// --- Deferred Geometry Pass Fragment Shader ---
// Purpose: Outputs material properties to the G-Buffer (Multiple Render Targets).

// G-Buffer Outputs
// Location 0: Albedo (RGB) + Roughness (A) - [RGBA8]
layout (location = 0) out vec4 gAlbedoRough;
// Location 1: Normal (RGB) + Metallic (A) - [RGBA16F for precision]
layout (location = 1) out vec4 gNormalMetal;
// Location 2: Emissive (RGB) + AO (A) - [RGBA8]
layout (location = 2) out vec4 gEmissiveAO;

// Inputs from Vertex Shader
layout (location = 0) in vec3 WorldPos;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoords;
layout (location = 3) in mat3 TBN;

// Material Parameters
// Note: Locations match FStandardPBRMaterial logic
layout (location = 20) uniform vec3 albedo;
layout (location = 21) uniform float metallic;
layout (location = 22) uniform float roughness;
layout (location = 23) uniform float ao;

// Texture Samplers
layout (location = 30) uniform sampler2D albedoMap;
layout (location = 31) uniform sampler2D normalMap;
layout (location = 32) uniform int useAlbedoMap;
layout (location = 33) uniform int useNormalMap;

void main()
{
    // 1. Calculate Albedo
    vec3 baseColor = albedo;
    if (useAlbedoMap == 1) {
        baseColor = texture(albedoMap, TexCoords).rgb;
        // IMPORTANT: Convert sRGB texture to Linear Space for PBR calculations
        baseColor = pow(baseColor, vec3(2.2)); 
    }

    // 2. Calculate Normal (World Space)
    vec3 N = normalize(Normal);
    if (useNormalMap == 1) {
        // Sample Normal Map [0,1]
        vec3 normalMapValue = texture(normalMap, TexCoords).rgb;
        // Remap to [-1,1]
        normalMapValue = normalMapValue * 2.0 - 1.0;
        // Transform Tangent Space -> World Space
        N = normalize(TBN * normalMapValue);
    }

    // 3. Pack Data into G-Buffer
    float r = roughness;
    float m = metallic;

    gAlbedoRough = vec4(baseColor, r);
    gNormalMetal = vec4(N, m);
    gEmissiveAO  = vec4(0.0, 0.0, 0.0, ao); // Emissive placeholder (todo)
}
