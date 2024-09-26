#include"Storage.h"
#include"VulkanBase.h"

Storage* Storage::storage = nullptr;

void Storage::cleanup()
{
	for (auto itr = descriptorStorage.begin(); itr != descriptorStorage.end(); itr++)
	{

	}
}

//StorageにFbxModelを追加する
void Storage::addModel(OBJECT obj, FbxModel* model)
{
	fbxModelStorage[obj] = std::shared_ptr<FbxModel>(model);
}

void Storage::addAnimation(OBJECT obj, Animation* animation)
{
	fbxAnimationStorage[obj] = std::shared_ptr<Animation>(animation);
}

void Storage::addImageData(std::string path,ImageData* image)
{
	imageDataStorage[path] = std::shared_ptr<ImageData>(image);
}

//StorageにDescriptorInfoを追加する
void Storage::addDescriptorInfo(uint32_t imageDataCount, DescriptorInfo& info)
{
	descriptorStorage[imageDataCount] = info;
}

//StorageにCameraを追加する
void Storage::setCamera(std::shared_ptr<Camera> c)
{
	camera = c;
}

//StorageにModelを追加する
void Storage::addModel(std::shared_ptr<Model> model)
{
	DescriptorInfo info{};
	VulkanBase::GetInstance()->setModelData(model);
	sceneModelStorage.push_back(model);
}

//Storageから指定されたDescriptorInfoへの参照を返す
DescriptorInfo* Storage::accessDescriptorInfo(uint32_t imageDataCount)
{
	return &descriptorStorage[imageDataCount];
}

//StorageからDescriptorInfoのマップへの参照を返す
void Storage::accessDescriptorInfoItr(std::unordered_map<uint32_t, DescriptorInfo>::iterator& begin,
	std::unordered_map<uint32_t, DescriptorInfo>::iterator& end)
{
	begin = descriptorStorage.begin();
	end = descriptorStorage.end();
}

void Storage::accessFbxModel(std::unordered_map<OBJECT, std::shared_ptr<FbxModel>>::iterator& itr,
	std::unordered_map<OBJECT, std::shared_ptr<FbxModel>>::iterator& itr2)
{
	itr = fbxModelStorage.begin();
	itr2 = fbxModelStorage.end();
}

//StorageのCameraにアクセスする
std::shared_ptr<Camera> Storage::accessCamera()
{
	return camera;
}

//Storageに指定されあFbxModelが既に存在しているかどうかを返す
bool Storage::containModel(OBJECT obj)
{
	if (fbxModelStorage[obj] != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Storage::containAnimation(OBJECT obj)
{
	if (fbxAnimationStorage[obj] != nullptr)
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

//Storageに指定されたDescritporInfoがすでに存在するかどうかを返す
bool Storage::containDescriptorInfo(uint32_t imageDataCount)
{
	if (descriptorStorage.find(imageDataCount) != descriptorStorage.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

//StorageからFbxModelを読み取る
std::shared_ptr<FbxModel> Storage::getFbxModel(OBJECT obj)
{
	return fbxModelStorage[obj];
}

std::shared_ptr<Animation> Storage::getAnimation(OBJECT obj)
{
	return fbxAnimationStorage[obj];
}

std::shared_ptr<ImageData> Storage::getImageData(std::string path)
{
	return imageDataStorage[path];
}