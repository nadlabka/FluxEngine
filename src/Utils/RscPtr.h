#pragma once

template <typename T>
struct RscPtr
{
	RscPtr() = default;
	~RscPtr() { Release(); }

	RscPtr(const RscPtr& other) { *this = other; }
	RscPtr& operator=(const RscPtr& other)
	{
		if (m_ptr) m_ptr->Release();
		m_ptr = other.m_ptr;
		if (m_ptr) m_ptr->AddRef();
		return *this;
	}

	RscPtr(RscPtr&& other) noexcept { *this = std::move(other); }
	RscPtr& operator=(RscPtr&& other) noexcept
	{
		if (m_ptr) m_ptr->Release();
		m_ptr = other.m_ptr;
		other.m_ptr = nullptr;
		return *this;
	}

	T* ptr() const { return m_ptr; }
	T* ptr() { return m_ptr; }

	T* operator->() const { return m_ptr; }
	T* operator->() { return m_ptr; }

	operator T* () const { return m_ptr; }
	operator T* () { return m_ptr; }

	T** operator&() const { return &m_ptr; }
	T** operator&() { return &m_ptr; }

	T** Reset()
	{
		Release();
		return &m_ptr;
	}

	void Reset(T* ptr)
	{
		Release();
		m_ptr = ptr;
	}

	void Release()
	{
		if (m_ptr)
		{
			m_ptr->Release();
			m_ptr = nullptr;
		}
	}

protected:
	T* m_ptr = nullptr;
};
