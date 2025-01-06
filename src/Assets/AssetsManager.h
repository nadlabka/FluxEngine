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

		void RemoveAsset(AssetId assetId)
		{
			m_assetsStorage.erase(assetId);
		}

		void Destroy()
		{
			m_assetsStorage.clear();
			m_assetsCacheMap.clear();
		}

	private:
		AssetsManager() {}

		SolidVector<T> m_assetsStorage;
		std::unordered_map<std::wstring, AssetId> m_assetsCacheMap;
	};
}