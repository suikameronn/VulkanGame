#pragma once
#include"Geometry.h"
#include"FileManager.h"

class Storage
{
private:
	std::unordered_map<OBJECT, std::shared_ptr<Geometry>> objStorage;
	std::unordered_map<IMAGE, std::shared_ptr<ImageData>> imageStorage;

public:
	void addObj(OBJECT obj, Geometry* geo);
	void addImage(IMAGE image, ImageData* imageData);

	std::shared_ptr<Geometry> shareObj(OBJECT obj);
	std::shared_ptr<ImageData> shareImage(IMAGE image);

	bool containGeometry(OBJECT obj);
	bool containImageData(IMAGE image);
};