#include"Storage.h"

Storage* Storage::storage = nullptr;

//StorageにMeshesを追加する
void Storage::addObj(OBJECT obj, Meshes* geo)
{
	meshesStorage[obj].reset(geo);
}

//StorageにImageDataを追加する
void Storage::addImage(IMAGE image, ImageData* imageData)
{
	imageStorage[image].reset(imageData);
}

//StorageにDescriptorInfoを追加する
void Storage::addDescriptorInfo(std::bitset<8> layoutBit, DescriptorInfo* info)
{
	descriptorStorage[layoutBit].reset(info);
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
	return descriptorStorage[layoutBit].get();
}

//Storageから指定されたMeshesへの参照を返す
std::shared_ptr<Meshes> Storage::shareObj(OBJECT obj)
{
	return meshesStorage[obj];
}

//Storageから指定されたImageDataへの参照を返す
std::shared_ptr<ImageData> Storage::shareImage(IMAGE image)
{
	return imageStorage[image];
}

//Storageから指定されたDescriptorInfoへの参照を返す
std::shared_ptr<DescriptorInfo> Storage::shareDescriptor(std::bitset<8> layoutBit)
{
	return descriptorStorage[layoutBit];
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