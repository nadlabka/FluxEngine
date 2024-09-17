#pragma once
#include <vector>
#include <DebugMacros.h>

struct IndicesRangeSolidVector
{
	void EraseIndex(size_t index)
	{
		m_freeIndices.push_back(index);
	}

	size_t AllocateIndex()
	{
		if (m_freeIndices.empty())
		{
			ASSERT(m_maxIndexCreated + 1 > m_maxIndexLimit, "Indeces total count exceeded limit");
			return ++m_maxIndexCreated;
		}
		size_t result = m_freeIndices.back();
		m_freeIndices.pop_back();
		return result;
	}

	size_t GetTotalSize()
	{
		return m_maxIndexCreated + 1;
	}

	size_t GetCurrentlyAllocatedIndicesNumber()
	{
		return GetTotalSize() - m_freeIndices.size();
	}

	void SetMaxIndexLimit(size_t value)
	{
		m_maxIndexLimit = value;
	}

	std::vector<size_t> m_freeIndices{};
	size_t m_maxIndexCreated = -1;
	size_t m_maxIndexLimit = ULLONG_MAX;
};