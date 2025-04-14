#pragma once
#include <DataStructures/SolidVector.h>
#include <unordered_map>

namespace Assets
{
	template <typename T>
	class AssetsManager
	{
	public:
		using AssetId = SolidVector<T>::ID;

		static AssetsManager& GetInstance()
		{
			static AssetsManager instance;
			return instance;
		}

		AssetsManager(const AssetsManager& arg) = delete;
		AssetsManager& operator=(const AssetsManager& arg) = delete;

		template <typename... Args>
		AssetId CreateAsset(Args&&... args)
		{
			return m_assetsStorage.insert(std::forward<Args>(args)...);
		}

		template <typename... Args>
		T& AllocateAsset(Args&&... args)
		{
			return m_assetsStorage[m_assetsStorage.insert(std::forward<Args>(args)...)];
		}

		T& GetAsset(AssetId assetId)
		{
			return m_assetsStorage[assetId];
		}

		const T& GetAsset(AssetId assetId) const
		{
			return m_assetsStorage[assetId];
		}

		void AssignName(AssetId assetId, const std::string& name)
		{
			ASSERT(!m_assetsCacheMap.contains(name), "Name is already used");
			m_assetsCacheMap[name] = assetId;
		}

		const AssetId& GetAssetByName(const std::string& name) const
		{
			ASSERT(m_assetsCacheMap.contains(name), "Asset with this name doesn't exist");
			return m_assetsCacheMap[name];
		}

		bool IsAssetNameRegistered(const std::string& name)
		{
			return m_assetsCacheMap.contains(name);
		}

		void RemoveAsset(AssetId assetId)
		{
			m_assetsStorage.erase(assetId);
		}

		void Destroy()
		{
			m_assetsStorage.clear();
			m_assetsCacheMap.clear();
		}

		SolidVector<T>& GetAssetsStorage()
		{
			return m_assetsStorage;
		}

	private:
		AssetsManager() {}

		SolidVector<T> m_assetsStorage;
		std::unordered_map<std::string, AssetId> m_assetsCacheMap;
	};
}