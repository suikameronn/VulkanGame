#pragma once

#include"DescriptorSetBuilder.h"
#include"DescriptorSetLayoutFactory.h"

struct DescriptorSet;

class DescriptorSetFactory : public std::enable_shared_from_this<DescriptorSetFactory>
{
private:

	uint32_t frameIndex;

	VkDevice device;

	std::shared_ptr<DescriptorSetBuilder> builder;

	std::shared_ptr<DescriptorSetLayoutFactory> layoutFactory;

public:

	DescriptorSetFactory(VkDevice& d, std::shared_ptr<DescriptorSetBuilder> b
		, std::shared_ptr<DescriptorSetLayoutFactory> lf);

	~DescriptorSetFactory()
	{
#ifdef _DEBUG
		std::cout << "DescriptorSetFactory :: デストラクタ" << std::endl;
#endif
	}

	std::shared_ptr<DescriptorSetBuilder> getBuilder()
	{
		return builder;
	}

	std::shared_ptr<DescriptorSet> Create(const DescriptorSetProperty& property);
};

struct DescriptorSet
{
	VkDescriptorSet descriptorSet;

	std::shared_ptr<DescriptorSetLayout> layout;

	std::vector<std::shared_ptr<GpuBuffer>> buffer;
	std::vector<std::shared_ptr<Texture>> texture;

	DescriptorSet()
	{
		descriptorSet = nullptr;
		layout = nullptr;

		buffer.clear();
		texture.clear();
	}
};