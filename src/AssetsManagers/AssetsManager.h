#pragma once
#include <DataStructures/SolidVector.h>

template <typename T>
class AssetsManager
{
public:
	using AssetId = SolidVector::ID;

	static AssetsManager& GetInstance()
	{
		static AssetsManager instance;
		return instance;
	}

	AssetsManager(const AssetsManager& arg) = delete;
	AssetsManager& operator=(const AssetsManager& arg) = delete;

	template <typename... Args>
	T& CreateAsset(Args&&... args)
	{
		auto assetId = m_assetsStorage.insert(std::forward<Args>(args)...);
		return m_assetsStorage[assetId];
	}

	T& GetAsset(AssetId assetId) 
	{
		return m_assetsStorage[assetId];
	}

	const T& GetAsset(AssetId assetId) const 
	{
		return m_assetsStorage[assetId];
	}

	void RemoveAsset(AssetId assetId)
	{
		m_assetsStorage.erase(assetId);
	}

private:
	AssetsManager() {}

	SolidVector<T> m_assetsStorage;
	std::unordered_map<std::wstring, AssetId> m_assetsCacheMap;
};