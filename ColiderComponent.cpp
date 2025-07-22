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

//�����t���[���Ɏ��s
void ColiderComponent::OnStart()
{
	colider->initFrameSettings(glm::vec3(1.0f));
}

//�X�V�t�F�[�Y�Ŏ��s
void ColiderComponent::OnUpdate()
{
	colider->reflectMovement(transformComponent->transformMat());
}

//�X�V�t�F�[�Y��Ɏ��s
void ColiderComponent::OnLateUpdate()
{
	colider->reflectMovement(transformComponent->transformMat());
}