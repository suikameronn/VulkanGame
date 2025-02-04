#include"Storage.h"
#include"VulkanBase.h"

Storage* Storage::storage = nullptr;

Storage::Storage()
{
	lightLayout = nullptr;
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

	VulkanBase::GetInstance()->setGltfModelData(gltfModelStorage[obj]);
}

/*
void Storage::addAnimation(OBJECT obj, Animation* animation)
{
	gltfAnimationStorage[obj] = std::shared_ptr<Animation>(animation);
}
*/

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
	VulkanBase::GetInstance()->setLightData(scenePointLightStorage, sceneDirectionalLightStorage);
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

/*
bool Storage::containAnimation(OBJECT obj)
{
	if (gltfAnimationStorage[obj] != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}
*/

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