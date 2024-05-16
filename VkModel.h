#pragma once
#include"Model.h"
#include"VkObject.h"

#include<bitset>

//ModelクラスにVulkan特有の変数を増やしたようなクラス
class VkModel:public VkObject
{
private:
	Model* model;

	VkBuffer vertBuffer = nullptr;
	VkDeviceMemory vertHandler = nullptr;

	VkBuffer indiBuffer = nullptr;
	VkDeviceMemory indiHandler = nullptr;

	uint32_t mipLevel = 0;
	VkImage image = nullptr;
	VkDeviceMemory memory = nullptr;
	VkImageView view = nullptr;
	VkSampler sampler = nullptr;

	VkPipeline* pipeline = nullptr;

public:
	VkModel(Model* model);
	~VkModel();

	void setModel(Model* model);
	void setPipeline(VkPipeline* p);

	Meshes* getMeshes();
	Material* getMaterial();
	ImageData* getImageData();

	VkImageView getImageView();
	VkSampler getSampler();

	//DescriptroSetの登録済みの判定を調べるための関数
	//DescriptorSetはレイアウトが同じでも、テクスチャが異なる場合は、DescriptorSetも異なるものを使用するため
	//レイアウトとImageData両方が同じときのみ登録済みとする
	std::pair<std::bitset<8>, ImageData*> getPairLayoutImage();
};