#version 450 core
// --- Deferred Lighting Pass Fragment Shader ---
// Purpose: Calculates PBR lighting for a single Point Light source.
// This shader is executed for each fragment of the Light Volume sphere.
// It reconstructs World Position from the Depth Buffer and accumulates lighting.

// Output: Added to the HDR Color Buffer via Additive Blending (ONE, ONE)
layout (location = 0) out vec4 FragColor;

// G-Buffer Inputs (Samplers)
layout (location = 20) uniform sampler2D gAlbedoRough;
layout (location = 21) uniform sampler2D gNormalMetal;
layout (location = 22) uniform sampler2D gDepth;

// Global Inputs
layout (location = 23) uniform mat4 invVP;    // Inverse View-Projection Matrix (for Pos reconstruction)
layout (location = 24) uniform vec3 camPos;   // Camera Position in World Space
layout (location = 25) uniform vec2 viewportSize;

// Light Properties (Per-Instance Uniforms)
layout (location = 26) uniform vec3 lightPos;
layout (location = 27) uniform vec3 lightColor;
layout (location = 28) uniform float lightRadius;
layout (location = 29) uniform float lightIntensity;

// Sun parameters (Unused in point light pass, but reserved)
layout (location = 30) uniform vec3 sunDir;
layout (location = 31) uniform vec3 sunColor;

const float PI = 3.14159265359;

// --- Cook-Torrance BRDF Functions ---

// Normal Distribution Function (Trowbridge-Reitz GGX)
// Approximates the amount of microfacets aligned to the halfway vector.
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / denom;
}

// Geometry Function (Schlick-GGX)
// Approximates microfacet self-shadowing.
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

// Smith's method (combines view and light direction shadowing)
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// Fresnel Equation (Fresnel-Schlick approximation)
// Calculates the ratio of surface reflection at different angles.
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    // 0. Calculate Screen UV
    vec2 uv = gl_FragCoord.xy / viewportSize;
    
    // 1. Sample G-Buffer
    vec4 albedoRough = texture(gAlbedoRough, uv);
    vec4 normalMetal = texture(gNormalMetal, uv);
    float depth = texture(gDepth, uv).r;

    // Optimization: Skip sky pixels (Depth = 1.0)
    // Note: Depends on Depth Function (LessEqual) and Clear Value (1.0)
    if (depth >= 1.0) discard; 

    // 2. Reconstruct World Position from Depth
    // Transform NDC (x, y, z*2-1) back to World Space using Inverse VP
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 worldPos = invVP * ndc;
    vec3 WorldPos = worldPos.xyz / worldPos.w;

    // 3. Unpack PBR Parameters
    vec3 albedo = albedoRough.rgb;
    float roughness = albedoRough.a;
    vec3 N = normalize(normalMetal.rgb);
    float metallic = normalMetal.a;
    vec3 V = normalize(camPos - WorldPos);

    // Calculate F0 (Surface Reflection at zero incidence)
    // 0.04 for dielectrics, Albedo for metals
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // 4. Lighting Calculation (Cook-Torrance BRDF)
    vec3 L = normalize(lightPos - WorldPos);
    vec3 H = normalize(V + L);
    float distance = length(lightPos - WorldPos);
    
    // Hard cutoff at radius
    if (distance > lightRadius) discard;

    // Attenuation
    // Physical inverse-square falloff
    float attenuation = 1.0 / (distance * distance + 1.0); // +1.0 to prevent division by zero
    
    // Smooth falloff to zero at radius (Karis' function)
    float fade = clamp(1.0 - pow(distance / lightRadius, 4.0), 0.0, 1.0);
    fade = fade * fade;
    
    vec3 radiance = lightColor * lightIntensity * attenuation * fade;

    // BRDF Terms
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
       
    // Specular contribution
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    // Energy Conservation
    // kS = Specular part (F), kD = Diffuse part
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    // Metals have no diffuse component
    kD *= 1.0 - metallic;	  

    float NdotL = max(dot(N, L), 0.0);        
    
    // Final Radiance Sum
    vec3 color = (kD * albedo / PI + specular) * radiance * NdotL;

    // Output
    FragColor = vec4(color, 1.0);
}
