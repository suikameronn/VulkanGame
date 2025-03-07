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

//Storage��GltfModel��ǉ�����
void Storage::addModel(GLTFOBJECT obj, GltfModel* model)
{
	gltfModelStorage[obj] = std::shared_ptr<GltfModel>(model);
}

void Storage::addImageData(std::string path,ImageData* image)
{
	imageDataStorage[path] = std::shared_ptr<ImageData>(image);
}

//Storage��Camera��ǉ�����
void Storage::setCamera(std::shared_ptr<Camera> c)
{
	camera = c;
}

//Storage��Model��ǉ�����
void Storage::addModel(std::shared_ptr<Model> model)
{
	VulkanBase::GetInstance()->setModelData(model);
	sceneModelStorage.push_back(std::shared_ptr<Model>(model));
}

void Storage::addLight(std::shared_ptr<PointLight> pl)
{
	scenePointLightStorage.push_back(pl);
}

void Storage::addLight(std::shared_ptr<DirectionalLight> dl)
{
	sceneDirectionalLightStorage.push_back(dl);
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
	VulkanBase::GetInstance()->prepareDescriptorData(static_cast<int>(sceneDirectionalLightStorage.size()) + scenePointLightStorage.size());

	for (auto gltfModel : gltfModelStorage)
	{
		//gltfMdodel�N���X�̃e�N�X�`���Ȃǂ�p�ӂ���
		VulkanBase::GetInstance()->setGltfModelData(gltfModel.second);
	}
}

//Storage��Camera�ɃA�N�Z�X����
std::shared_ptr<Camera> Storage::accessCamera()
{
	return camera;
}

//Storage�Ɏw�肳�ꂠGltfModel�����ɑ��݂��Ă��邩�ǂ�����Ԃ�
bool Storage::containModel(GLTFOBJECT obj)
{
	if (gltfModelStorage[obj] != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

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

std::unordered_map<GLTFOBJECT, std::shared_ptr<GltfModel>>& Storage::getgltfModel()
{
	return gltfModelStorage;
}

//Storage����GltfModel��ǂݎ��
std::shared_ptr<GltfModel> Storage::getgltfModel(GLTFOBJECT obj)
{
	return gltfModelStorage[obj];
}

/*
std::shared_ptr<Animation> Storage::getAnimation(OBJECT obj)
{
	return gltfAnimationStorage[obj];
}
*/

std::shared_ptr<ImageData> Storage::getImageData(std::string path)
{
	return imageDataStorage[path];
}

MappedBuffer& Storage::getPointLightsBuffer()
{
	return pointLightsBuffer;
}

MappedBuffer& Storage::getDirectionalLightsBuffer()
{
	return directionalLightsBuffer;
}

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

void Storage::setCubemapTexture(std::shared_ptr<ImageData> image)
{
	cubemapImage = image;
}

std::shared_ptr<ImageData> Storage::getCubemapImage()
{
	return cubemapImage;
}