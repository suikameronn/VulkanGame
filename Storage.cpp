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

//StorageにGltfModelを追加する
void Storage::addModel(GLTFOBJECT obj, GltfModel* model)
{
	gltfModelStorage[obj] = std::shared_ptr<GltfModel>(model);

	VulkanBase::GetInstance()->setGltfModelData(gltfModelStorage[obj]);
}

void Storage::addImageData(std::string path,ImageData* image)
{
	imageDataStorage[path] = std::shared_ptr<ImageData>(image);
}

//StorageにCameraを追加する
void Storage::setCamera(std::shared_ptr<Camera> c)
{
	camera = c;
}

//StorageにModelを追加する
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
	VulkanBase::GetInstance()->setLightData(scenePointLightStorage, sceneDirectionalLightStorage);
}

//StorageのCameraにアクセスする
std::shared_ptr<Camera> Storage::accessCamera()
{
	return camera;
}

//Storageに指定されあGltfModelが既に存在しているかどうかを返す
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

//StorageからGltfModelを読み取る
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