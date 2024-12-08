#pragma once
#include <cstdint>
#include <memory>
#include "Buffer.h"
#include "Texture.h"
#include "Sampler.h"
#include "Shader.h"
#include <DebugMacros.h>

namespace RHI
{
	struct PipelineStageDescription
	{
		std::shared_ptr<IShader> shader;
	};

	enum class DescriptorResourceType
	{
		UniformBuffer,
		DataReadOnlyBuffer,
		StorageBuffer,
		Sampler,
		SampledImage,
		StorageImage
	};

	enum BindingVisibility : uint8_t
	{
		Vertex = 1 << 0,
		Fragment = 1 << 1,
		Compute = 1 << 2
	};

	inline BindingVisibility ConvertPipelineStageTypeToBindingVisibility(PipelineStageType stage)
	{
		switch (stage)
		{
		case PipelineStageType::Vertex: return BindingVisibility::Vertex;
		case PipelineStageType::Fragment: return BindingVisibility::Fragment;
		case PipelineStageType::Compute: return BindingVisibility::Compute;
		}
	}

	// fix these stupid duplicates later
	class DynamicallyBoundResources
	{
	public:
		struct TextureSubresourceDescriptorInfo
		{
			uint32_t slice = 0;
			uint32_t mip = 0;
		};
		
		struct ResourceBindingInfo
		{
			DescriptorResourceType type;
			BindingVisibility visibility;
		};

		struct TextureBindingInfo 
		{
			ResourceBindingInfo resourceBindingInfo;
			TextureSubresourceDescriptorInfo subresourceInfo;
		};

		void SetBufferBindingResource(const std::string& bindingName, std::shared_ptr<IBuffer> buffer)
		{
			m_buffersBindingsResourceMappings.at(bindingName).first = buffer;
		}
		void SetTextureBindingResource(const std::string& bindingName, std::shared_ptr<ITexture> texture)
		{
			m_texturesBindingsResourceMappings.at(bindingName).first = texture;
		}
		void SetSamplerToBinding(const std::string& bindingName, std::shared_ptr<ISampler> sampler)
		{
			m_samplersBindingsMappings.at(bindingName) = sampler;
		}

		void SetBufferDescriptorResourceType(const std::string& bindingName, DescriptorResourceType type)
		{
			ASSERT(m_parametersIndices.contains(bindingName), "No binding with this name is present in this Pipeline Layout");
			if (!m_buffersBindingsResourceMappings.contains(bindingName))
			{
				m_buffersBindingsNames.push_back(bindingName);
				m_buffersBindingsResourceMappings[bindingName].second.type = type;
			}
			m_buffersBindingsResourceMappings.at(bindingName).second.type = type;
		}
		void SetTextureDescriptorResourceType(const std::string& bindingName, DescriptorResourceType type)
		{
			ASSERT(m_parametersIndices.contains(bindingName), "No binding with this name is present in this Pipeline Layout");
			if (!m_texturesBindingsResourceMappings.contains(bindingName))
			{
				m_texturesBindingsNames.push_back(bindingName);
				m_texturesBindingsResourceMappings[bindingName].second.resourceBindingInfo.type = type;
			}
			m_texturesBindingsResourceMappings.at(bindingName).second.resourceBindingInfo.type = type;
		}

		void SetTextureSubresourceDescriptorInfo(const std::string& bindingName, const TextureSubresourceDescriptorInfo& info)
		{
			ASSERT(m_parametersIndices.contains(bindingName), "No binding with this name is present in this Pipeline Layout");
			if (!m_texturesBindingsResourceMappings.contains(bindingName))
			{
				m_texturesBindingsNames.push_back(bindingName);
				m_texturesBindingsResourceMappings[bindingName].second.subresourceInfo = info;
			}
			m_texturesBindingsResourceMappings.at(bindingName).second.subresourceInfo = info;
		}

		void SetBufferDescriptorVisibility(const std::string& bindingName, BindingVisibility visibility)
		{
			ASSERT(m_parametersIndices.contains(bindingName), "No binding with this name is present in this Pipeline Layout");
			if (!m_buffersBindingsResourceMappings.contains(bindingName))
			{
				m_buffersBindingsNames.push_back(bindingName);
				m_buffersBindingsResourceMappings[bindingName].second.visibility = visibility;
			}
			m_buffersBindingsResourceMappings.at(bindingName).second.visibility = visibility;
		}
		void SetTextureDescriptorVisibility(const std::string& bindingName, BindingVisibility visibility)
		{
			ASSERT(m_parametersIndices.contains(bindingName), "No binding with this name is present in this Pipeline Layout");
			if (!m_texturesBindingsResourceMappings.contains(bindingName))
			{
				m_texturesBindingsNames.push_back(bindingName);
				m_texturesBindingsResourceMappings[bindingName].second.resourceBindingInfo.visibility = visibility;
			}
			m_texturesBindingsResourceMappings.at(bindingName).second.resourceBindingInfo.visibility = visibility;
		}

