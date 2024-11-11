#pragma once
#include <vector>
#include <DebugMacros.h>

struct IndicesRangeSolidVector
{
	void EraseIndex(uint32_t index)
	{
		m_freeIndices.push_back(index);
	}

	uint32_t AllocateIndex()
	{
		if (m_freeIndices.empty())
		{
			ASSERT(m_maxIndexCreated + 1 < m_maxIndexLimit, "Indeces total count exceeded limit");
			return ++m_maxIndexCreated;
		}
		uint32_t result = m_freeIndices.back();
		m_freeIndices.pop_back();
		return result;
	}

	std::vector<uint32_t> AllocateIndicesRange(uint32_t totalNum)
	{
		std::vector<uint32_t> result;
		result.reserve(totalNum);
		for (uint32_t i = 0; i < totalNum; i++)
		{
			if (m_freeIndices.empty())
			{
				ASSERT(m_maxIndexCreated + 1 < m_maxIndexLimit, "Indeces total count exceeded limit");
				result.push_back(++m_maxIndexCreated);
			}
			else
			{
				uint32_t index = m_freeIndices.back();
				m_freeIndices.pop_back();
				result.push_back(index);
			}
		}
		return result;
	}

	uint32_t EraseIndicesRange(uint32_t rangeBeginIndex, uint32_t totalNum)
	{
		for (uint32_t i = rangeBeginIndex; i < rangeBeginIndex + totalNum; i++)
		{
			m_freeIndices.push_back(i);
		}
	}

	uint32_t GetTotalSize()
	{
		return m_maxIndexCreated + 1;
	}

	uint32_t GetCurrentlyAllocatedIndicesNumber()
	{
		return GetTotalSize() - m_freeIndices.size();
	}

	void SetMaxIndexLimit(uint32_t value)
	{
		m_maxIndexLimit = value;
	}

	std::vector<uint32_t> m_freeIndices {};
	uint32_t m_maxIndexCreated = 0xffffffff;
	uint32_t m_maxIndexLimit = UINT32_MAX;
};