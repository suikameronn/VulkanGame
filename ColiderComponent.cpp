#include"ColiderComponent.h"

void ColiderComponent::initilize(std::shared_ptr<TransformComponent> transform
	, std::shared_ptr<GltfModelComponent> gltfModel)
{
	transformComponent = transform;

	colider = std::make_shared<Colider>
		(
			gltfModel->getGltfModel()
			, false
			, bufferFactory
			, layoutFactory
			, descriptorSetFactory
		);
}

//初期フレームに実行
void ColiderComponent::OnStart()
{
	colider->initFrameSettings(glm::vec3(1.0f));
}

//更新フェーズで実行
void ColiderComponent::OnUpdate()
{
	colider->reflectMovement(transformComponent->transformMat());
}

//更新フェーズ後に実行
void ColiderComponent::OnLateUpdate()
{
	colider->reflectMovement(transformComponent->transformMat());
}