#include"ColiderFactory.h"

ColiderFactory::ColiderFactory(std::shared_ptr<GltfModelFactory> gltf, std::shared_ptr<GpuBufferFactory> buffer
	, std::shared_ptr<DescriptorSetLayoutFactory> layout, std::shared_ptr<DescriptorSetFactory> desc)
{
	gltfModelFactory = gltf;

	bufferFactory = buffer;

	layoutFactory = layout;

	descriptorSetFactory = desc;

	lastIndex = 0;

	coliderStorage.resize(30);

	recycleIndex.clear();
}

size_t ColiderFactory::Create(const size_t& modelID)
{
	const std::shared_ptr<GltfModel> gltfModel = gltfModelFactory->GetModel(modelID);

	std::unique_ptr<Colider> colider = std::make_unique<Colider>(gltfModel, false, bufferFactory, layoutFactory
		, descriptorSetFactory);

	if (recycleIndex.size() == 0)
	{
		coliderStorage[lastIndex] = std::move(colider);

		lastIndex++;

		return lastIndex - 1;
	}
	else
	{
		size_t index = recycleIndex.back();

		recycleIndex.pop_back();

		coliderStorage[index] = std::move(colider);

		return index;
	}
}

std::unique_ptr<Colider>& ColiderFactory::GetColider(const size_t& coliderID)
{
	return coliderStorage[coliderID];
}