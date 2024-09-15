#pragma once
#include <vector>

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
			return ++m_maxIndexExisted;
		}
		size_t result = m_freeIndices.back();
		m_freeIndices.pop_back();
		return result;
	}

	std::vector<size_t> m_freeIndices{};
	size_t m_maxIndexExisted = -1;
};