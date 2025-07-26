#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <array>
#include <vector>
#include <bitset>
#include <functional>

using Archetype = std::bitset<128>;

class ECSManager
{
public:
	// エンティティを新規作成
	inline const size_t GenerateEntity()
	{
		size_t nEntity;
		// リサイクル待ちエンティティがあればそこから1つ取り出す
		if (recycleEntities.size())
		{
			nEntity = recycleEntities.back();
			recycleEntities.pop_back();
		}
		// 無ければ新規発行
		else
		{
			nEntity = nextID;

			nextID++;
		}
		// エンティティを有効にする
		if (entityToActive.size() <= nEntity)
		{
			entityToActive.resize(nEntity + 1, false);
		}

		entityToActive[nEntity] = true;

		// 生成したエンティティを返す
		return nEntity;
	}

	// コンポーネントを追加する
	template<typename CompType>
	CompType* AddComponent(const size_t a_entity)
	{
		// コンポーネントの型のIDを取得
		size_t type = ComponentPool<CompType>::GetID();

		// 初めて生成するコンポーネントなら
		if (!typeToComponents[type])
		{
			// ComponentPoolを実体化する
			typeToComponents[type] = std::make_shared<ComponentPool<CompType>>(4096);
		}

		// 追加するコンポーネントを格納するコンテナクラスを取得
		std::shared_ptr<ComponentPool<CompType>> spCompPool = std::static_pointer_cast<ComponentPool<CompType>>(typeToComponents[type]);

		// コンポーネントを追加し取得
		CompType* pResultComp = spCompPool->AddComponent(a_entity);

		// コンポーネントを追加する前のアーキタイプを取得
		Archetype arch;
		if (entityToArchetype.size() > a_entity)
		{
			arch = entityToArchetype[a_entity];

			// 前アーキタイプのエンティティリストからエンティティを削除
			archToEntities[arch].Remove(a_entity);
		}
		else
		{
			entityToArchetype.resize(a_entity + 1, Archetype());

			//今回がコンポーネントを初めて追加するエンティティなら
			//上記の削除処理はしない
		}

		// アーキタイプを編集
		arch.set(type);

		if(archToEntities.find(arch) == archToEntities.end())
		{
			// 新しいアーキタイプならエンティティリストを初期化
			archToEntities[arch] = EntityContainer();
		}

		// 新しいアーキタイプをセット
		archToEntities[arch].Add(a_entity);

		entityToArchetype[a_entity] = arch;

		// 追加したコンポーネントを返す
		return pResultComp;
	}

	// コンポーネントを削除する
	template<typename CompType>
	void RemvoeComponent(const size_t a_entity)
	{
		// コンポーネントの型のIDを取得
		size_t type = ComponentPool<CompType>::GetID();

		Archetype arch;
		// コンポーネントを削除する前のアーキタイプを取得
		if (entityToArchetype.size() > a_entity)
		{
			arch = entityToArchetype[a_entity];
		}
		else
		{
			return;
		}
		// エンティティが削除するコンポーネントを持ってなかったらreturn
		if (!arch.test(type))
		{
			return;
		}
		// 前アーキタイプのエンティティリストからエンティティを削除
		archToEntities[arch].Remove(a_entity);

		// アーキタイプを編集
		arch.reset(type);

		// 新しいアーキタイプをセット
		archToEntities[arch].Add(a_entity);
		entityToArchetype[a_entity] = arch;
	}

	// エンティティを無効にする
	inline void RemoveEntity(const size_t a_entity)
	{
		entityToActive[a_entity] = false;
		archToEntities[entityToArchetype[a_entity]].Remove(a_entity);
		recycleEntities.emplace_back(a_entity);
	}

	// 処理を実行する
	template<typename ...CompType>
	void RunFunction(std::function<void(CompType&...)> a_func)
	{
		// 処理に必要なコンポーネントのアーキタイプを取得
		Archetype arch;
		(arch.set(ComponentPool<CompType>::GetID()), ...);
		// 処理に必要なアーキタイプを含むアーキタイプを持つエンティティのリストを検索
		for (auto&& entities : archToEntities)
		{
			if ((entities.first & arch) == arch)
			{
				for (auto&& entity : entities.second.GetEntities())
				{
					a_func(std::static_pointer_cast<ComponentPool<CompType>>
						(typeToComponents[ComponentPool<CompType>::GetID()])->components[entity]...);
				}
			}
		}
	}

