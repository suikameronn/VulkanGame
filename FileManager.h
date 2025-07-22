#pragma once
#include<iostream>
#include <windows.h>
#include<map>
#include<ostream>
#include<thread>
#include<time.h>

#include "resource1.h"

#include"EnumList.h"
#include"Storage.h"
#include"ThreadPool.h"

#include<limits>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"
#include"StructList.h"

enum IMAGE;

//�摜��gltf���f���̓ǂݍ��݂�S������N���X
class FileManager
{
private:
	static FileManager* fileManager;

	//�X�N���v�g����w�肳�ꂽ3D���f���̃t�@�C���̃��X�g��
	//�������������Model�N���X
	std::unordered_map<std::string,std::vector<Model*>> loadModelList;

	FileManager();

	//�R���C�_�[�̌v�Z�p�ϐ�
	glm::vec3 minPos;
	glm::vec3 maxPos;
	int vertexNum;
	int indexNum;

	//�^����ꂽ�����񂩂�t�@�C���̖��O�݂̂��擾����
	std::string splitFileName(std::string filePath);

	//gltf���f���̓ǂݍ��݂��J�n����
	std::shared_ptr<GltfModel> loadGLTFModel(const tinygltf::Scene& scene,const tinygltf::Model& gltfModel);
	//gltf���f���̃m�[�h���ċA�I�ɓǂݍ���
	void loadNode(GltfNode* parent,GltfNode* current, std::shared_ptr<GltfModel> model, const tinygltf::Node& gltfNode, uint32_t nodeIndex, const tinygltf::Model& gltfModel, float globalscale);
	//gltf���f���̃��b�V����ǂݍ���
	void processMesh(const tinygltf::Node& gltfNode, const tinygltf::Model gltfModel, GltfNode* currentNode, std::shared_ptr<GltfModel> model,int meshIndex);
	//AABB�̂��߂�gltf���f���̒��_��xyz�̊e���̍ŏ��l�ƍő�l�����߂�
	void calcMinMaxVertexPos(glm::vec3 min,glm::vec3 max);
	//gltf���f���̃v���~�e�B�u�P�ʂœǂݍ���
	void processPrimitive(Mesh* mesh, int& indexStart, tinygltf::Primitive glPrimitive, tinygltf::Model glModel, std::shared_ptr<GltfModel> model);
	//gltf���f���̃A�j���[�V������ǂݍ���
	void loadAnimations(std::shared_ptr<GltfModel> model, const tinygltf::Scene& scene, const tinygltf::Model& gltfModel);
	//gltf���f���̃X�P���g����ǂݍ���
	void loadSkin(std::shared_ptr<GltfModel> model, tinygltf::Model gltfModel);
	//���O��Model�N���X�ɃX�P���g����ݒ肷��
	void setSkin(GltfNode* node, std::shared_ptr<GltfModel> model);

	//�e�N�X�`����ǂݍ���
	void loadTextures(std::shared_ptr<GltfModel> model,const tinygltf::Model gltfModel);
	//�}�e���A�������b�V���ɐݒ肷��
	void loadMaterial(std::shared_ptr<GltfModel> model, const tinygltf::Model gltfModel);
	//���ߍ��܂ꂽgltf���f�����擾����
	void loadgltfModel(int id, void** ptr, int& size);

	//lua�X�N���v�g����Scene�N���X����ČĂяo�����B�V�������f�������߂�ꂽ�Ƃ��̂݁A��͏���������
	std::shared_ptr<GltfModel> loadModel(std::string modelPath);

public:
	static FileManager* GetInstance()
	{
		if (!fileManager)
		{
			fileManager = new FileManager();
		}

		return fileManager;
	}

	static void FinishFileManger()
	{
		if (fileManager)
		{
			delete fileManager;
		}
	}

	~FileManager()
	{

		fileManager = nullptr;
	}

	//���Ƃœǂݎ��gltf�t�@�C���Ƃ������������Model�N���X�����X�g�ɉ�����
	void addLoadModelList(std::string filePath, Model* model);

	//Model�N���X��gltf�N���X�𑗂�
	void setGltfModel();

	//lua�X�N���v�g����Scene�N���X����ČĂяo�����B�V�����摜�����߂�ꂽ�Ƃ��̂݁A��͏���������
	std::shared_ptr<ImageData> loadImage(std::string filePath);	
};