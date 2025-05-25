cbuffer BoundResources : register(b0)
{
    uint perMeshDataBufferIndex;
    uint perInstancePerMeshHandleBufferIndex;
    uint perInstanceMaterialParamsBufferIndex;
    uint pointLightsBufferIndex;
    uint spotLightsBufferIndex;
    uint directionalLightsBufferIndex;
    uint samplerDescriptorIndex;
    uint shadowSamplerDescriptorIndex;
};

cbuffer PerView : register(b1)
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 viewProjectionMatrix;
    float4 cameraPosition;
    float4 cameraDirection;

    float nearPlane;
    float farPlane;

    float viewportWidth;
    float viewportHeight;
}

cbuffer PerFrame : register(b2)
{
    uint pointLightNum;
    uint spotLightNum;
    uint directionalLightNum;
}

struct PerMeshHandle
{
    uint index;
};

struct PBRMaterial
{
    uint albedoIndex;
    uint normalIndex;
    uint metallicRoughnessIndex;
    uint aoIndex;
    uint emissiveIndex;
};

struct PerMeshData
{
    float4x4 transform;
    float4x4 inverseTransposeTransform;
};

struct PointLightSourceData
{
    float4x4 worldToLightView;
    float4x4 worldToLightClip;
    float4 color;
    float3 position;
    float padding;
};

struct SpotLightSourceData
{
    float4x4 worldToLightView;
    float4x4 worldToLightClip;
    float3 position;
    float innerConeCos;
    float4 color;
    float3 direction;
    float outerConeCos;
};

struct DirectionalLightSourceData
{
    float4x4 worldToLightView;
    float4x4 worldToLightClip;
    float4 color;
    float3 direction;
    uint shadowmapDescriptorIndex;
};

struct VSInput
{
    float3 position : POSITION;
    float3 normals : NORMAL;
    float2 texCoords : TEXCOORD;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    uint instanceId : SV_InstanceID;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 texCoords : TEXCOORD;
    float3x3 TBN : TBN_MATRIX;
    nointerpolation uint instanceId : INSTANCE_ID;
};

static const float PI = 3.14159265359;
static const float MIN_ROUGHNESS = 0.04;

