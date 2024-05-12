#include"Storage.h"

//StorageにGeometryを追加する
void Storage::addObj(OBJECT obj, Geometry* geo)
{
	objStorage[obj].reset(geo);
}

//StorageにImageDataを追加する
void Storage::addImage(IMAGE image, ImageData* imageData)
{
	imageStorage[image].reset(imageData);
}

//Storageから指定されたGeometryへの参照を返す
std::shared_ptr<Geometry> Storage::shareObj(OBJECT obj)
{
	return objStorage[obj];
}

//Storageから指定されたImageDataへの参照を返す
std::shared_ptr<ImageData> Storage::shareImage(IMAGE image)
{
	return imageStorage[image];
}

//Storageに指定されたGeometryがすでに存在するかどうかを返す
bool Storage::containGeometry(OBJECT obj)
{
	return objStorage.contains(obj);
}

//Storageに指定されたImageDataがすでに存在するかどうかを返す
bool Storage::containImageData(IMAGE image)
{
	return imageStorage.contains(image);
}