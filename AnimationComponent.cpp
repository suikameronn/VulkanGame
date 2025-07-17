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

//3Dモデルを設定する
void AnimationComponent::setGltfModel(std::shared_ptr<GltfModel> model)
{
	gltfModel = model;

	nodeTransform.setNodeCount(gltfModel->nodeCount);
}

//アニメーションを切り替える
void AnimationComponent::switchPlayAnimation(std::string next)
{
	nextPlayAnimationName = next;
}

////特定のスケルトンのアニメーション行列を取得
std::shared_ptr<DescriptorSet> AnimationComponent::getJointMatrices(int index)
{
	return descriptorSet[index];
}

//アタッチ後の次のフレーム開始時点で実行
void AnimationComponent::OnStart()
{
	jointMatrices.resize(gltfModel->jointNum);
	animationBuffer.resize(gltfModel->primitiveCount);
	descriptorSet.resize(gltfModel->primitiveCount);
	animationNames.resize(gltfModel->animations.size());

	//gpu上のバッファを作成
	createBuffer();

	//ディスクリプタセットを作成
	createDescriptorSet();
}

//GPU上のバッファを作成する
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

//ディスクリプタセットを作る
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

//フレーム終了時に実行
void AnimationComponent::OnFrameEnd()
{
	if (currentPlayAnimationName != "")
	{
		double currentTime = clock();

		deltaTime = static_cast<double>(currentTime - startTime) / CLOCKS_PER_SEC;

		//アニメーションを再生し終えた
		//あるいは、アニメーションが切り替わった場合
		if (deltaTime >= gltfModel->animationDuration(currentPlayAnimationName)
			|| currentPlayAnimationName != nextPlayAnimationName)
		{
			currentPlayAnimationName = nextPlayAnimationName;
			//再生時間を再び計測し始める

			startTime = clock();

			deltaTime = 0.0;
		}

		//アニメーション行列の計算
		gltfModel->updateAnimation(deltaTime, currentPlayAnimationName, nodeTransform, jointMatrices);
	}
}