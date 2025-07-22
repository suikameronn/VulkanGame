#pragma once

#include<iostream>

#include"GltfModel.h"

#include"MaterialBuilder.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

class GltfModelFactory
{
private:

	//�}�e���A����ǂݍ��݂���
	std::shared_ptr<MaterialBuilder> materialBuilder;
	//�e�N�X�`�����쐬
	std::shared_ptr<TextureFactory> textureFactory;

	//���[�h����gltf���f����ۑ�����}�b�v
	std::unordered_map<uint32_t, std::shared_ptr<GltfModel>> modelStorage;

	//�R���C�_�[�p��AABB�̍��W�����߂�
	glm::vec3 minPos;
	glm::vec3 maxPos;

	//���f���S�̂̒��_���ƃC���f�b�N�X��
	int vertexNum;
	int indexNum;

	//�t�@�C���p�X����n�b�V���l���v�Z����
	uint32_t hashFilePath(const std::string& filePath)
	{
		std::hash<std::string> hasher;
		return static_cast<uint32_t>(hasher(filePath));
	}

	//gltf���f���̃m�[�h���ċA�I�ɓǂݍ���
	void loadNode(GltfNode* current, std::shared_ptr<GltfModel> model
		, const tinygltf::Node& gltfNode, uint32_t nodeIndex, const tinygltf::Model& gltfModel);

	//gltf���f���̃��b�V����ǂݍ���
	void loadMesh(const tinygltf::Node& gltfNode, const tinygltf::Model& gltfModel
		, GltfNode* currentNode, std::shared_ptr<GltfModel> model, int meshIndex);

	//�v���~�e�B�u�̓ǂݎ��
	void loadPrimitive(Mesh* mesh, int& indexStart
		, tinygltf::Primitive glPrimitive, tinygltf::Model glModel, std::shared_ptr<GltfModel> model);

	//�A�j���[�V������ǂݍ���
	void loadAnimations(std::shared_ptr<GltfModel> model
		, const tinygltf::Scene& scene, const tinygltf::Model& gltfModel);

	//�X�L����ǂݍ���
	void loadSkin(std::shared_ptr<GltfModel> model, tinygltf::Model gltfModel);

	//�X�L����ݒ肷��
	void setSkin(GltfNode* node, std::shared_ptr<GltfModel> model);

	//�}�e���A����ǂݍ���
	void loadMaterial(std::shared_ptr<GltfModel> model, tinygltf::Model& gltfModel);

public:

	GltfModelFactory(std::shared_ptr<MaterialBuilder> builder
		,std::shared_ptr<TextureFactory> texture)
	{
		materialBuilder = builder;

		textureFactory = texture;
	}
	
	uint32_t Load(const std::string& filePath);

	std::shared_ptr<GltfModel> GetModel(uint32_t hash)
	{
		if (modelStorage.find(hash) != modelStorage.end())
		{
			return modelStorage[hash];
		}

		return nullptr;
	}

};