#pragma once
#include<bitset>
#include<unordered_map>

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
	//gltf���f�����i�[�����
	std::unordered_map<std::string, std::shared_ptr<GltfModel>> gltfModelStorage;
	//�t�@�C�������L�[�Ƃ��āA�摜�f�[�^���i�[����
	std::unordered_map<std::string, std::shared_ptr<ImageData>> imageDataStorage;

	//VulkanBase����A�N�Z�X�ł���悤��Scene�N���X�̃J�����ւ̎Q�Ƃ���������A�ȉ����l
	std::shared_ptr<Camera> camera;
	//�L���[�u�}�b�s���O�p������
	std::shared_ptr<Model> cubemap;
	//�L���[�u�}�b�s���O�pHDRI�摜
	std::shared_ptr<ImageData> cubemapImage;
	
	//���[�h��ʗp��UI
	std::shared_ptr<UI> loadUI;

	//��ʏ�Ɍ����I�u�W�F�N�g���i�[����
	std::vector<std::shared_ptr<Model>> sceneModelStorage;
	//�V�[�����̃|�C���g���C�g���i�[����
	std::vector<std::shared_ptr<PointLight>> scenePointLightStorage;
	//�V�[�����̕��s�������i�[����
	std::vector<std::shared_ptr<DirectionalLight>> sceneDirectionalLightStorage;
	//UI���L�^����
	std::vector<std::shared_ptr<UI>> uiStorage;

	//�|�C���g���C�g�p��DescriptorSet �F�ƍ��W������uniform buffer
	VkDescriptorSet pointLightDescSet;
	//���s�����p��DescriptorSet �F�ƕ���������uniform buffer
	VkDescriptorSet directionalLightDescSet;

	//���C�g�̃f�[�^�͎�ނ��ƂɈ�̃o�b�t�@�[��ɔz��Ƃ��Ċi�[����
	//�|�C���g���C�g��uniform buffer���i�[���邽�߂̃o�b�t�@�[
	MappedBuffer pointLightsBuffer;
	//���s�������C�g��uniform buffer���i�[���邽�߂̃o�b�t�@�[
	MappedBuffer directionalLightsBuffer;

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
	
	//�L���[�u�}�b�s���O�p��HDRI�摜��ݒ肷��
	void setCubemapTexture(std::shared_ptr<ImageData> image);
	//HDRI�摜��Ԃ��A�L���[�u�}�b�v�쐬����VulkanBase����Ăяo�����
	std::shared_ptr<ImageData> getCubemapImage();
	//�L���[�u�}�b�s���O�p�̗����̂�Ԃ��A�L���[�u�}�b�v�쐬����VulkanBase����Ăяo�����
	std::shared_ptr<Model> getCubeMap() { return cubemap; }
	//��ʏ�̃I�u�W�F�N�g��Ԃ��A�����_�����O����VulkanBase����Ăяo�����
	std::vector<std::shared_ptr<Model>>& getModels() { return sceneModelStorage; }
	//�|�C���g���C�g��Ԃ��A�����_�����O����VulkanBase����Ăяo�����
	std::vector<std::shared_ptr<PointLight>>& getPointLights() { return scenePointLightStorage; }
	//���s������Ԃ��A�����_�����O����VulkanBase����Ăяo�����
	std::vector<std::shared_ptr<DirectionalLight>>& getDirectionalLights() { return sceneDirectionalLightStorage; }
	//�|�C���g���C�g�ƕ��s�����̐����擾����
	int getLightCount() { return static_cast<int>(scenePointLightStorage.size() + sceneDirectionalLightStorage.size()); }
	//UI�̔z���Ԃ�
	std::vector<std::shared_ptr<UI>>& getUI() {return uiStorage; }

	//�|�C���g���C�g�ƕ��s������descriptorSet��Ԃ��B���C�g�͔z��Ƃ��Ă܂Ƃ߂�gpu�ɓn�����߁AdescriptorSet�͎�ނ��ƂɈ��
	VkDescriptorSet& getPointLightDescriptorSet() { return pointLightDescSet; }
	VkDescriptorSet& getDirectionalLightDescriptorSet() { return directionalLightDescSet; }

	//�L���[�u�}�b�s���O�p�̗����̂�ݒ�
	void setCubeMapModel(std::shared_ptr<Model> cube) { cubemap = cube; }

	//gltf���f����ǂݍ��񂾍ۂɁA���̃N���X�Ɋi�[����B
	//�Ăт���gltf���f�����K�v�ɂȂ����ꍇ�́A���̃N���X����Q�Ǝ擾����
	void addModel(std::string obj, GltfModel* geo);
	//��Ɠ��l�A�摜��ǂݍ��񂾍ۂɂ��̃N���X�Ɋi�[����
	void addImageData(std::string, ImageData* image);

	//VulkanBase����J�����ɃA�N�Z�X�o��悤�ɁAScene�N���X�̃J������ݒ肷��
	void setCamera(std::shared_ptr<Camera> c);

	//Scene�N���X��lua�Őݒ肳�ꂽ�I�u�W�F�N�g�͂��̃N���X�Ɋi�[���A���̃N���X����VulkanBase�ł��̃I�u�W�F�N�g�̃����_�����O���s��
	void addModel(std::shared_ptr<Model> model);
	//Model�N���X���l�A���C�g�����̃N���X�Ɋi�[���A�����_�����O����VulkanBase���痘�p�����
	void addLight(std::shared_ptr<PointLight> light);
	void addLight(std::shared_ptr<DirectionalLight> light);
	//UI�̒ǉ�
	void addUI(std::shared_ptr<UI> ui);

	void prepareDescriptorSets();//�ʏ�̃����_�����O�ŕK�v��descriptorSet�̍쐬
	void prepareDescriptorData();//���C�g�̃o�b�t�@�̗p��
	void prepareLightsForVulkan();//descriptorSet�̗p��

	//���̃N���X�ɂ��łɊi�[���ꂽgltfModel��Ԃ�
	std::shared_ptr<GltfModel> getgltfModel(std::string obj);
	//���̃N���X�ɂ��łɊi�[���ꂽ�摜��Ԃ�
	std::shared_ptr<ImageData> getImageData(std::string path);
	//���̃N���X�ɂ��łɊi�[���ꂽgltfModel��map��Ԃ�
	std::unordered_map<std::string, std::shared_ptr<GltfModel>>& getgltfModel();
	
	//�e�탉�C�g�p�̃o�b�t�@��Ԃ��A�Ȃ���ނ��Ƃɕ����̃��C�g����̔z��Ƃ��Ă܂Ƃ߂č\���̂ɂ��Ă��邽��
	//������ނ̃��C�g���������낤�ƁA���̃o�b�t�@�͈�̂�
	MappedBuffer& getPointLightsBuffer();
	MappedBuffer& getDirectionalLightsBuffer();

	//�J�����ւ̎Q�Ƃ�Ԃ�
	std::shared_ptr<Camera> accessCamera();

	//�R���C�_�[�p��AABB���v�Z����
	void calcSceneBoundingBox(glm::vec3& boundingMin, glm::vec3& boundingMax);

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