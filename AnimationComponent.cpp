#include"AnimationComponent.h"

AnimationComponent::AnimationComponent(std::shared_ptr<GpuBufferFactory> buffer
	, std::shared_ptr<GpuDescriptorSetLayoutFactory> layout
	, std::shared_ptr<DescriptorSetFactory> desc)
{
	bufferFactory = buffer;
	layoutFactory = layout;
	descriptorSetFactory = desc;

	gltfModel = nullptr;

	deltaTime = 0.0;
	startTime = 0.0;
	animationChange = false;

	currentPlayAnimationName = "";
	nextPlayAnimationName = "";
}

//3D���f����ݒ肷��
void AnimationComponent::setGltfModel(std::shared_ptr<GltfModel> model)
{
	gltfModel = model;

	nodeTransform.setNodeCount(gltfModel->nodeCount);
}

//�A�j���[�V������؂�ւ���
void AnimationComponent::switchPlayAnimation(std::string next)
{
	nextPlayAnimationName = next;
}

////����̃X�P���g���̃A�j���[�V�����s����擾
std::shared_ptr<DescriptorSet> AnimationComponent::getJointMatrices(int index)
{
	return descriptorSet[index];
}

//�A�^�b�`��̎��̃t���[���J�n���_�Ŏ��s
void AnimationComponent::OnStart()
{
	jointMatrices.resize(gltfModel->jointNum);
	animationBuffer.resize(gltfModel->primitiveCount);
	descriptorSet.resize(gltfModel->primitiveCount);
	animationNames.resize(gltfModel->animations.size());

	//gpu��̃o�b�t�@���쐬
	createBuffer();

	//�f�B�X�N���v�^�Z�b�g���쐬
	createDescriptorSet();
}

//GPU��̃o�b�t�@���쐬����
void AnimationComponent::createBuffer()
{
	const GltfNode* root = gltfModel->getRootNode();

	for (auto node : root->children)
	{
		createBuffer(node);
	}
}

void AnimationComponent::createBuffer(const GltfNode* node)
{
	for (auto mesh : node->meshArray)
	{
		animationBuffer[mesh->meshIndex] =
			bufferFactory->Create(sizeof(AnimationUBO), BufferUsage::UNIFORM, BufferTransferType::DST);
	}

	for (auto child : node->children)
	{
		createBuffer(child);
	}
}

//�f�B�X�N���v�^�Z�b�g�����
void AnimationComponent::createDescriptorSet()
{
	const GltfNode* root = gltfModel->getRootNode();

	for (auto node : root->children)
	{
		createDescriptorSet(node);
	}
}

void AnimationComponent::createDescriptorSet(const GltfNode* node)
{
	const std::shared_ptr<DescriptorSetLayout> layout
		= layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT);

	for (auto mesh : node->meshArray)
	{
		DescriptorSetProperty property = descriptorSetFactory->getBuilder()
			->initProperty()
			.withBindingBuffer(0)
			.withBuffer(animationBuffer[mesh->meshIndex])
			.withDescriptorSetCount(1)
			.withDescriptorSetLayout(layout)
			.withRange(sizeof(AnimationUBO))
			.withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			.Build();

		descriptorSet[mesh->meshIndex] = descriptorSetFactory->Create(property);
	}

	for (auto child : node->children)
	{
		createDescriptorSet(child);
	}
}

//�t���[���I�����Ɏ��s
void AnimationComponent::OnFrameEnd()
{
	if (currentPlayAnimationName != "")
	{
		double currentTime = clock();

		deltaTime = static_cast<double>(currentTime - startTime) / CLOCKS_PER_SEC;

		//�A�j���[�V�������Đ����I����
		//���邢�́A�A�j���[�V�������؂�ւ�����ꍇ
		if (deltaTime >= gltfModel->animationDuration(currentPlayAnimationName)
			|| currentPlayAnimationName != nextPlayAnimationName)
		{
			currentPlayAnimationName = nextPlayAnimationName;
			//�Đ����Ԃ��Ăьv�����n�߂�

			startTime = clock();

			deltaTime = 0.0;
		}

		//�A�j���[�V�����s��̌v�Z
		gltfModel->updateAnimation(deltaTime, currentPlayAnimationName, nodeTransform, jointMatrices);
	}
}