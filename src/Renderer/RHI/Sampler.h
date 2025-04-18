#pragma once

namespace RHI
{
    enum class FilterMode : uint8_t
    {
        Nearest,
        Linear,
        Anisotropic
    };

    enum class AddressMode : uint8_t
    {
        Wrap,
        Mirror,
        Clamp,
        Border,
        MirrorOnce
    };

    enum class SamplerComparisonFunc : uint8_t
    {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always
    };

    struct SamplerDescription
    {
        FilterMode Filter;
        AddressMode AddressU;
        AddressMode AddressV;
        AddressMode AddressW;
        float MipLODBias;
        uint32_t MaxAnisotropy;
        SamplerComparisonFunc ComparisonFunc;
        Vector4 BorderColor;
        float MinLOD;
        float MaxLOD;
	};

	struct ISampler
	{
		virtual ~ISampler() {}
	};
}