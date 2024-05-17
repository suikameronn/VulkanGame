#include"Storage.h"

Storage* Storage::storage = nullptr;

//Storage��Meshes��ǉ�����
void Storage::addObj(OBJECT obj, Meshes* geo)
{
	meshesStorage[obj].reset(geo);
}

//Storage��ImageData��ǉ�����
void Storage::addImage(IMAGE image, ImageData* imageData)
{
	imageStorage[image].reset(imageData);
}

//Storage����w�肳�ꂽMeshes�ւ̎Q�Ƃ�Ԃ�
Meshes* Storage::accessObj(OBJECT obj)
{
	return meshesStorage[obj].get();
}

//Storage����w�肳�ꂽImageData�ւ̎Q�Ƃ�Ԃ�
ImageData* Storage::accessImage(IMAGE image)
{
	return imageStorage[image].get();
}

//Storage����w�肳�ꂽMeshes�ւ̎Q�Ƃ�Ԃ�
std::shared_ptr<Meshes> Storage::shareObj(OBJECT obj)
{
	return meshesStorage[obj];
}

//Storage����w�肳�ꂽImageData�ւ̎Q�Ƃ�Ԃ�
std::shared_ptr<ImageData> Storage::shareImage(IMAGE image)
{
	return imageStorage[image];
}

//Storage�Ɏw�肳�ꂽMeshes�����łɑ��݂��邩�ǂ�����Ԃ�
bool Storage::containMeshes(OBJECT obj)
{
	return meshesStorage.contains(obj);
}

//Storage�Ɏw�肳�ꂽImageData�����łɑ��݂��邩�ǂ�����Ԃ�
bool Storage::containImageData(IMAGE image)
{
	return imageStorage.contains(image);
}

//Storage�Ɋi�[���ꂽMeshes�̃T�C�Y��Ԃ�
uint32_t Storage::getSizeObjStorage()
{
	return meshesStorage.size();
}

//Storage�Ɋi�[���ꂽImage�̃T�C�Y��Ԃ�
uint32_t Storage::getSizeImageStorage()
{
	return imageStorage.size();
}