#include"Storage.h"
#include"VulkanBase.h"

Storage* Storage::storage = nullptr;

void Storage::cleanup()
{

}

//StorageにGltfModelを追加する
void Storage::addModel(OBJECT obj, GltfModel* model)
{
	gltfModelStorage[obj] = std::shared_ptr<GltfModel>(model);
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

//StorageにDescriptorInfoを追加する
void Storage::addDescriptorInfo(PrimitiveTextureCount ptc, DescriptorInfo& info)
{
	descriptorStorage[ptc] = info;
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

//Storageから指定されたDescriptorInfoへの参照を返す
DescriptorInfo* Storage::accessDescriptorInfo(PrimitiveTextureCount ptc)
{
	return &descriptorStorage[ptc];
}

//StorageからDescriptorInfoのマップへの参照を返す
void Storage::accessDescriptorInfoItr(std::unordered_map<PrimitiveTextureCount, DescriptorInfo>::iterator& begin,
	std::unordered_map<PrimitiveTextureCount, DescriptorInfo>::iterator& end)
{
	begin = descriptorStorage.begin();
	end = descriptorStorage.end();
}

void Storage::accessgltfModel(std::unordered_map<OBJECT, std::shared_ptr<GltfModel>>::iterator& itr,
	std::unordered_map<OBJECT, std::shared_ptr<GltfModel>>::iterator& itr2)
{
	itr = gltfModelStorage.begin();
	itr2 = gltfModelStorage.end();
}

//StorageのCameraにアクセスする
std::shared_ptr<Camera> Storage::accessCamera()
{
	return camera;
}

//Storageに指定されあGltfModelが既に存在しているかどうかを返す
bool Storage::containModel(OBJECT obj)
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

//Storageに指定されたDescritporInfoがすでに存在するかどうかを返す
bool Storage::containDescriptorInfo(PrimitiveTextureCount ptc)
{
	if (descriptorStorage.find(ptc) != descriptorStorage.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

//StorageからGltfModelを読み取る
std::shared_ptr<GltfModel> Storage::getgltfModel(OBJECT obj)
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