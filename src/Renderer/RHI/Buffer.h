#pragma once
#include <cstdint>

namespace RHI
{
	enum class BufferAccess : uint8_t
	{
		DefaultPrivate,
		Upload,
		Readback
	};

	enum BufferUsage : uint8_t
	{
		None = 0,
		UniformBuffer = 1 << 0,
		StorageBuffer = 1 << 1,
		IndexBuffer = 1 << 2,
		VertexBuffer = 1 << 3,
		IndirectBuffer = 1 << 4,
	};

	struct BufferFlags
	{
		bool isMutable = false;
		bool isCopySrc = false;
		bool isCopyDst = false;
	};

	struct BufferDescription
	{
		uint32_t elementsNum;
		uint32_t elementStride;
		uint32_t unstructuredSize;
		BufferAccess access;
		BufferUsage usage;
		BufferFlags flags;
	};

	enum class BufferBindingType : uint8_t
	{	
		SRV,
		CBV,
		UAV
	};

	struct IBuffer
	{
		virtual ~IBuffer() {}
;	};
}