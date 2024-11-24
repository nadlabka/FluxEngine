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
		StorageBuffer = 1 << 1, //UAV
		DataReadBuffer = 1 << 2, //SRV
		IndexBuffer = 1 << 3,
		VertexBuffer = 1 << 4,
		IndirectBuffer = 1 << 5,
	};

	struct BufferFlags
	{
		bool requiredCopyStateToInit = true;
	};

	struct BufferDescription
	{
		uint32_t elementsNum;
		uint32_t elementStride;
		uint32_t unstructuredSize; //this is used for Uniform Buffers, %255
		BufferAccess access;
		BufferUsage usage;
		BufferFlags flags;
	};

	struct BufferRegionCopyDescription
	{
		uint32_t srcOffset;
		uint32_t destOffset;
		uint32_t width;
	};

	struct BufferRegionDescription
	{
		uint32_t offset;
		uint32_t size;
	};

	struct IBuffer
	{
		virtual void UploadData(void* srcData, const BufferRegionCopyDescription& regionCopyDesc) = 0;

		virtual uint32_t GetSize() = 0;

		virtual ~IBuffer() {}
;	};
}