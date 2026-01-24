#version 450 core

layout (location = 0) out vec4 FragColor;

layout (location = 20) uniform sampler2D gAlbedoRough;
layout (location = 21) uniform sampler2D gNormalMetal;
layout (location = 22) uniform sampler2D gDepth;

layout (location = 23) uniform mat4 invVP;
layout (location = 24) uniform vec3 camPos;
layout (location = 25) uniform vec2 viewportSize;

// Light parameters
layout (location = 26) uniform vec3 lightPos; // Direction/Pos for point lights
layout (location = 27) uniform vec3 lightColor;
layout (location = 28) uniform float lightRadius;
layout (location = 29) uniform float lightIntensity;

// Sun parameters
layout (location = 30) uniform vec3 sunDir;
layout (location = 31) uniform vec3 sunColor;

const float PI = 3.14159265359;

// --- BRDF Functions (Copied from StandardPBR) ---
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

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    vec2 uv = gl_FragCoord.xy / viewportSize;
    
    // 1. Get G-Buffer data
    vec4 albedoRough = texture(gAlbedoRough, uv);
    vec4 normalMetal = texture(gNormalMetal, uv);
    float depth = texture(gDepth, uv).r;

    if (depth == 1.0) discard; // Sky

    // 2. Reconstruct World Position
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 worldPos = invVP * ndc;
    vec3 WorldPos = worldPos.xyz / worldPos.w;

    // 3. Setup PBR Params
    vec3 albedo = albedoRough.rgb;
    float roughness = albedoRough.a;
    vec3 N = normalize(normalMetal.rgb);
    float metallic = normalMetal.a;
    vec3 V = normalize(camPos - WorldPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // 4. Point Light Calculation
    vec3 L = normalize(lightPos - WorldPos);
    vec3 H = normalize(V + L);
    float distance = length(lightPos - WorldPos);
    
    // if (distance > lightRadius) discard; // DEBUG: Disable culling

    // Standard attenuation
    float attenuation = 1.0 / (distance * distance + 1.0); // Add epsilon
    // Smooth radius fade
    float fade = clamp(1.0 - pow(distance / lightRadius, 4.0), 0.0, 1.0);
    fade = fade * fade;
    
    vec3 radiance = lightColor * lightIntensity * attenuation * fade;

    // BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
       
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  

    float NdotL = max(dot(N, L), 0.0);        
    vec3 color = (kD * albedo / PI + specular) * radiance * NdotL;

    // Output color (Additive blending will sum this)
    FragColor = vec4(color, 1.0);
    
    // DEBUG: Visualization
    // FragColor = vec4(albedo, 1.0); // Show Albedo
}
