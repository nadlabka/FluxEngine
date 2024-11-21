#pragma once
#include <vector>
#include <cassert>
#include <DebugMacros.h>

template <typename T>
class SolidVector
{
public:
    using ID = uint32_t;
    using Index = uint32_t;

    uint32_t size() const { return uint32_t(m_data.size()); }

    const T* data() const { return m_data.data(); }
    T* data() { return m_data.data(); }

    const T& operator[](ID id) const 
    {
        ASSERT(isValidID(id), "Invalid ID");
        return m_data[m_forwardMap[id]];
    }
    T& operator[](ID id) 
    {
        ASSERT(isValidID(id), "Invalid ID");
        return m_data[m_forwardMap[id]];
    }

    template <typename... Args>
    ID insert(Args&&... args) 
    {
        ID id = m_nextUnused;

        if (id == m_forwardMap.size()) 
        {
            m_forwardMap.push_back(Index(m_forwardMap.size() + 1));
        }

        m_nextUnused = m_forwardMap[id];
        m_forwardMap[id] = Index(m_data.size());

        m_data.emplace_back(std::forward<Args>(args)...);
        m_backwardMap.emplace_back(id);

        return id;
    }

    void erase(ID id) 
    {
        ASSERT(isValidID(id), "Invalid ID for erase operation");

        Index& forwardIndex = m_forwardMap[id];

        // Move the last element to the erased position
        m_data[forwardIndex] = std::move(m_data.back());
        m_data.pop_back();

        ID backwardIndex = m_backwardMap.back();
        m_backwardMap[forwardIndex] = backwardIndex;
        m_backwardMap.pop_back();

        m_forwardMap[backwardIndex] = forwardIndex;

        // Mark the ID as unused
        forwardIndex = m_nextUnused;
        m_nextUnused = id;
    }

    void clear() 
    {
        m_forwardMap.clear();
        m_backwardMap.clear();
        m_data.clear();
        m_nextUnused = 0;
    }

    void reserve(Index count) 
    {
        m_data.reserve(count);
        m_forwardMap.reserve(count);
        m_backwardMap.reserve(count);
    }

protected:
    bool isValidID(ID id) const 
    {
        return id < m_forwardMap.size() && m_forwardMap[id] < m_data.size();
    }

    std::vector<T> m_data;
    std::vector<Index> m_forwardMap;
    std::vector<ID> m_backwardMap;
    ID m_nextUnused = 0;
};
