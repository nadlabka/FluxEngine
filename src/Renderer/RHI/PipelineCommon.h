#pragma once
#include "Shader.h"
#include "PipelineLayout.h"
#include "RenderPass.h"

namespace RHI
{
	enum class VertexAttributeFormat : uint8_t
	{
		Undefined,
		R32_Uint,
		R32G32_SignedFloat,
		R32G32B32_SignedFloat,
		R32G32B32A32_SignedFloat
	};

	enum class BindingInputRate : uint8_t
	{
		PerVertex,
		PerInstance
	};

	// even if it's dynamic in vulkan, set it implicitly in BindPipeline, 
	// because dx12 needs to call IASetPrimitiveTopology to specify exact one anyways
	enum class PrimitiveTopology : uint8_t
	{
		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip,
		PatchList
	};

	enum class PolygonFillMode : uint8_t
	{
		Fill,
		Line,
		Point
	};

	enum class WindingOrder : uint8_t
	{
		Clockwise,
		Counterclockwise
	};

	enum class CullMode : uint8_t
	{
		None,
		Front,
		Back
	};

	enum class BlendFactor : uint8_t
	{
		Zero,
		One,
		SourceColor,
		OneMinusSourceColor,
		DestColor,
		OneMinusDestColor,
		SourceAlpha,
		OneMinusSourceAlpha,
		DestAlpha,
		OneMinusDestAlpha,
		ConstantColor,
		OneMinusConstantColor,
		ConstantAlpha,
		OneMinusConstantAlpha,
		SourceAlphaSaturate,
		Source1Color,
		OneMinusSource1Color,
		Source1Alpha,
		OneMinusSource1Alpha
	};

	enum class BlendOperation : uint8_t
	{
		Add,
		Subtract,
		ReverseSubtract,
		Min,
		Max
	};

	enum class LogicalOperation : uint8_t
	{
		Clear,
		AND,
		ANDReverse,
		Copy,
		ANDInverted,
		NoOp,
		XOR,
		OR,
		NOR,
		Equivalent,
		Invert,
		ORReverse,
		CopyInverted,
		ORInverted,
		NAND,
		Set
	};

	enum class ColorWriteMask : uint8_t
	{
		Red = 0b0001,
		Green = 0b0010,
		Blue = 0b0100,
		Alpha = 0b1000,
		RGB = Red | Green | Blue,
		RGBA = RGB | Alpha
	};

	enum class DepthStencilCompareOperation : uint8_t
	{
		Never,
		Less,
		Equal,
		LessOrEqual,
		Greater,
		NotEqual,
		GreaterOrEqual,
		Always
	};

	enum class StencilOperation : uint8_t
	{
		Keep,
		Zero,
		Replace,
		IncrementClamp,
		DecrementClamp,
		Invert,
		IncrementWrap,
		DecrementWrap
	};

	struct ViewportInfo
	{
		float topLeftX;
		float topLeftY;
		float width;
		float height;
		float minDepth;
		float maxDepth;
	};

	struct ScissorsRect
	{
		int32_t offsetX;
		int32_t offsetY;
		uint32_t extentX;
		uint32_t extentY;
	};

	struct PipelineStageDescription
	{
		std::shared_ptr<IShader> shader;
	};

	struct InputAssemblerLayoutDescription
	{
		//data from this struct(offset, stride) is going to be used in BindVertexBuffer of CommandBuffer
		struct BindingDescription
		{
			uint32_t stride; // in bytes
			uint32_t binding;
			BindingInputRate inputRate;
		};

		struct AttributeDescription
		{
			uint32_t location;
			uint32_t binding;
			uint32_t offset; // in bytes
			VertexAttributeFormat format;
			std::string semanticsName;
			uint32_t semanticsIndex;
		};

		std::vector<BindingDescription> vertexBindings;
		std::vector<AttributeDescription> attributeDescriptions;
	};

	struct InputAssemblerDescription
	{
		PrimitiveTopology primitiveTopology;
	};

	struct RasterizerDescription
	{
		bool depthClipEnable = false;
		bool rasterizerDiscardEnable = false;

		PolygonFillMode polygonOverride = PolygonFillMode::Fill;

		CullMode cullMode = CullMode::Back;

		WindingOrder windingOrder = WindingOrder::Clockwise;

		struct DepthBias
		{
			float clampValue = 0;
			float constantFactor = 0;
			float slopeScaledFactor = 0;
			bool enable = false;
		};

		DepthBias depthBias;
	};

	struct ColorBlendDescription
	{
		struct ColorAttachmentBlendDesc
		{
			bool blendEnabled = false;

			BlendFactor sourceColorBlendFactor = BlendFactor::One;
			BlendFactor destinationColorBlendFactor = BlendFactor::Zero;
			BlendFactor sourceAlphaBlendFactor = BlendFactor::One;
			BlendFactor destinationAlphaBlendFactor = BlendFactor::Zero;

			BlendOperation colorBlendOperation = BlendOperation::Add;
			BlendOperation alphaBlendOperation = BlendOperation::Add;

			ColorWriteMask colorWriteMask = ColorWriteMask::RGBA;
		};

		bool independentBlendEnabled;
		std::vector<ColorAttachmentBlendDesc> attachmentsBlends;

		bool logicalOperationEnabled = false;
		LogicalOperation logicalOperation = LogicalOperation::Copy;
	};

	struct DepthStencilDescription
	{
		bool depthTestEnabled = false;
		bool depthWriteEnabled = false;
		DepthStencilCompareOperation depthCompareOperation = DepthStencilCompareOperation::Never;

		bool stencilTestEnabled = false;
		uint32_t stencilReadMask;
		uint32_t stencilWriteMask;

		struct StencilState
		{
			StencilOperation stencilFailOperation = StencilOperation::Keep;
			StencilOperation stencilDepthFailOperation = StencilOperation::Keep;
			StencilOperation stencilPassOperation = StencilOperation::Keep;
			DepthStencilCompareOperation stencilCompareOperation = DepthStencilCompareOperation::Never;
		};

		StencilState frontStencilState;
		StencilState backStencilState;
	};

	//don't forget about setting up dynamic states in vulkan
	struct RenderPipelineDescription
	{
		InputAssemblerLayoutDescription inputAssemblerLayout;
		InputAssemblerDescription inputAssembler;
		RasterizerDescription rasterizer;
		ColorBlendDescription colorBlend;
		DepthStencilDescription depthStencil;

		std::vector<PipelineStageDescription> pipelineStages;
		std::shared_ptr<IPipelineLayout> pipelineLayout;
		std::shared_ptr<IRenderPass> renderPass;
	};

	struct IRenderPipeline
	{
		virtual RenderPipelineDescription& GetPipelineDescription() = 0;

		virtual ~IRenderPipeline() {}
	};

	struct ComputePipelineDescription
	{
		PipelineStageDescription pipelineStage;
		std::shared_ptr<IPipelineLayout> pipelineLayout;
	};

	struct IComputePipeline
	{
		virtual ~IComputePipeline() {}
	};
}