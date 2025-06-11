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

//UI�̕\���D��x������ �~��
enum class UILayer
{
	MENU = 0,//�őO��
	INGAME,
	BACKGROUND
};

//2D�p�̒��_�\����
struct Vertex2D
{
	//z�l�́AUI�������_�C���O����ۂ̗D�揇�ʂɎg�p
	glm::vec3 pos;
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

//UI�p��2D�s��
struct MatricesUBO2D
{
	glm::mat4 transformMatrix;
	glm::mat4 projection;
};

class UI : public std::enable_shared_from_this<UI>
{
protected:

	bool initFrame;

	//UI��\�����郌�C���[
	UILayer layer;

	//�����`�̃C���f�b�N�X
	std::vector<uint32_t> indices;

	//�����`�̒��_
	std::vector<Vertex2D> vertices;

	//ui�摜�̃A�X�y�N�g��
	glm::vec2 aspect;

	//ui�̕��ƍ���
	float uiWidth, uiHeight;

	//���W�ϊ��s��
	glm::mat4 transformMatrix;

	//UI�̃^�C�v
	UINum uiNum;

	//ui�̐L�k
	float scale;

	//UI�̑��݃t���b�O
	bool exist;

	//�������`��̃t���b�O
	bool transparent;

	//UI�p�̃v���W�F�N�V�����s��
	glm::mat4 projMatrix;

	//���W�ϊ��s��̍X�V�t���b�O
	bool isUpdateTransformMatrix;

	//��ʏ��ui�̍��W
	glm::vec2 position;

	//ui�̉�]�̏��
	Rotate2D rotate;

	//ui�̕\����\��
	bool isVisible;

	//ui�̃e�N�X�`���𒣂�t���邽�߂̒����`�̃o�b�t�@
	std::array<BufferObject, 2> pointBuffers;

	//UI�p�̉摜�����ѕt�����Ă���
	VkDescriptorSet transformDescriptorSet;
	VkDescriptorSet imageDescriptorSet;

	//uniform buffer�p�̃o�b�t�@
	MappedBuffer mappedBuffer;

	//�摜�f�[�^(ui�C���X�g�Ȃ�)
	std::shared_ptr<ImageData> uiImage;

	void updateUniformBuffer();

	virtual void cleanupVulkan();

public:

	UI() {};
	UI(std::shared_ptr<ImageData> image);
	~UI()
	{
		//gpu��̃o�b�t�@�Ȃǂ�j��
		cleanupVulkan();
	}

	UINum getUINum()
	{
		return uiNum;
	}

	bool isInitFrame()
	{
		return initFrame;
	}

	bool isExist()
	{
		return exist;
	}

	bool isTransparent()
	{
		return transparent;
	}

	void setTransparent(bool t)
	{
		transparent = t;
	}

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
	bool getVisible() { return isVisible; }

	uint32_t getVerticesSize() { return static_cast<uint32_t>(vertices.size()); }
	uint32_t getIndicesSize() { return static_cast<uint32_t>(indices.size()); }

	//���_�z��̎擾
	Vertex2D* getVertices();

	//�C���f�b�N�X�z��̎擾
	uint32_t* getIndices();

	//gpu��̃o�b�t�@�̐�
	const int getPointBufferSize() { return static_cast<int>(pointBuffers.size()); }

	//gpu��̒��_�Ɋւ���o�b�t�@���擾
	BufferObject* getPointBuffer();

	//uniform buffer���擾
	MappedBuffer& getMappedBuffer() { return mappedBuffer; }

	//descriptroSet�̎擾
	VkDescriptorSet& getTransformDescriptorSet() { return transformDescriptorSet; }
	virtual VkDescriptorSet& getImageDescriptorSet() { return imageDescriptorSet; }

	//ui�Ƃ��Č�����e�N�X�`���̎擾
	TextureData* getUITexture();

	//�v���W�F�N�V�����s���Ԃ�
	glm::mat4 getProjectMatrix() { return projMatrix; }

	//ui�摜��Ԃ�
	std::shared_ptr<ImageData> getImageData();

	//�e�N�X�`���摜�̐ݒ�ƃe�N�X�`���̍쐬
	void createTexture(std::shared_ptr<ImageData> image);

	//���W�ϊ��s��̍X�V
	virtual void updateTransformMatrix();

	//�����t���[���̎��Ɏ��s����
	virtual void initFrameSettings();

	//�t���[���I�����Ɏ��s����
	void frameEnd();

	//���W�ϊ��s���Ԃ�
	glm::mat4 getTransfromMatrix() { return transformMatrix; }
};