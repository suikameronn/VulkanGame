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
	// �G���e�B�e�B��V�K�쐬
	inline const size_t GenerateEntity()
	{
		size_t nEntity;
		// ���T�C�N���҂��G���e�B�e�B������΂�������1���o��
		if (recycleEntities.size())
		{
			nEntity = recycleEntities.back();
			recycleEntities.pop_back();
		}
		// ������ΐV�K���s
		else
		{
			nEntity = ++nextID;
		}
		// �G���e�B�e�B��L���ɂ���
		if (entityToActive.size() < nEntity)
		{
			entityToActive.resize(nEntity + 1, false);
		}

		entityToActive[nEntity] = true;

		// ���������G���e�B�e�B��Ԃ�
		return nEntity;
	}

	// �R���|�[�l���g��ǉ�����
	template<typename CompType>
	CompType* AddComponent(const size_t a_entity)
	{
		// �R���|�[�l���g�̌^��ID���擾
		size_t type = ComponentPool<CompType>::GetID();

		// ���߂Đ�������R���|�[�l���g�Ȃ�
		if (!typeToComponents[type])
		{
			// ComponentPool�����̉�����
			typeToComponents[type] = std::make_shared<ComponentPool<CompType>>(4096);
		}

		// �ǉ�����R���|�[�l���g���i�[����R���e�i�N���X���擾
		std::shared_ptr<ComponentPool<CompType>> spCompPool = std::static_pointer_cast<ComponentPool<CompType>>(typeToComponents[type]);

		// �R���|�[�l���g��ǉ����擾
		CompType* pResultComp = spCompPool->AddComponent(a_entity);

		// �R���|�[�l���g��ǉ�����O�̃A�[�L�^�C�v���擾
		Archetype arch;
		if (entityToArchetype.size() > a_entity)
		{
			arch = entityToArchetype[a_entity];
		}
		else
		{
			entityToArchetype.resize(a_entity + 1, Archetype());
		}

		// �O�A�[�L�^�C�v�̃G���e�B�e�B���X�g����G���e�B�e�B���폜
		archToEntities[arch].Remove(a_entity);

		// �A�[�L�^�C�v��ҏW
		arch.set(type);

		// �V�����A�[�L�^�C�v���Z�b�g
		archToEntities[arch].Add(a_entity);
		entityToArchetype[a_entity] = arch;

		// �ǉ������R���|�[�l���g��Ԃ�
		return pResultComp;
	}

	// �R���|�[�l���g���폜����
	template<typename CompType>
	void RemvoeComponent(const size_t a_entity)
	{
		// �R���|�[�l���g�̌^��ID���擾
		size_t type = ComponentPool<CompType>::GetID();

		Archetype arch;
		// �R���|�[�l���g���폜����O�̃A�[�L�^�C�v���擾
		if (entityToArchetype.size() > a_entity)
		{
			arch = entityToArchetype[a_entity];
		}
		else
		{
			return;
		}
		// �G���e�B�e�B���폜����R���|�[�l���g�������ĂȂ�������return
		if (!arch.test(type))
		{
			return;
		}
		// �O�A�[�L�^�C�v�̃G���e�B�e�B���X�g����G���e�B�e�B���폜
		archToEntities[arch].Remove(a_entity);

		// �A�[�L�^�C�v��ҏW
		arch.reset(type);

		// �V�����A�[�L�^�C�v���Z�b�g
		archToEntities[arch].Add(a_entity);
		entityToArchetype[a_entity] = arch;
	}

	// �G���e�B�e�B�𖳌��ɂ���
	inline void RemoveEntity(const size_t a_entity)
	{
		entityToActive[a_entity] = false;
		archToEntities[entityToArchetype[a_entity]].Remove(a_entity);
		recycleEntities.emplace_back(a_entity);
	}

	// ���������s����
	template<typename ...CompType>
	void RunFunction(std::function<void(CompType&...)> a_func)
	{
		// �����ɕK�v�ȃR���|�[�l���g�̃A�[�L�^�C�v���擾
		Archetype arch;
		(arch.set(ComponentPool<CompType>::GetID()), ...);
		// �����ɕK�v�ȃA�[�L�^�C�v���܂ރA�[�L�^�C�v�����G���e�B�e�B�̃��X�g������
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

	// �G���e�B�e�B���Ǘ����邽�߂̃R���e�i�N���X
	class EntityContainer
	{
	private:
		std::vector<size_t> entities;
		std::vector<size_t> entityToIndex;

	public:
		// �G���e�B�e�B��ǉ�
		inline void Add(const size_t a_entity)
		{
			entities.emplace_back(a_entity);

			if (entityToIndex.size() < a_entity)
			{
				entityToIndex.resize(a_entity + 1);
			}

			entityToIndex[a_entity] = entities.size() - 1;
		}
		// �G���e�B�e�B���폜
		inline void Remove(const size_t a_entity)
		{
			if (entityToIndex.size() < a_entity)
			{
				return;
			}

			size_t backIndex = entities.size() - 1;
			size_t backEntity = entities.back();
			size_t removeIndex = entityToIndex[a_entity];

			// �폜����v�f���Ō�̗v�f�łȂ����
			if (a_entity != entities.back())
			{
				entities[removeIndex] = backEntity;
				entityToIndex[backIndex] = removeIndex;
			}

			// �Ō����Entity���폜
			entities.pop_back();
		}

		inline const std::vector<size_t>& GetEntities()const noexcept
		{
			return entities;
		}
	};

	// ComponentPool�𓯂��R���e�i�ň������߂̊��N���X
	class IComponentPool
	{
	public:
	private:
	};

	// �R���|�[�l���g���Ǘ�����R���e�i�N���X
	template<typename CompType>
	class ComponentPool :public IComponentPool
	{
	private:

		friend class ECSManager;

		// �R���|�[�l���g�̃C���X�^���X���Ǘ�����R���e�i
		std::vector<CompType> components;

		// ���̃R���e�i�N���X���Ǘ�����R���|�[�l���g������ӂ�ID
		static size_t compTypeID;

	public:
		// �R���e�i�̃��������m��
		ComponentPool(const size_t a_size)
		{
			components.reserve(a_size);
		}
		// �R���|�[�l���g��ǉ�
		inline CompType* AddComponent(const size_t a_entity)
		{
			if (components.size() < a_entity)
			{
				components.resize(a_entity, CompType());
			}
			components[a_entity] = CompType();
			return &components[a_entity];
		}

		// �R���|�[�l���g���擾����
		inline CompType* GetComponent(const size_t a_entity)noexcept
		{
			// �G���e�B�e�B���L���Ȃ�
			if (components.size() >= a_entity)
			{
				return &components[a_entity];
			}
			return nullptr;
		}

		// CompTypeID���擾����֐�
		static inline const size_t GetID()
		{
			// ���̊֐������߂ēǂ񂾎���ID�𔭍s
			if (!ComponentPool<CompType>::compTypeID)
			{
				ComponentPool<CompType>::compTypeID = ++ECSManager::nextCcompTypeID;
			}
			return ComponentPool<CompType>::compTypeID;
		}
	};

	// ����̃R���|�[�l���g�̑g�ݍ��킹�����G���e�B�e�BID�̃��X�g���擾����
	template<typename ...CompType>
	std::vector<size_t> GetEntitiesWithComponents() const
	{
		std::vector<size_t> resultEntities;

		// �����������R���|�[�l���g�̃A�[�L�^�C�v���쐬
		Archetype searchArch;
		(searchArch.set(ComponentPool<CompType>::GetID()), ...); // �p�b�N�W�J�Ńr�b�g���Z�b�g

		// �S�Ă̊����̃A�[�L�^�C�v�𔽕�����
		for (const auto& pair : archToEntities) // pair.first �� Archetype, pair.second �� EntityContainer
		{
			// ���݂̃A�[�L�^�C�v�������������A�[�L�^�C�v���u�܂ށv���`�F�b�N
			// �܂�A�v�����ꂽ�R���|�[�l���g�����ׂĎ����Ă��邩
			if ((pair.first & searchArch) == searchArch)
			{
				// �����𖞂����A�[�L�^�C�v�ɑ�����G���e�B�e�BID�̃��X�g���擾
				// �����̃G���e�B�e�B�͂��ׂėL���ł���Ɖ��肷�邩�A�����ŗL�������`�F�b�N
				for (size_t entityId : pair.second.GetEntities())
				{
					// �G���e�B�e�B�����ۂɗL���ȃG���e�B�e�B�ł��邩�ŏI�`�F�b�N
					if (entityId < entityToActive.size() && entityToActive[entityId]) {
						resultEntities.push_back(entityId);
					}
				}
			}
		}
		return resultEntities;
	}

private:

	// �R���|�[�l���g���^�C�v�ʂŊǗ�����R���e�i
	std::unordered_map<size_t, std::shared_ptr<IComponentPool>> typeToComponents;

	// �G���e�B�e�B�ƃA�[�L�^�C�v��R�Â���R���e�i
	std::vector<Archetype> entityToArchetype;

	// �G���e�B�e�B�ƗL���t���O��R�Â���R���e�i
	std::vector<bool> entityToActive;

	// �G���e�B�e�B���A�[�L�^�C�v���Ƃɕ��������R���e�i
	std::unordered_map<Archetype, EntityContainer> archToEntities;

	// ���ɐ�������G���e�B�e�B��ID
	size_t nextID = 0;

	// �ė��p�҂���Entity�̃��X�g
	std::vector<size_t> recycleEntities;

	// ���ɔ��s����CompTypeID
	static size_t nextCcompTypeID;
};