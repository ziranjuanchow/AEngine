#version 450 core
// --- Deferred Geometry Pass Vertex Shader ---
// Purpose: Transforms vertex attributes to World Space and passes TBN matrix for Normal Mapping.

// Input Attributes (Matched to FVertex layout)
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

// Outputs to Fragment Shader
layout (location = 0) out vec3 WorldPos;
layout (location = 1) out vec3 Normal; // Fallback Geometry Normal
layout (location = 2) out vec2 TexCoords;
layout (location = 3) out mat3 TBN;    // Tangent-Bitangent-Normal Matrix for Normal Mapping

// Uniforms (Global)
layout (location = 0) uniform mat4 model;
layout (location = 1) uniform mat4 view;
layout (location = 2) uniform mat4 projection;

void main()
{
    // Transform position to World Space
    WorldPos = vec3(model * vec4(aPos, 1.0));
    TexCoords = aTexCoords;
    
    // Calculate TBN Matrix (Model Space -> World Space)
    // Used to transform Normal Map vectors from Tangent Space to World Space.
    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
    vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
    TBN = mat3(T, B, N);
    
    Normal = N; // Pass geometry normal as fallback

    gl_Position = projection * view * vec4(WorldPos, 1.0);
}