		const std::vector<std::string>& GetBuffersBindingsNames() const { return m_buffersBindingsNames; }
		const std::vector<std::string>& GetTexturesBindingsNames() const { return m_texturesBindingsNames; }
		const std::vector<std::string>& GetSamplersBindingsNames() const { return m_samplersBindingsNames; }

		const std::pair<std::shared_ptr<IBuffer>, ResourceBindingInfo>& GetBufferBindingMapping(const std::string& bindingName) const
		{
			return m_buffersBindingsResourceMappings.at(bindingName);
		}
		const std::pair<std::shared_ptr<ITexture>, TextureBindingInfo>& GetTextureBindingMapping(const std::string& bindingName) const
		{
			return m_texturesBindingsResourceMappings.at(bindingName);
		}
		std::shared_ptr<ISampler> GetSamplerOfBinding(const std::string& bindingName)
		{
			return m_samplersBindingsMappings.at(bindingName);
		}

		void SetParameterIndex(const std::string& bindingName, uint32_t index)
		{
			m_parametersIndices[bindingName] = index;
		}

		uint32_t GetParameterIndex(const std::string& bindingName) const
		{
			ASSERT(m_parametersIndices.contains(bindingName), "No binding with this name is present in this Pipeline Layout");
			return m_parametersIndices.at(bindingName);
		}
	private:
		std::unordered_map<std::string, uint32_t> m_parametersIndices;

		std::vector<std::string> m_buffersBindingsNames; //use this to iterate through resources for state transition and setting descriptors indices
		std::unordered_map<std::string, std::pair<std::shared_ptr<IBuffer>, ResourceBindingInfo>> m_buffersBindingsResourceMappings;

		std::vector<std::string> m_texturesBindingsNames;
		std::unordered_map<std::string, std::pair<std::shared_ptr<ITexture>, TextureBindingInfo>> m_texturesBindingsResourceMappings;

		std::vector<std::string> m_samplersBindingsNames;
		std::unordered_map<std::string, std::shared_ptr<ISampler>> m_samplersBindingsMappings;
	};

	class BoundConstantBuffers
	{
	public:
		//name of a buffer in hlsl where resources descriptors indices are stored 
		static constexpr LPCSTR boundResourcesBufferName = "BoundResources";

		struct ConstantBufferBindingMapping
		{
			uint32_t parameterIndex;
			std::shared_ptr<IBuffer> buffer;
		};

		void AddConstantBufferBinding(const std::string& bindingName, uint32_t parameterIndex)
		{
			if (!m_bindingNameToConstantBufferBindingMapping.contains(bindingName))
			{
				m_bindingNameToConstantBufferBindingMapping[bindingName].parameterIndex = parameterIndex;
				m_constantBuffersBindingsNames.push_back(bindingName);
			}
		}
		void SetConstantBufferBindingMapping(const std::string& bindingName, std::shared_ptr<IBuffer> buffer)
		{
			ASSERT(m_bindingNameToConstantBufferBindingMapping.contains(bindingName), "No constant buffer with this name is present in this Pipeline Layout");
			m_bindingNameToConstantBufferBindingMapping[bindingName].buffer = buffer;
		}

		const ConstantBufferBindingMapping& GetConstantBufferBinding(const std::string& bindingName)
		{
			ASSERT(m_bindingNameToConstantBufferBindingMapping.contains(bindingName), "No constant buffer with this name is present in this Pipeline Layout");
			return m_bindingNameToConstantBufferBindingMapping[bindingName];
		}

		bool IsConstantBufferPresent(const std::string& bindingName)
		{
			return m_bindingNameToConstantBufferBindingMapping.contains(bindingName);
		}

		const std::vector<std::string>& GetConstantBuffersBindingsNames() { return m_constantBuffersBindingsNames; }

	private:
		std::vector<std::string> m_constantBuffersBindingsNames;
		std::unordered_map<std::string, ConstantBufferBindingMapping> m_bindingNameToConstantBufferBindingMapping;
	};

	struct PipelineLayoutBindings
	{
		DynamicallyBoundResources m_dynamicallyBoundResources;
		BoundConstantBuffers m_BoundConstantBuffers;
	};
}