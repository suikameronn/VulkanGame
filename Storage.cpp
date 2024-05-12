#include"Storage.h"

//Storage��Geometry��ǉ�����
void Storage::addObj(OBJECT obj, Geometry* geo)
{
	objStorage[obj].reset(geo);
}

//Storage��ImageData��ǉ�����
void Storage::addImage(IMAGE image, ImageData* imageData)
{
	imageStorage[image].reset(imageData);
}

//Storage����w�肳�ꂽGeometry�ւ̎Q�Ƃ�Ԃ�
std::shared_ptr<Geometry> Storage::shareObj(OBJECT obj)
{
	return objStorage[obj];
}

//Storage����w�肳�ꂽImageData�ւ̎Q�Ƃ�Ԃ�
std::shared_ptr<ImageData> Storage::shareImage(IMAGE image)
{
	return imageStorage[image];
}

//Storage�Ɏw�肳�ꂽGeometry�����łɑ��݂��邩�ǂ�����Ԃ�
bool Storage::containGeometry(OBJECT obj)
{
	return objStorage.contains(obj);
}

//Storage�Ɏw�肳�ꂽImageData�����łɑ��݂��邩�ǂ�����Ԃ�
bool Storage::containImageData(IMAGE image)
{
	return imageStorage.contains(image);
}