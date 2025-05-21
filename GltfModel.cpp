#include"GltfModel.h"

GltfModel::~GltfModel()
{
	for (size_t i = 0; i < skins.size(); i++)
	{
		delete skins[i];
	}
}

void GltfModel::deleteNodes(GltfNode* node,VkDevice& device)
{
	for (size_t i = 0; i < node->children.size(); i++)
	{
		deleteNodes(node->children[i],device);
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

//gpu上のバッファなどの削除処理
void GltfModel::cleanUpVulkan(VkDevice& device)
{
	for (size_t i = 0; i < pointBuffers.size(); i++)//頂点用バッファの解放
	{
		vkDestroyBuffer(device, pointBuffers[i].vertBuffer, nullptr);
		vkFreeMemory(device, pointBuffers[i].vertHandler, nullptr);

		vkDestroyBuffer(device, pointBuffers[i].indeBuffer, nullptr);
		vkFreeMemory(device, pointBuffers[i].indeHandler, nullptr);
	}

	for (std::shared_ptr<Material> material:materials)
	{
		vkDestroyBuffer(device, material->sMaterialMappedBuffer.uniformBuffer, nullptr);
		vkFreeMemory(device, material->sMaterialMappedBuffer.uniformBufferMemory, nullptr);
		material->sMaterialMappedBuffer.uniformBufferMapped = nullptr;
	}

	for (int i = 0; i < imageDatas.size(); i++)
	{
		//テクスチャのデータの破棄
		imageDatas[i]->getTexture()->destroy(device);
	}

	deleteNodes(root,device);
}