	// エンティティを管理するためのコンテナクラス
	class EntityContainer
	{
	private:
		std::vector<size_t> entities;
		std::vector<size_t> entityToIndex;

	public:
		// エンティティを追加
		inline void Add(const size_t a_entity)
		{
			entities.emplace_back(a_entity);

			if (entityToIndex.size() <= a_entity)
			{
				entityToIndex.resize(a_entity + 1);
			}

			entityToIndex[a_entity] = entities.size() - 1;
		}
		// エンティティを削除
		inline void Remove(const size_t a_entity)
		{
			if (entityToIndex.size() < a_entity)
			{
				return;
			}

			size_t backIndex = entities.size() - 1;
			size_t backEntity = entities.back();
			size_t removeIndex = entityToIndex[a_entity];

			// 削除する要素が最後の要素でなければ
			if (a_entity != entities.back())
			{
				entities[removeIndex] = backEntity;
				entityToIndex[backIndex] = removeIndex;
			}

			// 最後尾のEntityを削除
			entities.pop_back();
		}

		inline const std::vector<size_t>& GetEntities()const noexcept
		{
			return entities;
		}
	};

	// ComponentPoolを同じコンテナで扱うための基底クラス
	class IComponentPool
	{
	public:
	private:
	};

	// コンポーネントを管理するコンテナクラス
	template<typename CompType>
	class ComponentPool :public IComponentPool
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
			components.reserve(a_size);
		}
		// コンポーネントを追加
		inline CompType* AddComponent(const size_t a_entity)
		{
			if (components.size() <= a_entity)
			{
				components.resize(a_entity + 1, CompType());
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
	};

	// 特定のコンポーネントの組み合わせを持つエンティティIDのリストを取得する
	template<typename ...CompType>
	std::vector<size_t> GetEntitiesWithComponents() const
	{
		std::vector<size_t> resultEntities;

		// 検索したいコンポーネントのアーキタイプを作成
		Archetype searchArch;
		(searchArch.set(ComponentPool<CompType>::GetID()), ...); // パック展開でビットをセット

		// 全ての既存のアーキタイプを反復処理
		for (const auto& pair : archToEntities) // pair.first は Archetype, pair.second は EntityContainer
		{
			// 現在のアーキタイプが検索したいアーキタイプを「含む」かチェック
			// つまり、要求されたコンポーネントをすべて持っているか
			if ((pair.first & searchArch) == searchArch)
			{
				// 条件を満たすアーキタイプに属するエンティティIDのリストを取得
				// これらのエンティティはすべて有効であると仮定するか、ここで有効性もチェック
				for (size_t entityId : pair.second.GetEntities())
				{
					// エンティティが実際に有効なエンティティであるか最終チェック
					if (entityId < entityToActive.size() && entityToActive[entityId]) {
						resultEntities.push_back(entityId);
					}
				}
			}
		}
		return resultEntities;
	}

private:

	// コンポーネントをタイプ別で管理するコンテナ
	std::unordered_map<size_t, std::shared_ptr<IComponentPool>> typeToComponents;

	// エンティティとアーキタイプを紐づけるコンテナ
	std::vector<Archetype> entityToArchetype;

	// エンティティと有効フラグを紐づけるコンテナ
	std::vector<bool> entityToActive;

	// エンティティをアーキタイプごとに分割したコンテナ
	std::unordered_map<Archetype, EntityContainer> archToEntities;

	// 次に生成するエンティティのID
	size_t nextID = 0;

	// 再利用待ちのEntityのリスト
	std::vector<size_t> recycleEntities;

	// 次に発行するCompTypeID
	static size_t nextCcompTypeID;
};

// スタティック変数の初期化
inline size_t ECSManager::nextCcompTypeID = 0;

template<typename CompType>
inline size_t ECSManager::ComponentPool<CompType>::compTypeID = 0;