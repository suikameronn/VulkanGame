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

//Storageに指定されたMeshesがすでに存在するかどうかを返す
bool Storage::containMeshes(OBJECT obj)
{
	return meshesStorage.contains(obj);
}

//Storageに指定されたImageDataがすでに存在するかどうかを返す
bool Storage::containImageData(IMAGE image)
{
	return imageStorage.contains(image);
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