#include"GltfModelComponent.h"

GltfModelComponent::GltfModelComponent(std::shared_ptr<GpuBufferFactory> buffer
	, std::shared_ptr<GpuDescriptorSetLayoutFactory> layout
	, std::shared_ptr<DescriptorSet> desc, std::shared_ptr<GltfModel> model)
{
	bufferFactory = buffer;
	layoutFactory = layout;
	descriptorSetFactory = desc;

	gltfModel = model;
}

//3Dモデルの中心を求める
glm::vec3 GltfModelComponent::getCenter()
{
	return (gltfModel->initPoseMax + gltfModel->initPoseMin) / 2;
}


//コンポーネントをアタッチした時点で実行
void GltfModelComponent::OnAwake()
{
}