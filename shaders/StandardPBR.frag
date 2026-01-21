#version 450 core

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec3 WorldPos;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoords;

// Material parameters
layout (location = 20) uniform vec3 albedo;
layout (location = 21) uniform float metallic;
layout (location = 22) uniform float roughness;
layout (location = 23) uniform float ao;

// Light parameters
layout (location = 24) uniform vec3 lightPosition;
layout (location = 25) uniform vec3 lightColor;
layout (location = 26) uniform vec3 camPos;

// IBL parameters
layout (location = 27) uniform samplerCube irradianceMap;
layout (location = 28) uniform samplerCube prefilterMap;
layout (location = 29) uniform sampler2D brdfLUT;

// Texture parameters
layout (location = 30) uniform sampler2D albedoMap;
layout (location = 31) uniform sampler2D normalMap;
layout (location = 32) uniform int useAlbedoMap;
layout (location = 33) uniform int useNormalMap;

layout (location = 34) uniform sampler2DShadow shadowMap;

layout (location = 3) in vec4 FragPosLightSpace;

const float PI = 3.14159265359;

// --- Shadow Calculation ---
float ShadowCalculation(vec4 fragPosLightSpace) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // check if outside light frustum
    if(projCoords.z > 1.0) return 0.0;

    // 3x3 PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, projCoords.z));
            shadow += pcfDepth; // texture() returns 1.0 if not in shadow, 0.0 if in shadow
        }    
    }
    shadow /= 9.0;
    
    return 1.0 - shadow;
}

// --- BRDF Functions ---

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

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{		
    vec3 N = normalize(Normal);
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(-V, N); 

    // F0: Surface reflection at zero incidence
    vec3 F0 = vec3(0.04); 
    
    vec3 baseColor = albedo;
    if (useAlbedoMap == 1) {
        baseColor = texture(albedoMap, TexCoords).rgb;
        // Assume sRGB texture, convert to linear
        baseColor = pow(baseColor, vec3(2.2));
    }

    F0 = mix(F0, baseColor, metallic);

    // --- Direct Lighting ---
    vec3 Lo = vec3(0.0);
    
    vec3 L = normalize(lightPosition - WorldPos);
    vec3 H = normalize(V + L);
    float distance = length(lightPosition - WorldPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = lightColor * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F_dir = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
       
    vec3 numerator    = NDF * G * F_dir; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    vec3 kS = F_dir;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  

    float NdotL = max(dot(N, L), 0.0);        

    float shadow = ShadowCalculation(FragPosLightSpace);
    Lo += (1.0 - shadow) * (kD * baseColor / PI + specular) * radiance * NdotL;

    // --- IBL Part ---
    vec3 F_ibl = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec3 kS_ibl = F_ibl;
    vec3 kD_ibl = 1.0 - kS_ibl;
    kD_ibl *= 1.0 - metallic;	  
    
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse    = irradiance * baseColor;
    
    // Split-Sum approximation
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specularIBL = prefilteredColor * (F_ibl * brdf.x + brdf.y);

    vec3 ambient = (kD_ibl * diffuse + specularIBL) * ao;
    
    vec3 color = ambient + Lo;

    // HDR tone mapping & Gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}