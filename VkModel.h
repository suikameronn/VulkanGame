#pragma once
#include"Model.h"

//GeometryクラスにVulkan特有の変数を増やしたようなクラス
class VkModel
{
private:
	Model* model;

	int texDescriptorID;
	int uniformDescriptorID;

	VkBuffer buffer;
	VkDeviceMemory handler;

	uint32_t mipLevel;
	VkImage image;
	VkDeviceMemory memory;
	VkImageView view;
	VkSampler sampler;

public:
	VkModel(Model* model);

	void setModel(Model* model);

	void setTexDescriptorID(uint32_t id);
	void setUniformDescriptorID(uint32_t id);

	Meshes* getMeshes();
	Material* getMaterial();
	ImageData* getImageData();
};