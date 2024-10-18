#pragma once
#include <cstdint>

namespace RHI
{
		enum class BufferVisibility : uint8_t
		{
			DefaultPrivate = 1 << 0,
			Upload = 1 << 1,
			Readback = 1 << 2,
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
			BufferVisibility visibility;
			BufferUsage usage;
			BufferFlags flags;
		};

	struct IBuffer
	{
		virtual ~IBuffer() {}
;	};
}