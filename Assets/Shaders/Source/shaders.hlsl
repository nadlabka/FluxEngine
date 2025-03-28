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

    float2 padding;
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
    float4 color : COLOR;
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

    result.color = float4((input.normals.xyz + float3(1.0f, 1.0f, 1.0f)) / 2.0f, 1.0f);
    result.instanceId = input.instanceId;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
