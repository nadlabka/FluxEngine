#pragma once
#include <cstdint>
#include <memory>
#include "Buffer.h"
#include "Sampler.h"

namespace RHI
{
	enum class DescriptorType 
	{
		UniformBuffer,
		StorageBuffer,
		Sampler,
		SampledImage,
		StorageImage
	};

	enum class BindingVisibility 
	{
		Vertex = 1 << 0,
		Fragment = 1 << 1,
		Compute = 1 << 2,
		VertexFragment = Vertex | Fragment,
	};

	struct DescriptorBinding 
	{
		uint32_t bindingIndex;
		DescriptorType descriptorsType;
		uint32_t descriptorsCount;
		BindingVisibility stageVisbility;
	};

	struct TextureBindDescription
	{
		std::shared_ptr<ITexture> texture;
		bool isUAV = false;
	};

	struct BufferBindDescription
	{
		std::shared_ptr<IBuffer> buffer;
		BufferBindingType bindingType;
	};

	struct ConstantBinding
	{
		size_t size;
		uint32_t bindingIndex;
		BindingVisibility visibility;
	};

	struct PipelineLayoutDescription
	{
		std::unordered_map<BufferBindDescription, DescriptorBinding> buffersBindings;
		std::unordered_map<TextureBindDescription, DescriptorBinding> texturesBindings;
		std::unordered_map<std::shared_ptr<ISampler>, DescriptorBinding> samplersBindings;

		std::vector<ConstantBinding> constantsBindings;
	};

	struct IPipelineLayout
	{
		virtual ~IPipelineLayout() {}
	};
}