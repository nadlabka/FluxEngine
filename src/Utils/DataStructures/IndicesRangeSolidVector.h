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

	std::vector<size_t> AllocateIndicesRange(size_t totalNum)
	{
		std::vector<size_t> result;
		result.reserve(totalNum);
		for (size_t i = 0; i < totalNum; i++)
		{
			if (m_freeIndices.empty())
			{
				ASSERT(m_maxIndexCreated + 1 > m_maxIndexLimit, "Indeces total count exceeded limit");
				result.push_back(++m_maxIndexCreated);
			}
			else
			{
				size_t index = m_freeIndices.back();
				m_freeIndices.pop_back();
				result.push_back(index);
			}
		}
		return result;
	}

	size_t EraseIndicesRange(size_t rangeBeginIndex, size_t totalNum)
	{
		for (size_t i = rangeBeginIndex; i < rangeBeginIndex + totalNum; i++)
		{
			m_freeIndices.push_back(i);
		}
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