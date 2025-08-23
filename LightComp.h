#pragma once

#include<iostream>
#include<glm/glm.hpp>

#include"GpuBufferFactory.h"
#include"DescriptorSetFactory.h"
#include"TextureFactory.h"

//�e��ނ̃��C�g�̍ő��
#define LIGHT_MAX 10
//���C�g�̎�ނ̐�
#define LIGHT_TYPE_COUNT 2

struct PointLightComp
{
	//�V�[���S�̂̃��C�g�̔z��̃C���f�b�N�X
	//����index���z��̏�̔ԍ�
	int index;

	glm::vec3 position;
	glm::vec4 color;
};

struct DirectionLightComp
{
	int index;

	glm::vec3 direction;
	glm::vec4 color;
};

struct PointLightUniform
{
	uint32_t lightCount;
	alignas(16) std::array<glm::vec4, LIGHT_MAX> position;
	alignas(16) std::array<glm::vec4, LIGHT_MAX> color;
	alignas(16) std::array<glm::mat4, LIGHT_MAX> viewProj;
};

struct DirectionLightUniform
{
	uint32_t lightCount;
	alignas(16) std::array<glm::vec4, LIGHT_MAX> direction;
	alignas(16) std::array<glm::vec4, LIGHT_MAX> color;
	alignas(16) std::array<glm::mat4, LIGHT_MAX> viewProj;
};

struct SceneLight
{
	int dirIndex;
	int pointIndex;
	DirectionLightUniform dirUniform;
	PointLightUniform pointUniform;

	//���C�g�̎�ނ̐������z��̗v�f��������
	//�e���C�g�̍s��Ȃǂ́A���j�t�H�[���o�b�t�@�̔z��ɓ����
	std::array<std::shared_ptr<GpuBuffer>, LIGHT_TYPE_COUNT> uniformBuffer;

	//���C�g�̎�ނ̐������z��̗v�f����������
	std::array<std::shared_ptr<DescriptorSet>, LIGHT_TYPE_COUNT> uniformDescriptorSet;

	//���C�g�̎�ނ̐��ƃ��C�g�̐������v�f����������
	//�����A���ۂɂ͓���̎�ނ̃��C�g�̏ꍇ�A����̃e�N�X�`���ŊǗ�����
	//�e���C�g�͈�̃e�N�X�`���̎������̃��C�������蓖�Ă��
	//�����Ɋe���C�g���_�̃V���h�E�}�b�v���L�^�����
	std::array<std::vector<std::shared_ptr<FrameBuffer>>, LIGHT_TYPE_COUNT> frameBuffer;

	//���C�g�̎�ނ̐������v�f������
	//�����ނ̃��C�g�̃V���h�E�}�b�v�́A���̈�̃e�N�X�`����
	//���蓖�Ă�ꂽ���C���ɋL�^�����
	std::array<std::shared_ptr<Texture>, LIGHT_TYPE_COUNT> shadowMap;

	//���C�g�͂܂Ƃ߂Ĉ��VkDescriptorSet�ŊǗ�����
	std::shared_ptr<DescriptorSet> texDescriptorSet;

	SceneLight()
	{
		dirIndex = 0;
		pointIndex = 0;

		for (auto buffer : uniformBuffer)
		{
			buffer.reset();
		}

		for (auto desc : uniformDescriptorSet)
		{
			desc.reset();
		}

		for (auto frameArray : frameBuffer)
		{
			for (auto frame : frameArray)
			{
				frame.reset();
			}
		}

		for (auto tex : shadowMap)
		{
			tex.reset();
		}

		texDescriptorSet.reset();
	}

	int getPointLightIndex()
	{
		int index = pointIndex;
		
		pointIndex++;

		return index;
	}

	int getDirectionLightIndex()
	{
		int index = dirIndex;

		dirIndex++;

		return index;
	}
};