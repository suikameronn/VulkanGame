#include"DescriptorSetFactory.h"

DescriptorSetFactory::DescriptorSetFactory(VkDevice& d,std::shared_ptr<DescriptorSetBuilder> b
	, std::shared_ptr<DescriptorSetLayoutFactory> lf)
{
	builder = b;

	layoutFactory = lf;

	device = d;

	frameIndex = 0;
}

std::shared_ptr<DescriptorSet> DescriptorSetFactory::Create(const DescriptorSetProperty& property)
{
	std::shared_ptr<DescriptorSet> descriptorSet = std::make_shared<DescriptorSet>(shared_from_this());

	builder->Create(property, descriptorSet->descriptorSet);

	descriptorSet->layout = property.layout;
	
	descriptorSet->buffer.resize(property.buffer.size());
	descriptorSet->texture.resize(property.texture.size());

	for (int i = 0; i < property.buffer.size(); i++)
	{
		descriptorSet->buffer[i] = property.buffer[i].buffer;
	}

	for(int i = 0; i < property.texture.size(); i++)
	{
		descriptorSet->texture[i] = property.texture[i].texture;
	}

	return descriptorSet;
}

void DescriptorSetFactory::addDefferedDestruct(VkDescriptorSet& descriptorSet)
{
	destructList[frameIndex].push_back(descriptorSet);
}

void DescriptorSetFactory::resourceDestruct()
{
	frameIndex = (frameIndex == 0) ? 1 : 0; //フレームインデックスを切り替える

	for (VkDescriptorSet& descriptorSet : destructList[frameIndex])
	{
		vkFreeDescriptorSets(device, builder->getPool(), 1, &descriptorSet);
	}

	destructList[frameIndex].clear();
}