#pragma once

#include<chrono>
#include <thread>

#include<memory>

#include"DescriptorSetFactory.h"
#include"FrameBufferFactory.h"
#include"GpuBufferFactory.h"
#include"DescriptorSetLayoutFactory.h"
#include"PipelineLayoutFactory.h"
#include"PipelineFactory.h"
#include"RenderPassFactory.h"
#include"ShaderFactory.h"
#include"TextureFactory.h"

#include"GltfModelFactory.h"
#include"MaterialBuilder.h"

#include"SwapChain.h"
#include"Render.h"

#include"ECSManager.h"

//fps����Ȃǂ̃Q�[���S�̂̐����S������
class GameManager
{
private:

	static GameManager* gameManager;

	std::shared_ptr<VulkanCore> vulkanCore;

	//ECS�}�l�[�W���[
	std::shared_ptr<ECSManager> ecsManager;

	//�r���_�[

	std::shared_ptr<DescriptorSetBuilder> descriptorSetBuilder;
	std::shared_ptr<FrameBufferBuilder> frameBufferBuilder;
	std::shared_ptr<GpuBufferBuilder> bufferBuilder;
	std::shared_ptr<DescriptorSetLayoutBuilder> descriptorSetLayoutBuilder;
	std::shared_ptr<PipelineLayoutBuilder> pipelineLayoutBuilder;
	std::shared_ptr<PipelineBuilder> pipelineBuilder;
	std::shared_ptr<RenderPassBuilder> renderPassBuilder;
	std::shared_ptr<TextureBuilder> textureBuilder;

	//�t�@�N�g���[

	std::shared_ptr<DescriptorSetFactory> descriptorSetFactory;
	std::shared_ptr<FrameBufferFactory> frameBufferFactory;
	std::shared_ptr<GpuBufferFactory> bufferFactory;
	std::shared_ptr<DescriptorSetLayoutFactory> descriptorSetLayoutFactory;
	std::shared_ptr<PipelineLayoutFactory> pipelineLayoutFactory;
	std::shared_ptr<PipelineFactory> pipelineFactory;
	std::shared_ptr<RenderPassFactory> renderPassFactory;
	std::shared_ptr<ShaderFactory> shaderFactory;
	std::shared_ptr<TextureFactory> textureFactory;

	//�X���b�v�`�F�[���̃N���X
	std::shared_ptr<SwapChain> swapChain;

	//�����_�[�N���X
	std::shared_ptr<Render> render;

	//�}�e���A���r���_�[
	std::shared_ptr<MaterialBuilder> materialBuilder;

	//Gltf���f���t�@�N�g���[
	std::shared_ptr<GltfModelFactory> modelFactory;

	//�ő�t���[�����[�g
	const int fps = 60;
	//�ő�t���[�����[�g���̃t���[���̌o�ߎ���
	float frameDuration;
	//���Ԃ̌v���p�ϐ�
	std::chrono::system_clock::time_point  start, end;
	//�v�������t���[������
	long long elapsed;

	//�C���X�^���X���쐬����
	void createInstance();

	//�r���_�[�̗p��
	void createBuilder();
	//�t�@�N�g���[�̗p��
	void createFactory();

	void setLoadUI();

	//�R���|�[�l���g�̏��񏈗�
	void OnStart();

	//�R���|�[�l���g�̍X�V����
	void OnUpdate();

	//�X�V������̏���
	void OnLateUpdate();

	//�I�u�W�F�N�g�̃����_�����O
	void Rendering();

	//�t���[���I�����̏���
	void OnFrameEnd();

public:

	static GameManager* GetInstance()
	{
		if (!gameManager)
		{
			gameManager = new GameManager();
		}

		return gameManager;
	}

	//�Q�[���̃t���[�����[�g�̐ݒ��X�e�[�W�̓ǂݍ��݂��s��
	void initGame();

	//�V�[���̍쐬
	void createScene();

	//���C�����[�v
	void mainGameLoop();
	//�X�e�[�W����o��
	void exitScene();
	//�Q�[���S�̂̏I������
	void FinishGame();
};