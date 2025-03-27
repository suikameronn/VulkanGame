#pragma once

#include<iostream>

#include<glm/glm.hpp>
#include<glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include"VulkanBase.h"

#define PI 3.14159265359

//��]�̍\����
//�e���ɓx���@�P�ʂ̉�]�p�x���w�肷��
struct Rotate
{
	float z;

	//���W�A���֕ϊ�
	float getRadian(float deg)
	{
		return static_cast<float>(deg * (PI / 180.0f));
	}

	//��]�s��̎擾
	glm::mat3 getRotateMatrix()
	{
		glm::mat4 mat = glm::rotate(glm::mat4(1.0f), getRadian(z), glm::vec3(0.0f, 0.0f, 1.0f));

		return glm::mat3(mat);
	}
};

class UI
{
protected:
	//��ʏ��ui�̍��W
	glm::vec2 position;

	//ui�̉�]�̏��
	Rotate rotate;

	//ui�̐L�k
	glm::vec2 scale;

	//���W�ϊ��s��


	//ui�̕\����\��
	bool isVisible;

	//�����`�̒��_
	std::array<glm::vec2, 4> vertices;

	//uv���W
	std::array<glm::vec2, 4> uv;

	//�����`�̃C���f�b�N�X
	std::array<uint32_t, 6> indices;

	//ui�̃e�N�X�`���𒣂�t���邽�߂̒����`�̃o�b�t�@
	BufferObject pointBuffer;

	//ui�̃e�N�X�`��
	TextureData* uiTexture;

public:

	UI();

	//���W�̐ݒ�
	void setPosition(glm::vec2 pos);
	//��]�̐ݒ�
	void setRotate(Rotate rot);
	//�L�k�̐ݒ�
	void setScale(glm::vec2 scale);

	//�\����\���̐ݒ� �����ɂ���Đݒ�
	void setVisible(bool visible);

	//���_�z��̎擾
	glm::vec2* getVertices();

	//uv���W�̎擾
	glm::vec2* getUV();

	//�C���f�b�N�X�z��̎擾
	uint32_t* getIndices();

	//gpu��̒��_�Ɋւ���o�b�t�@���擾
	BufferObject& getPointBuffer();

	//ui�Ƃ��Č�����e�N�X�`���̎擾
	TextureData* getUiTexture();
};