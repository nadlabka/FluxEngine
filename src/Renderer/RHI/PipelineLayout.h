#pragma once
#include <cstdint>
#include <memory>
#include "Buffer.h"
#include "Texture.h"
#include "Sampler.h"

namespace RHI
{
	enum class DescriptorType 
	{
		UniformBuffer,
		DataReadBuffer,
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

	struct ConstantBinding
	{
		size_t size;
		uint32_t bindingIndex;
		BindingVisibility visibility;
	};

	struct PipelineLayoutDescription
	{
		// you need resource pointers to transit resource state in DX12 and to update Descriptors Sets in Vulkan
		std::unordered_map<std::shared_ptr<IBuffer>, DescriptorBinding> buffersBindings;
		std::unordered_map<std::shared_ptr<ITexture>, DescriptorBinding> texturesBindings;
		std::unordered_map<std::shared_ptr<ISampler>, DescriptorBinding> samplersBindings;

		std::vector<ConstantBinding> constantsBindings;
	};

	struct IPipelineLayout
	{
		virtual ~IPipelineLayout() {}
	};
}