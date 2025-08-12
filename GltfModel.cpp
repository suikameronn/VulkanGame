#include"GltfModel.h"

GltfModel::~GltfModel()
{
	nodes.clear();

	skins.clear();
}

int GltfModel::findNode(const int& index)
{
	for (int i = 0; i < nodes.size(); i++)
	{
		if (nodes[i].index == index)
		{
			return i;
		}
	}

	return -1;
}

int GltfModel::nodeFromIndex(const int& index)
{
	return findNode(index);
}

//アニメーションの各ノードの更新処理
void GltfModel::updateAllNodes(NodeTransform& nodeTransform, std::vector<std::array<glm::mat4, 128>>& jointMatrices, size_t& updatedIndex)
{
	/*
	if (parent->meshArray.size() != 0 && parent->skin)
	{
		//このノードの所属するジョイントのアニメーション行列を計算
		parent->update(nodeTransform, jointMatrices[parent->globalHasSkinNodeIndex], updatedIndex);
	}

	for (size_t i = 0; i < parent->children.size(); i++)
	{
		updateAllNodes(parent->children[i], nodeTransform, jointMatrices, updatedIndex);
	}
	*/

	for (int i = 0; i < nodes.size(); i++)
	{
		if (nodes[i].mesh.vertices.size() != 0 && nodes[i].skinIndex > -1)
		{
			nodes[i].update(nodeTransform, jointMatrices[nodes[i].globalHasSkinNodeIndex], updatedIndex);
		}
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
	nodeTransform.resetMat();

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

					//GltfNodeの配列上のノードの番号を求める
					size_t nodeIndex = channel.nodeOffset;

					switch (channel.path) {
					case AnimationChannel::PathType::TRANSLATION://平行移動
						nodeTransform.translation[nodeIndex] = sampler.translate(i, animationTime);
						break;
					case AnimationChannel::PathType::SCALE://スケール
						nodeTransform.scale[nodeIndex] = sampler.scale(i, animationTime);
						break;
					case AnimationChannel::PathType::ROTATION://回転
						nodeTransform.rotation[nodeIndex] = sampler.rotate(i, animationTime);
						break;
					}
					updated = true;
				}
			}
		}
	}

	if (updated) {
		size_t updatedIndex = 0;

		for (int i = 0; i < nodes.size(); i++)
		{
			nodes[i].setLocalMatrix(nodeTransform);
		}

		updateAllNodes(nodeTransform, jointMatrices, updatedIndex);
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
	for (int i = 0; i < nodes.size(); i++)
	{
		if (nodes[i].mesh.vertices.size() != 0)
		{
			vertBuffer[nodes[i].mesh.meshIndex] = bufferFactory->Create(sizeof(Vertex) * nodes[i].mesh.vertices.size()
				, nodes[i].mesh.vertices.data(), BufferUsage::VERTEX, BufferTransferType::DST);

			indeBuffer[nodes[i].mesh.meshIndex] = bufferFactory->Create(sizeof(uint32_t) * nodes[i].mesh.indices.size()
				, nodes[i].mesh.indices.data(), BufferUsage::INDEX, BufferTransferType::DST);
		}
	}
}

void GltfModel::createDescriptorSet(std::vector<std::shared_ptr<DescriptorSet>>& descriptorSet)
{
	for (int i = 0; i < nodes.size(); i++)
	{
		if (nodes[i].mesh.vertices.size() != 0)
		{
			const std::shared_ptr<DescriptorSetLayout> layout =
				layoutFactory->Create(LayoutPattern::RAYCAST);

				const DescriptorSetProperty descProperty = descriptorSetFactory->getBuilder()
					->initProperty()
					.withBindingBuffer(0)
					.withDescriptorSetCount(1)
					.withTypeBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
					.withBuffer(vertBuffer[nodes[i].mesh.meshIndex])
					.withRange(sizeof(Vertex) * static_cast<uint32_t>(nodes[i].mesh.vertices.size()))
					.addBufferInfo()
					.withBindingBuffer(1)
					.withDescriptorSetCount(1)
					.withTypeBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
					.addBufferInfo()
					.withDescriptorSetLayout(layout)
					.withBuffer(indeBuffer[nodes[i].mesh.meshIndex])
					.withRange(sizeof(uint32_t) * static_cast<uint32_t>(nodes[i].mesh.indices.size()))
					.addBufferInfo()
					.Build();

				descriptorSet[nodes[i].mesh.meshIndex] = descriptorSetFactory->Create(descProperty);
		}
	}
}