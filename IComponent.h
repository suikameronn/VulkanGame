#pragma once

#include<memory>

class Entity;

class IComponent
{
private:

	std::weak_ptr<Entity> entity;

public:

	IComponent() {};
	~IComponent() {};

	//�R���|�[�l���g���A�^�b�`�������_�Ŏ��s
	virtual void OnAwake() = 0;

	//�A�^�b�`��̎��̃t���[���J�n���_�Ŏ��s
	virtual void OnStart() = 0;

	//�X�V�t�F�[�Y�Ŏ��s
	virtual void OnUpdate() = 0;

	//�X�V�t�F�[�Y��Ɏ��s
	virtual void OnLateUpdate() = 0;

	//�t���[���I�����Ɏ��s
	virtual void OnFrameEnd() = 0;

	//�R���|�[�l���g�̏��L�҂�ݒ肷��
	void setOwner(std::weak_ptr<Entity> e)
	{
		entity = e;

		OnAwake();
	}

	//�R���|�[�l���g�̏��L�҂��擾����
	std::weak_ptr<Entity> getOwner()
	{
		return entity;
	}
};