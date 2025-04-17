#include"Storage.h"
#include"VulkanBase.h"

Storage* Storage::storage = nullptr;

Storage::Storage()
{
	pointLightDescSet = nullptr;
	directionalLightDescSet = nullptr;
}

void Storage::cleanup()
{

}

//gltf���f����ǂݍ��񂾍ۂɁA���̃N���X�Ɋi�[����B
//�Ăт���gltf���f�����K�v�ɂȂ����ꍇ�́A���̃N���X����Q�Ǝ擾����
void Storage::addModel(std::string obj, GltfModel* model)
{
	gltfModelStorage[obj] = std::shared_ptr<GltfModel>(model);
}

//��Ɠ��l�A�摜��ǂݍ��񂾍ۂɂ��̃N���X�Ɋi�[����
void Storage::addImageData(std::string path,ImageData* image)
{
	imageDataStorage[path] = std::shared_ptr<ImageData>(image);
}

//VulkanBase����J�����ɃA�N�Z�X�o��悤�ɁAScene�N���X�̃J������ݒ肷��
void Storage::setCamera(std::shared_ptr<Camera> c)
{
	camera = c;
}

//Scene�N���X��lua�Őݒ肳�ꂽ�I�u�W�F�N�g�͂��̃N���X�Ɋi�[���A���̃N���X����VulkanBase�ł��̃I�u�W�F�N�g�̃����_�����O���s��
void Storage::addModel(std::shared_ptr<Model> model)
{
	VulkanBase::GetInstance()->setModelData(model);
	sceneModelStorage.push_back(std::shared_ptr<Model>(model));
}

//Model�N���X���l�A���C�g�����̃N���X�Ɋi�[���A�����_�����O����VulkanBase���痘�p�����
void Storage::addLight(std::shared_ptr<PointLight> pl)
{
	scenePointLightStorage.push_back(pl);
}

//Model�N���X���l�A���C�g�����̃N���X�Ɋi�[���A�����_�����O����VulkanBase���痘�p�����
void Storage::addLight(std::shared_ptr<DirectionalLight> dl)
{
	sceneDirectionalLightStorage.push_back(dl);
}

//UI�̒ǉ�
void Storage::addUI(std::shared_ptr<UI> ui)
{
	VulkanBase::GetInstance()->setUI(ui);
	uiStorage.push_back(ui);
}

//�ʏ�̃����_�����O�ŕK�v��descriptorSet�̍쐬
void Storage::prepareDescriptorSets()
{
	VulkanBase::GetInstance()->prepareDescriptorSets();
}

void Storage::prepareLightsForVulkan()
{
	VulkanBase::GetInstance()->setLightData(scenePointLightStorage, sceneDirectionalLightStorage);//���C�g�p�̃o�b�t�@�̗p��
	VulkanBase::GetInstance()->setCubeMapModel(cubemap);//�L���[�u�}�b�v�p�̃o�b�t�@�̗p��
}

//descriptorSet�̗p��
void Storage::prepareDescriptorData()
{
	//DescriptorSetLayout��p�ӂ���
	int lightCount = static_cast<int>(sceneDirectionalLightStorage.size()) + static_cast<int>(scenePointLightStorage.size());
	VulkanBase::GetInstance()->prepareDescriptorData(lightCount);

	for (auto gltfModel : gltfModelStorage)
	{
		//gltfMdodel�N���X�̃e�N�X�`���Ȃǂ�p�ӂ���
		VulkanBase::GetInstance()->setGltfModelData(gltfModel.second);
	}
}

//�J�����ւ̎Q�Ƃ�Ԃ�
std::shared_ptr<Camera> Storage::accessCamera()
{
	return camera;
}

//���߂�ꂽ���\�[�X�����łɂ��̃N���X�Ɋi�[����Ă��邩�ǂ�����Ԃ�
//���̊֐��ł́A���̔���݂̂�S��
bool Storage::containModel(std::string path)
{
	if (gltfModelStorage[path] != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//���߂�ꂽ���\�[�X�����łɂ��̃N���X�Ɋi�[����Ă��邩�ǂ�����Ԃ�
//���̊֐��ł́A���̔���݂̂�S��
bool Storage::containImageData(std::string path)
{
	if (imageDataStorage[path] != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//���̃N���X�ɂ��łɊi�[���ꂽgltfModel��Ԃ�
std::unordered_map<std::string, std::shared_ptr<GltfModel>>& Storage::getgltfModel()
{
	return gltfModelStorage;
}

//���̃N���X�ɂ��łɊi�[���ꂽgltfModel��map��Ԃ�
std::shared_ptr<GltfModel> Storage::getgltfModel(std::string obj)
{
	return gltfModelStorage[obj];
}

//���̃N���X�ɂ��łɊi�[���ꂽ�摜��Ԃ�
std::shared_ptr<ImageData> Storage::getImageData(std::string path)
{
	return imageDataStorage[path];
}

//�e�탉�C�g�p�̃o�b�t�@��Ԃ��A�Ȃ���ނ��Ƃɕ����̃��C�g����̔z��Ƃ��Ă܂Ƃ߂č\���̂ɂ��Ă��邽��
//������ނ̃��C�g���������낤�ƁA���̃o�b�t�@�͈�̂�
MappedBuffer& Storage::getPointLightsBuffer()
{
	return pointLightsBuffer;
}

//�e�탉�C�g�p�̃o�b�t�@��Ԃ��A�Ȃ���ނ��Ƃɕ����̃��C�g����̔z��Ƃ��Ă܂Ƃ߂č\���̂ɂ��Ă��邽��
//������ނ̃��C�g���������낤�ƁA���̃o�b�t�@�͈�̂�
MappedBuffer& Storage::getDirectionalLightsBuffer()
{
	return directionalLightsBuffer;
}

//�R���C�_�[�p��AABB���v�Z����
void Storage::calcSceneBoundingBox(glm::vec3& boundingMin, glm::vec3& boundingMax)
{
	boundingMin = glm::vec3(FLT_MAX);
	boundingMax = glm::vec3(-FLT_MAX);

	for (auto model : storage->getModels())
	{
		glm::vec3 minimum = (model->scale * model->getGltfModel()->initPoseMin) + model->getPosition();
		glm::vec3 max = (model->scale * model->getGltfModel()->initPoseMax) + model->getPosition();

		if (boundingMin.x > minimum.x)
		{
			boundingMin.x = minimum.x;
		}
		if (boundingMin.y > minimum.y)
		{
			boundingMin.y = minimum.y;
		}
		if (boundingMin.z > minimum.z)
		{
			boundingMin.z = minimum.z;
		}

		if (boundingMax.x < max.x)
		{
			boundingMax.x = max.x;
		}
		if (boundingMax.y < max.y)
		{
			boundingMax.y = max.y;
		}
		if (boundingMax.z < max.z)
		{
			boundingMax.z = max.z;
		}
	}
}

//�L���[�u�}�b�s���O�p��HDRI�摜��ݒ肷��
void Storage::setCubemapTexture(std::shared_ptr<ImageData> image)
{
	cubemapImage = image;
}

//HDRI�摜��Ԃ��A�L���[�u�}�b�v�쐬����VulkanBase����Ăяo�����
std::shared_ptr<ImageData> Storage::getCubemapImage()
{
	return cubemapImage;
}