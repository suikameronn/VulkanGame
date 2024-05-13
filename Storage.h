#pragma once
#include"Meshes.h"
#include"Material.h"
#include"FileManager.h"

class Storage
{
private:
	std::unordered_map<OBJECT, std::shared_ptr<Meshes>> meshesStorage;
	std::unordered_map<IMAGE, std::shared_ptr<ImageData>> imageStorage;

	void addObj(OBJECT obj, Meshes* geo);
	void addImage(IMAGE image, ImageData* imageData);

public:

	std::shared_ptr<Meshes> shareObj(OBJECT obj);
	std::shared_ptr<ImageData> shareImage(IMAGE image);

	bool containMeshes(OBJECT obj);
	bool containImageData(IMAGE image);

	uint32_t getSizeObjStorage();
	uint32_t getSizeImageStorage();
};