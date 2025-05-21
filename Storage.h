#pragma once
#include<bitset>
#include<unordered_map>
#include<mutex>

#include"EnumList.h"
#include"Light.h"
#include"Camera.h"
#include"Model.h"
#include"UI.h"

enum IMAGE
{
	IMAGETEST = 0,
};

class ImageData;

//�ǂݍ��񂾉摜��gltf���f���Ȃǂ��L�^���Ă����N���X
//�摜��gltf���f���Ȃǂ̃��\�[�X�����߂�ꂽ�ꍇ�A�܂����̃N���X�ɂ��łɊi�[����Ă��Ȃ����m�F������
class Storage
{
private:
	std::mutex mtx;

	//gltf���f�����i�[�����
	std::unordered_map<std::string, std::shared_ptr<GltfModel>> gltfModelStorage;
	//�t�@�C�������L�[�Ƃ��āA�摜�f�[�^���i�[����
	std::unordered_map<std::string, std::shared_ptr<ImageData>> imageDataStorage;

	//VulkanBase����A�N�Z�X�ł���悤��Scene�N���X�̃J�����ւ̎Q�Ƃ���������A�ȉ����l
	std::shared_ptr<Camera> camera;
	
	//���[�h��ʗp��UI
	std::shared_ptr<UI> loadUI;

	Storage();
	~Storage()
	{
		cleanup();
	}
	static Storage* storage;

	void cleanup();
public:

	static Storage* GetInstance()
	{
		if (!storage)
		{
			storage = new Storage();
		}

		return storage;
	}

	//���[�hUI�̐ݒ�
	void setLoadUI(std::shared_ptr<UI> ui) { loadUI = ui; }
	//���[�hUI��Ԃ�
	std::shared_ptr<UI>getLoadUI() { return loadUI; }

	//gltf���f����ǂݍ��񂾍ۂɁA���̃N���X�Ɋi�[����B
	//�Ăт���gltf���f�����K�v�ɂȂ����ꍇ�́A���̃N���X����Q�Ǝ擾����
	void addModel(std::string obj, std::shared_ptr<GltfModel> geo);
	//��Ɠ��l�A�摜��ǂݍ��񂾍ۂɂ��̃N���X�Ɋi�[����
	void addImageData(std::string, ImageData* image);

	//VulkanBase����J�����ɃA�N�Z�X�o��悤�ɁAScene�N���X�̃J������ݒ肷��
	void setCamera(std::shared_ptr<Camera> c);

	//���̃N���X�ɂ��łɊi�[���ꂽgltfModel��Ԃ�
	std::shared_ptr<GltfModel> getgltfModel(std::string obj);
	//���̃N���X�ɂ��łɊi�[���ꂽ�摜��Ԃ�
	std::shared_ptr<ImageData> getImageData(std::string path);
	//���̃N���X�ɂ��łɊi�[���ꂽgltfModel��map��Ԃ�
	std::unordered_map<std::string, std::shared_ptr<GltfModel>>& getgltfModel();
	//���̃N���X�ɂ��łɊi�[���ꂽimageData��map��Ԃ�
	std::unordered_map<std::string, std::shared_ptr<ImageData>>& getImageData();

	//�J�����ւ̎Q�Ƃ�Ԃ�
	std::shared_ptr<Camera> accessCamera();

	//���߂�ꂽ���\�[�X�����łɂ��̃N���X�Ɋi�[����Ă��邩�ǂ�����Ԃ�
	//���̊֐��ł́A���̔���݂̂�S��
	bool containModel(std::string obj);
	bool containImageData(std::string path);

	//�f�X�g���N�^
	static void FinishStorage()
	{
		if (storage)
		{
			delete storage;
		}
	}
};