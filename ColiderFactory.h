#pragma once

#include"GltfModelFactory.h"
#include"Colider.h"

class ColiderFactory
{
private:

	//gltfモデルを取り出す
	std::shared_ptr<GltfModelFactory> gltfModelFactory;

	std::shared_ptr<GpuBufferFactory> bufferFactory;

	std::shared_ptr<DescriptorSetLayoutFactory> layoutFactory;

	std::shared_ptr<DescriptorSetFactory> descriptorSetFactory;

	//次に配列に代入する予定のインデックス
	size_t lastIndex;

	//コライダーを記録しておく
	std::vector<std::unique_ptr<Colider>> coliderStorage;

	//コライダーが破棄された場合の、空いたインデックスを集めておく配列
	std::vector<int> recycleIndex;

public:

	ColiderFactory(std::shared_ptr<GltfModelFactory> gltf, std::shared_ptr<GpuBufferFactory> buffer
		, std::shared_ptr<DescriptorSetLayoutFactory> layout, std::shared_ptr<DescriptorSetFactory> desc);

	size_t Create(const size_t& modelID);

	std::unique_ptr<Colider>& GetColider(const size_t& coliderID);
};