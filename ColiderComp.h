#pragma once

struct ColiderComp
{
	//�R���C�_�[�̎��
	enum class ColliderType
	{
		Box
	};

	//�R���C�_�[�̃^�C�v
	ColliderType type;

	//�R���C�_�[�̃T�C�Y�┼�a�Ȃǂ̃p�����[�^
	//���݂̓{�b�N�X�̂�
	//���̒l�́A3D���f���̃X�P�[���ɂ���ɁA�|�����킳���
	//���ۂ�3D���f�������R���C�_�[�̂ݑ傫���������Ƃ��悤
	float size;

	ColiderComp()
	{
		type = ColliderType::Box;
		size = 1.0f;
	};
};