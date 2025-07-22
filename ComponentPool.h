#pragma once

#include"ECSManager.h"

// コンポーネントを管理するコンテナクラス
template<typename CompType>
class ComponentPool
{

private:
	friend class ECSManager;
	// コンポーネントのインスタンスを管理するコンテナ
	std::vector<CompType> components;
	// このコンテナクラスが管理するコンポーネントが持つ一意なID
	static size_t compTypeID;

public:

	// コンテナのメモリを確保
	ComponentPool(const size_t a_size)
	{
		components.resize(a_size);
	}

	// CompTypeIDを取得する関数
	static inline const size_t GetID()
	{
		// この関数を初めて読んだ時にIDを発行
		if (!ComponentPool<CompType>::compTypeID)
		{
			ComponentPool<CompType>::compTypeID = ++ECSManager::nextCcompTypeID;
		}

		return ComponentPool<CompType>::compTypeID;
	}

	// コンポーネントを追加
	inline CompType* AddComponent(const size_t a_entity)
	{
		if (components.size() < a_entity)
		{
			components.resize(a_entity, CompType());
		}

		components[a_entity] = CompType();
		
		return &components[a_entity];
	}

	// コンポーネントを取得する
	inline CompType* GetComponent(const size_t a_entity)noexcept
	{
		// エンティティが有効なら
		if (components.size() >= a_entity)
		{
			return &components[a_entity];
		}

		return nullptr;
	}
};