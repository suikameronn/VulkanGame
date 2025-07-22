#include"GltfModel.h"

GltfModel::~GltfModel()
{
	deleteNodes(root);

	for (size_t i = 0; i < skins.size(); i++)
	{
		delete skins[i];
	}
}

void GltfModel::deleteNodes(GltfNode* node)
{
	for (size_t i = 0; i < node->children.size(); i++)
	{
		deleteNodes(node->children[i]);
	}

	delete node;
}

GltfNode* GltfModel::findNode(GltfNode* parent,int index)
{
	GltfNode* dstNode = nullptr;

	if (parent->index == index)
	{
		return parent;
	}

	for (size_t i = 0; i < parent->children.size(); i++)
	{
		dstNode = findNode(parent->children[i], index);
		if (dstNode)
		{
			break;
		}
	}

	return dstNode;
}

GltfNode* GltfModel::nodeFromIndex(int index)
{
	GltfNode* node = nullptr;

	node = findNode(root,index);

	return node;
}

//アニメーションの各ノードの更新処理
void GltfModel::updateAllNodes(GltfNode* parent, NodeTransform& nodeTransform, std::vector<std::array<glm::mat4, 128>>& jointMatrices, size_t& updatedIndex)
{
	if (parent->meshArray.size() != 0 && parent->skin)
	{
		//このノードの所属するジョイントのアニメーション行列を計算
		parent->update(nodeTransform, jointMatrices[parent->globalHasSkinNodeIndex], updatedIndex);
	}

	for (size_t i = 0; i < parent->children.size(); i++)
	{
		updateAllNodes(parent->children[i], nodeTransform, jointMatrices, updatedIndex);
	}
}

//アニメーション長さを取得
float GltfModel::animationDuration(std::string animationName)
{
	Animation& animation = animations[animationName];

	return animation.end - animation.start;
}

//指定したアニメーションの行列を取得
void GltfModel::updateAnimation(double animationTime, std::string animationName
	, NodeTransform& nodeTransform, std::vector<std::array<glm::mat4, 128>>& jointMatrices)
{
	if (animations.empty()) {
		std::cout << ".glTF does not contain animation." << std::endl;
		return;
	}

	Animation& animation = animations[animationName];

	//アニメーション行列の初期化
	nodeTransform.setNodeCount(nodeCount);

	bool updated = false;
	for (auto& channel : animation.channels) {

		AnimationSampler& sampler = animation.samplers[channel.samplerIndex];

		if (sampler.inputs.size() > sampler.outputsVec4.size()) {
			continue;
		}

		for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
			if ((animationTime >= sampler.inputs[i]) && (animationTime <= sampler.inputs[i + 1])) {
				float u = static_cast<float>(std::max(0.0, animationTime - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]));
				if (u <= 1.0f) {

					int nodeIndex = channel.node->index;

					switch (channel.path) {
					case AnimationChannel::PathType::TRANSLATION://平行移動
						nodeTransform.translation[nodeIndex] = sampler.translate(i, animationTime, channel.node);
						break;
					case AnimationChannel::PathType::SCALE://スケール
						nodeTransform.scale[nodeIndex] = sampler.scale(i, animationTime, channel.node);
						break;
					case AnimationChannel::PathType::ROTATION://回転
						nodeTransform.rotation[nodeIndex] = sampler.rotate(i, animationTime, channel.node);
						break;
					}
					updated = true;
				}
			}
		}
	}

	if (updated) {
		size_t updatedIndex = 0;

		root->setLocalMatrix(nodeTransform);

		updateAllNodes(root, nodeTransform, jointMatrices, updatedIndex);
	}
}

//gltfモデルの初期ポーズの頂点の座標の最小値最大値の取得
void GltfModel::getVertexMinMax(GltfNode* node)
{
	if (node->bvh.valid)
	{
		initPoseMin = glm::min(initPoseMin, node->bvh.min);
		initPoseMax = glm::max(initPoseMax, node->bvh.max);
	}

	for (int i = 0; i < node->children.size(); i++)
	{
		getVertexMinMax(node->children[i]);
	}
}

void GltfModel::setPointBufferNum()
{
	vertBuffer.resize(meshCount);
	indeBuffer.resize(meshCount);
	descriptorSet.resize(meshCount);

	createBuffer();
}

void GltfModel::createBuffer()
{
	createBuffer(root);

	for (auto& child: root->children)
	{
		createBuffer(child);
	}
}

void GltfModel::createBuffer(const GltfNode* node)
{
	if (node->meshArray.size() != 0)
	{
		for (auto& mesh : node->meshArray)
		{
			vertBuffer[mesh->meshIndex] = bufferFactory->Create(sizeof(Vertex) * mesh->vertices.size()
				, BufferUsage::VERTEX, BufferTransferType::DST);

			indeBuffer[mesh->meshIndex] = bufferFactory->Create(sizeof(Vertex) * mesh->vertices.size()
				, BufferUsage::INDEX, BufferTransferType::DST);
		}
	}

	for (auto& child : node->children)
	{
		createBuffer(child);
	}
}

void GltfModel::createDescriptorSet(std::vector<std::shared_ptr<DescriptorSet>>& descriptorSet)
{
	createDescriptorSet(root, descriptorSet);

	for (auto& child : root->children)
	{
		createDescriptorSet(child, descriptorSet);
	}
}

void GltfModel::createDescriptorSet(const GltfNode* node
	, std::vector<std::shared_ptr<DescriptorSet>>& descriptorSet)
{
	if (node->meshArray.size() != 0)
	{
		const std::shared_ptr<DescriptorSetLayout> layout =
			layoutFactory->Create(LayoutPattern::RAYCAST);

			for (auto& mesh : node->meshArray)
			{
				const DescriptorSetProperty descProperty = descriptorSetFactory->getBuilder()
					->initProperty()
					.withBindingBuffer(0)
					.withDescriptorSetCount(1)
					.withTypeBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
					.withBuffer(vertBuffer[mesh->meshIndex])
					.withRange(sizeof(Vertex) * mesh->vertices.size())
					.addBufferInfo()
					.withBindingBuffer(1)
					.withDescriptorSetCount(1)
					.withTypeBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
					.addBufferInfo()
					.withDescriptorSetLayout(layout)
					.withBuffer(indeBuffer[mesh->meshIndex])
					.withRange(sizeof(uint32_t) * mesh->indices.size())
					.addBufferInfo()
					.Build();

				descriptorSet[mesh->meshIndex] = descriptorSetFactory->Create(descProperty);
			}
	}

	for (auto& child : node->children)
	{
		createDescriptorSet(child, descriptorSet);
	}
}