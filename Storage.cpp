#include"Storage.h"
#include"VulkanBase.h"

Storage* Storage::storage = nullptr;

void Storage::cleanup()
{
	for (auto itr = descriptorStorage.begin(); itr != descriptorStorage.end(); itr++)
	{

	}
}

//StorageにMeshesを追加する
void Storage::addObj(OBJECT obj, Meshes* meshes)
{
	meshesStorage[obj].reset(meshes);
}

//StorageにImageDataを追加する
void Storage::addImage(IMAGE image, ImageData* imageData)
{
	imageStorage[image].reset(imageData);
}

//StorageにDescriptorInfoを追加する
void Storage::addDescriptorInfo(std::bitset<8> layoutBit, DescriptorInfo* info)
{
	descriptorStorage[layoutBit] = info;
}

//StorageにCameraを追加する
void Storage::setCamera(Camera* c)
{
	camera = c;
}

//StorageにModelを追加する
void Storage::addModel(Model* model)
{
	VulkanBase::GetInstance()->setModelData(model);
	sceneModelStorage[model->getDescriptorInfo()].push_back(std::move(std::unique_ptr<Model>(model)));
}

//Storageから指定されたMeshesへの参照を返す
Meshes* Storage::accessObj(OBJECT obj)
{
	return meshesStorage[obj].get();
}

//Storageから指定されたImageDataへの参照を返す
ImageData* Storage::accessImage(IMAGE image)
{
	return imageStorage[image].get();
}

//Storageから指定されたDescriptorInfoへの参照を返す
DescriptorInfo* Storage::accessDescriptorInfo(std::bitset<8> layoutBit)
{
	return descriptorStorage[layoutBit];
}

//StorageからDescriptorInfoのマップへの参照を返す
void Storage::accessDescriptorInfoItr(std::unordered_map<std::bitset<8>, DescriptorInfo*>::iterator& begin,
	std::unordered_map<std::bitset<8>, DescriptorInfo*>::iterator& end)
{
	begin = descriptorStorage.begin();
	end = descriptorStorage.end();
}

//StorageのCameraにアクセスする
Camera* Storage::accessCamera()
{
	return camera;
}

//Storageから指定された個別のグループのvectorの参照を返す
void Storage::accessModelVector(std::unordered_map<DescriptorInfo*, std::vector<std::unique_ptr<Model>>, Hash>::iterator current,
	std::vector<std::unique_ptr<Model>>::iterator& itr,std::vector<std::unique_ptr<Model>>::iterator& itr2)
{
	itr = current->second.begin();
	itr2 = current->second.end();
}

//Storageから指定された全体のunordered_mapの参照を返す
void Storage::accessModelUnMap(std::unordered_map<DescriptorInfo*, std::vector<std::unique_ptr<Model>>, Hash>::iterator* itr,
	std::unordered_map<DescriptorInfo*, std::vector<std::unique_ptr<Model>>, Hash>::iterator* itr2)
{
	*itr = sceneModelStorage.begin();
	*itr2 = sceneModelStorage.end();
}

//Storageに指定されたMeshesがすでに存在するかどうかを返す
bool Storage::containMeshes(OBJECT obj)
{
	if (meshesStorage[obj] != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//Storageに指定されたImageDataがすでに存在するかどうかを返す
bool Storage::containImageData(IMAGE image)
{
	if (imageStorage[image] != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//Storageに指定されたDescritporInfoがすでに存在するかどうかを返す
bool Storage::containDescriptorInfo(std::bitset<8> layoutBit)
{
	if (descriptorStorage[layoutBit] != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//Storageに格納されたMeshesのサイズを返す
uint32_t Storage::getSizeObjStorage()
{
	return meshesStorage.size();
}

//Storageに格納されたImageのサイズを返す
uint32_t Storage::getSizeImageStorage()
{
	return imageStorage.size();
}