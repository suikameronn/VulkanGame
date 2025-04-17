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

#include<limits>
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include"font.h"

enum IMAGE;

//�摜��gltf���f���̓ǂݍ��݂�S������N���X
class FileManager
{
private:
	static FileManager* fileManager;

	FileManager();

	//���ߍ��߂�ꂽgltf���f�����擾����
	int getModelResource(GLTFOBJECT obj);

	//�R���C�_�[�̌v�Z�p�ϐ�
	glm::vec3 minPos;
	glm::vec3 maxPos;
	int vertexNum;
	int indexNum;

	//std::vector<GLTFOBJECT> loadAnimationFiles;

	//�^����ꂽ�����񂩂�t�@�C���̖��O�݂̂��擾����
	std::string splitFileName(std::string filePath);

	//gltf���f���̓ǂݍ��݂��J�n����
	GltfModel* loadGLTFModel(const tinygltf::Scene& scene,const tinygltf::Model& gltfModel);
	//gltf���f���̃m�[�h���ċA�I�ɓǂݍ���
	void loadNode(GltfNode* parent,GltfNode* current, GltfModel* model, const tinygltf::Node& gltfNode, uint32_t nodeIndex, const tinygltf::Model& gltfModel, float globalscale);
	//gltf���f���̃��b�V����ǂݍ���
	void processMesh(const tinygltf::Node& gltfNode, const tinygltf::Model gltfModel, GltfNode* currentNode, GltfModel* model);
	//AABB�̂��߂�gltf���f���̒��_��xyz�̊e���̍ŏ��l�ƍő�l�����߂�
	void calcMinMaxVertexPos(glm::vec3 min,glm::vec3 max);
	//gltf���f���̃v���~�e�B�u�P�ʂœǂݍ���
	void processPrimitive(Mesh* mesh, int& indexStart, tinygltf::Primitive glPrimitive, tinygltf::Model glModel, GltfModel* model);
	//gltf���f���̃A�j���[�V������ǂݍ���
	void loadAnimations(GltfModel* model, const tinygltf::Scene& scene, const tinygltf::Model& gltfModel);
	//gltf���f���̃X�P���g����ǂݍ���
	void loadSkin(GltfModel* model, tinygltf::Model gltfModel);
	//���O��Model�N���X�ɃX�P���g����ݒ肷��
	void setSkin(GltfNode* node, GltfModel* model);

	//�e�N�X�`����ǂݍ���
	void loadTextures(GltfModel* model,const tinygltf::Model gltfModel);
	//�}�e���A�������b�V���ɐݒ肷��
	void loadMaterial(GltfModel* model,tinygltf::Model gltfModel);
	//���ߍ��܂ꂽgltf���f�����擾����
	void loadgltfModel(int id, void** ptr, int& size);

	//gif�摜��ǂݍ���
	void loadGif(std::string gifFilName);

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

	//lua�X�N���v�g����Scene�N���X����ČĂяo�����B�V�������f�������߂�ꂽ�Ƃ��̂݁A��͏���������
	std::shared_ptr<GltfModel> loadModel(std::string modelPath);
	//lua�X�N���v�g����Scene�N���X����ČĂяo�����B�V�����摜�����߂�ꂽ�Ƃ��̂݁A��͏���������
	std::shared_ptr<ImageData> loadImage(std::string filePath);	
};