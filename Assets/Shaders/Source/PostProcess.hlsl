cbuffer BoundResources : register(b0)
{
    uint hdrTextureIndex;
    uint pointSamplerIndex;
};

float3 ACESTonemap(float3 rgb)
{
    const float3x3 IN = float3x3(
        0.59719, 0.35458, 0.04823,
        0.07600, 0.90834, 0.01566,
        0.02840, 0.13383, 0.83777
    );
    
    const float3x3 OUT = float3x3(
        1.60475, -0.53108, -0.07367,
        -0.10208, 1.10813, -0.00605,
        -0.00327, -0.07276, 1.07602
    );
    
    rgb = max(0.0, rgb);
    
    float3 col = mul(IN, rgb);
    
    float3 a = col * (col + 0.0245786f) - 0.000090537f;
    float3 b = col * (0.983729f * col + 0.4329510f) + 0.238081f;
    col = a / max(b, 0.0001f);
    
    return clamp(mul(OUT, col), 0.0, 1.0);
}

float3 GammaCorrect(float3 color)
{
    return pow(color, 1.0 / 2.2);
}

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

VSOutput VSMain(uint vertexID : SV_VertexID)
{
    VSOutput output;
    
    // Full-screen triangle (covers NDC: [-1,1] x [-1,1])
    // vertexID: 0 -> (-1, -1), 1 -> (3, -1), 2 -> (-1, 3)
    output.uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.position = float4(output.uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
    
    return output;
}

float4 PSMain(VSOutput input) : SV_Target
{
    Texture2D<float4> hdrTexture = ResourceDescriptorHeap[hdrTextureIndex];
    SamplerState pointSampler = SamplerDescriptorHeap[pointSamplerIndex];
    
    uint2 texDims;
    hdrTexture.GetDimensions(texDims.x, texDims.y);

    float3 hdrColor = hdrTexture.SampleLevel(pointSampler, input.uv, 0).rgb;
    
    float3 ldrColor = ACESTonemap(hdrColor);
    
    float3 finalColor = GammaCorrect(ldrColor);
    
    return float4(finalColor, 1.0);
}