#pragma once

#include"ECSManager.h"

// �R���|�[�l���g���Ǘ�����R���e�i�N���X
template<typename CompType>
class ComponentPool
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
		components.resize(a_size);
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
};