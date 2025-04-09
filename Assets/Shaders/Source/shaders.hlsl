//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

cbuffer BoundResources : register(b0)
{
    uint perMeshDataBufferIndex;
    uint perInstancePerMeshHandleBufferIndex;
    uint perInstanceMaterialParamsBufferIndex;

    uint pointLightsBufferIndex;
    uint spotLightsBufferIndex;
    uint directionalLightsBufferIndex;
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

struct UnlitDefaultParams
{
    uint testParameter;
};

struct PerMeshData
{
    float4x4 transform;
    float4x4 inverseTransposeTransform;
};

struct PointLightSourceData
{
    float4x4 worldToLight;
    float4x4 lightToWorld;
    float4 color;
    float3 position;
    float padding;
};

struct SpotLightSourceData
{
    float4x4 worldToLight;
    float4x4 lightToWorld;
    float3 position;
    float innerConeCos;
    float4 color;
    float3 direction;
    float outerConeCos;
};

struct DirectionalLightSourceData
{
    float4x4 worldToLight;
    float4x4 lightToWorld;
    float4 color;
    float3 direction;
    float padding;
};

struct VSInput
{
    float3 position : POSITION;
    float3 normals : NORMALS;
    uint instanceId : SV_InstanceID;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float3 worldNormal : WORLD_NORMAL;
    nointerpolation uint instanceId : INSTANCE_ID;
};

PSInput VSMain(VSInput input)
{
    PSInput result;

    StructuredBuffer<PerMeshData> perMeshDataBuffer = ResourceDescriptorHeap[perMeshDataBufferIndex];
    StructuredBuffer<PerMeshHandle> perInstancePerMeshHandlesBuffer = ResourceDescriptorHeap[perInstancePerMeshHandleBufferIndex];

    PerMeshData perMeshData = perMeshDataBuffer[perInstancePerMeshHandlesBuffer[input.instanceId].index];

    StructuredBuffer<UnlitDefaultParams> perInstanceMaterialParamsBuffer = ResourceDescriptorHeap[perInstanceMaterialParamsBufferIndex];
    UnlitDefaultParams perInstanceMaterialParams = perInstanceMaterialParamsBuffer[input.instanceId];

    float4 worldPosition = mul(float4(input.position, 1.0f), perMeshData.transform);
    result.position = mul(worldPosition, viewProjectionMatrix);

    float4x4 invTrans = perMeshData.inverseTransposeTransform;
    result.worldNormal = normalize(mul(float4(input.normals.xyz, 0.0f), invTrans).xyz);
    result.instanceId = input.instanceId;
    result.worldPosition = worldPosition.xyz;
    result.normal = input.normals;

    return result;
}

float3 CalculateBlinnPhong(float3 normal, float3 viewDir, float3 lightDir, float3 lightColorIntensity, float specularPower)
{
    normal = normalize(normal);
    lightDir = normalize(lightDir);
    viewDir = normalize(viewDir);

    // Diffuse term
    float diffuseFactor = max(dot(normal, lightDir), 0.0);
    float3 diffuse = diffuseFactor * lightColorIntensity;

    // Specular term (Blinn-Phong)
    float3 halfwayDir = normalize(lightDir + viewDir);
    float specularFactor = pow(max(dot(normal, halfwayDir), 0.0), specularPower);
    float3 specular = specularFactor * float3(1.0f, 1.0f, 1.0f) * lightColorIntensity; // White specular color

    return diffuse + specular;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    //float3 materialColor = (input.normal + float3(1.0f, 1.0f, 1.0f)) / 2.0f;
    float3 materialColor = float3(1.0f, 1.0f, 1.0f);
    float3 viewDir = normalize(cameraPosition - input.worldPosition);
    float3 finalColor = float3(0.0f, 0.0f, 0.0f);

    StructuredBuffer<PointLightSourceData> pointLightsBuffer = ResourceDescriptorHeap[pointLightsBufferIndex];
    for (int i = 0; i < pointLightNum; i++)
    {
        float3 lightColorIntensity = pointLightsBuffer[i].color.rgb * pointLightsBuffer[i].color.a;
        float3 lightDir = pointLightsBuffer[i].position - input.worldPosition;

        float3 lighting = CalculateBlinnPhong(input.worldNormal, viewDir, lightDir, lightColorIntensity, 32.0f);

        finalColor += materialColor * lighting;
    }

    return float4(finalColor, 1.0f);
}