float D_GGX(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float3 F_Schlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float G_SchlickGGX(float NdotV, float NdotL, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float denomV = NdotV * (1.0 - k) + k;
    float denomL = NdotL * (1.0 - k) + k;
    return (NdotV / denomV) * (NdotL / denomL);
}

float3 Diffuse_Lambert(float3 albedo)
{
    return albedo / PI;
}

float Attenuate(float distance)
{
    float d = max(distance, 0.001);
    return 1.0 / (d * d);
}

PSInput VSMain(VSInput input)
{
    PSInput result;

    StructuredBuffer<PerMeshData> perMeshDataBuffer = ResourceDescriptorHeap[perMeshDataBufferIndex];
    StructuredBuffer<PerMeshHandle> perInstancePerMeshHandlesBuffer = ResourceDescriptorHeap[perInstancePerMeshHandleBufferIndex];

    PerMeshData perMeshData = perMeshDataBuffer[perInstancePerMeshHandlesBuffer[input.instanceId].index];

    StructuredBuffer<PBRMaterial> perInstanceMaterialParamsBuffer = ResourceDescriptorHeap[perInstanceMaterialParamsBufferIndex];
    PBRMaterial material = perInstanceMaterialParamsBuffer[input.instanceId];

    float4 worldPosition = mul(float4(input.position, 1.0f), perMeshData.transform);
    result.position = mul(worldPosition, viewProjectionMatrix);

    float4x4 invTrans = perMeshData.inverseTransposeTransform;
    float3 worldNormal = normalize(mul(float4(input.normals, 0.0f), invTrans).xyz);
    float3 worldTangent = normalize(mul(float4(input.tangent, 0.0f), invTrans).xyz);
    float3 worldBitangent = normalize(mul(float4(input.bitangent, 0.0f), invTrans).xyz);

    // Ensure TBN is orthogonal using Gram-Schmidt
    worldNormal = normalize(worldNormal);
    worldTangent = normalize(worldTangent - dot(worldTangent, worldNormal) * worldNormal);
    worldBitangent = normalize(worldBitangent - dot(worldBitangent, worldNormal) * worldNormal - dot(worldBitangent, worldTangent) * worldTangent);

    result.TBN = float3x3(worldTangent, worldBitangent, worldNormal);
    result.normal = input.normals;
    result.worldPosition = worldPosition.xyz;
    result.texCoords = input.texCoords;
    result.instanceId = input.instanceId;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    StructuredBuffer<PBRMaterial> perInstanceMaterialParamsBuffer = ResourceDescriptorHeap[perInstanceMaterialParamsBufferIndex];
    PBRMaterial material = perInstanceMaterialParamsBuffer[input.instanceId];
    SamplerState defaultSampler = SamplerDescriptorHeap[samplerDescriptorIndex];
    SamplerComparisonState shadowSampler = SamplerDescriptorHeap[shadowSamplerDescriptorIndex];
    
    float4 albedo = float4(0.5, 0.5, 0.5, 1.0);
    float metallic = 0.2;
    float roughness = 0.5;
    float ao = 1.0;
    float3 emissive = float3(0.0, 0.0, 0.0);
    float3 normal = input.normal;
    
    if (material.albedoIndex != 0xffffffff)
    {
        Texture2D albedoMap = ResourceDescriptorHeap[material.albedoIndex];
        albedo = albedoMap.Sample(defaultSampler, input.texCoords, 0);
        if (albedo.w < 0.25) discard;

    }
    if (material.normalIndex != 0xffffffff)
    {
        Texture2D normalMap = ResourceDescriptorHeap[material.normalIndex];
        float3 tangentNormal = normalMap.Sample(defaultSampler, input.texCoords, 0).xyz * 2.0 - 1.0;
        normal = normalize(mul(tangentNormal, input.TBN));
    }
    if (material.metallicRoughnessIndex != 0xffffffff)
    {
        Texture2D metallicRoughnessMap = ResourceDescriptorHeap[material.metallicRoughnessIndex];
        metallic = metallicRoughnessMap.Sample(defaultSampler, input.texCoords, 0).r;
        roughness = metallicRoughnessMap.Sample(defaultSampler, input.texCoords, 0).g;
    }
    if (material.aoIndex != 0xffffffff)
    {
        Texture2D aoMap = ResourceDescriptorHeap[material.aoIndex];
        ao = aoMap.Sample(defaultSampler, input.texCoords, 0).r;
    }
    if (material.emissiveIndex != 0xffffffff)
    {
        Texture2D emissiveMap = ResourceDescriptorHeap[material.emissiveIndex];
        emissive = emissiveMap.Sample(defaultSampler, input.texCoords, 0).rgb;
    }
    
    roughness = max(roughness, MIN_ROUGHNESS);
    
    float3 N = normalize(normal);
    float3 V = normalize(cameraPosition.xyz - input.worldPosition);
    float NdotV = max(dot(N, V), 0.0);
    
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo.xyz, metallic);

    float3 Lo = float3(0.0, 0.0, 0.0);
    
    StructuredBuffer<PointLightSourceData> pointLightsBuffer = ResourceDescriptorHeap[pointLightsBufferIndex];
    for (uint i = 0; i < pointLightNum; ++i)
    {
        PointLightSourceData light = pointLightsBuffer[i];
        float3 L = light.position - input.worldPosition;
        float distance = length(L);
        L = normalize(L);

        float3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float HdotV = max(dot(H, V), 0.0);
        
        float attenuation = Attenuate(distance);
        float3 radiance = light.color.rgb * light.color.a * attenuation;
        
        float D = D_GGX(NdotH, roughness);
        float3 F = F_Schlick(HdotV, F0);
        float G = G_SchlickGGX(NdotV, NdotL, roughness);

        float3 specular = (D * F * G) / max(4.0 * NdotV * NdotL, 0.001);
        float3 kS = F;
        float3 kD = (1.0 - kS) * (1.0 - metallic);
        float3 diffuse = kD * Diffuse_Lambert(albedo.xyz);

        Lo += (diffuse + specular) * radiance * NdotL;
    }
    
    StructuredBuffer<SpotLightSourceData> spotLightsBuffer = ResourceDescriptorHeap[spotLightsBufferIndex];
    for (uint i = 0; i < spotLightNum; ++i)
    {
        SpotLightSourceData light = spotLightsBuffer[i];
        float3 L = light.position - input.worldPosition;
        float distance = length(L);
        L = normalize(L);

        float3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float HdotV = max(dot(H, V), 0.0);
        
        float theta = dot(-L, normalize(light.direction));
        float epsilon = light.innerConeCos - light.outerConeCos;
        float intensity = clamp((theta - light.outerConeCos) / epsilon, 0.0, 1.0);
        
        float attenuation = Attenuate(distance) * intensity;
        float3 radiance = light.color.rgb * light.color.a * attenuation;
        
        float D = D_GGX(NdotH, roughness);
        float3 F = F_Schlick(HdotV, F0);
        float G = G_SchlickGGX(NdotV, NdotL, roughness);

        float3 specular = (D * F * G) / max(4.0 * NdotV * NdotL, 0.001);
        float3 kS = F;
        float3 kD = (1.0 - kS) * (1.0 - metallic);
        float3 diffuse = kD * Diffuse_Lambert(albedo.xyz);

        Lo += (diffuse + specular) * radiance * NdotL;
    }
    
    StructuredBuffer<DirectionalLightSourceData> directionalLightsBuffer = ResourceDescriptorHeap[directionalLightsBufferIndex];
    for (uint i = 0; i < directionalLightNum; ++i)
    {
        DirectionalLightSourceData light = directionalLightsBuffer[i];
        float3 L = -normalize(light.direction);
        float3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float HdotV = max(dot(H, V), 0.0);

        float3 radiance = light.color.rgb * light.color.a;

        // Cook-Torrance BRDF
        float D = D_GGX(NdotH, roughness);
        float3 F = F_Schlick(HdotV, F0);
        float G = G_SchlickGGX(NdotV, NdotL, roughness);

        float3 specular = (D * F * G) / max(4.0 * NdotV * NdotL, 0.001);
        float3 kS = F;
        float3 kD = (1.0 - kS) * (1.0 - metallic);
        float3 diffuse = kD * Diffuse_Lambert(albedo.xyz);
        
        float4 shadowTexCoord = mul(float4(input.worldPosition, 1.0f), light.worldToLightClip);
        shadowTexCoord.xy = saturate(shadowTexCoord.xy / float2(2.0f, -2.0f) + 0.5f);
        Texture2D<float> shadowmap = ResourceDescriptorHeap[light.shadowmapDescriptorIndex];
        
        uint2 shadowmapDimensions = uint2(0, 0);
        shadowmap.GetDimensions(shadowmapDimensions.x, shadowmapDimensions.y);
        
        float shadowed = 0.0f;
        float2 coordOffset = 1.0f / shadowmapDimensions;

        for (int m = -1; m <= 1; m++)
        {
            for (int n = -1; n <= 1; n++)
            {
                float2 sampleCoord = shadowTexCoord.xy + coordOffset;
                shadowed += shadowmap.SampleCmpLevelZero(shadowSampler, sampleCoord, shadowTexCoord.z);
            }
        }
        shadowed /= 9.0f;
        float shadowTerm = smoothstep(0.02f, 1.0f, shadowed);

        Lo += (diffuse + specular) * radiance * NdotL * shadowTerm;
    }
    
    float3 ambient = float3(0.03, 0.03, 0.03) * albedo.xyz * ao;
    
    float3 color = ambient + Lo + emissive;

    return float4(color, 1.0);
}