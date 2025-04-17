#pragma once

#include<iostream>
#include<array>
#include <numeric> 

#include<glm/glm.hpp>
#include<glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include<vulkan/vulkan.h>

#include"Model.h"

#define PI 3.14159265359

#define UIVertexCount 4
#define UIIndexCount 6

//2D�p�̒��_�\����
struct Vertex2D
{
	glm::vec2 pos;
	glm::vec2 uv;
};

//��]�̍\����
//�e���ɓx���@�P�ʂ̉�]�p�x���w�肷��
struct Rotate2D
{
	float z;

	//���W�A���֕ϊ�
	float getRadian(float deg)
	{
		return static_cast<float>(deg * (PI / 180.0f));
	}

	//��]�s��̎擾
	glm::mat4 getRotateMatrix()
	{
		glm::mat4 mat = glm::rotate(glm::mat4(1.0f), getRadian(z),glm::vec3(0.0f,0.0f,1.0f));

		return mat;
	}
};

class UI
{
protected:

	//UI�̍��W�̃I�t�Z�b�g��ݒ肷��
	int widthOffset, heightOffset;

	//UI�p�̃v���W�F�N�V�����s��
	glm::mat4 projMatrix;

	//���W�ϊ��s��̍X�V�t���b�O
	bool isUpdateTransformMatrix;

	//��ʏ��ui�̍��W
	glm::vec2 position;

	//ui�̉�]�̏��
	Rotate2D rotate;

	//ui�̐L�k
	float scale;

	//���W�ϊ��s��
	glm::mat4 transformMatrix;

	//ui�摜�̃A�X�y�N�g��
	glm::vec2 aspect;

	//ui�̕��ƍ���
	float uiWidth, uiHeight;

	//ui�̕\����\��
	bool isVisible;

	//�����`�̒��_
	std::array<Vertex2D, UIVertexCount> vertices;

	//�����`�̃C���f�b�N�X
	std::array<uint32_t, 6> indices;

	//ui�̃e�N�X�`���𒣂�t���邽�߂̒����`�̃o�b�t�@
	BufferObject pointBuffer;

	//uniform buffer�p�̃o�b�t�@
	MappedBuffer mappedBuffer;

	//UI�p�̉摜�����ѕt�����Ă���
	VkDescriptorSet descriptorSet;

	//�摜�f�[�^(ui�C���X�g�Ȃ�)
	std::shared_ptr<ImageData> uiImage;

	//ui�̃e�N�X�`��
	TextureData* uiTexture;

public:

	UI(std::shared_ptr<ImageData> image);

	const uint32_t vertexCount = UIVertexCount;
	const uint32_t indexCount = UIIndexCount;

	//���W�̐ݒ�
	void setPosition(glm::vec2 pos);
	//��]�̐ݒ�
	void setRotate(Rotate2D rot);
	//�L�k�̐ݒ�
	void setScale(float scale);

	//�e�N�X�`���̕��ƍ������擾
	float getTexWidth() { return uiWidth; }
	float getTexHeight() { return uiHeight; }

	//�\����\���̐ݒ� �����ɂ���Đݒ�
	void setVisible(bool visible);

	//���_�z��̎擾
	Vertex2D* getVertices();

	//�C���f�b�N�X�z��̎擾
	uint32_t* getIndices();

	//gpu��̒��_�Ɋւ���o�b�t�@���擾
	BufferObject& getPointBuffer();

	//uniform buffer���擾
	MappedBuffer& getMappedBuffer() { return mappedBuffer; }

	//descriptroSet�̎擾
	VkDescriptorSet& getDescriptorSet() { return descriptorSet; }

	//ui�Ƃ��Č�����e�N�X�`���̎擾
	TextureData* getUITexture();

	//�v���W�F�N�V�����s���Ԃ�
	glm::mat4 getProjectMatrix() { return projMatrix; }

	//ui�摜��Ԃ�
	std::shared_ptr<ImageData> getImageData();

	//�e�N�X�`���摜�̐ݒ�ƃe�N�X�`���̍쐬
	void createTexture(std::shared_ptr<ImageData> image);

	//���W�ϊ��s��̍X�V
	void updateTransformMatrix();

	//���W�ϊ��s���Ԃ�
	glm::mat4 getTransfromMatrix() { return transformMatrix; }

	//gpu��̃o�b�t�@�Ȃǂ�j��
	void cleanupVulkan();
};