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
void GltfModel::updateAllNodes(GltfNode* parent, std::vector<std::array<glm::mat4, 128>>& jointMatrices,size_t& updatedIndex)
{
	if (parent->mesh && parent->skin)
	{
		//このノードの所属するジョイントのアニメーション行列を計算
		parent->update(jointMatrices[parent->globalHasSkinNodeIndex],updatedIndex);
	}

	for (size_t i = 0; i < parent->children.size(); i++)
	{
		updateAllNodes(parent->children[i],jointMatrices,updatedIndex);
	}
}

//アニメーション長さを取得
float GltfModel::animationDuration(std::string animationName)
{
	Animation& animation = animations[animationName];

	return animation.end - animation.start;
}

//指定したアニメーションの行列を取得
void GltfModel::updateAnimation(double animationTime,Animation& animation, std::vector<std::array<glm::mat4, 128>>& jointMatrices)
{
	if (animations.empty()) {
		std::cout << ".glTF does not contain animation." << std::endl;
		return;
	}

	bool updated = false;
	for (auto& channel : animation.channels) {

		AnimationSampler sampler = animation.samplers[channel.samplerIndex];

		if (sampler.inputs.size() > sampler.outputsVec4.size()) {
			continue;
		}

		for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
			if ((animationTime >= sampler.inputs[i]) && (animationTime <= sampler.inputs[i + 1])) {
				float u = static_cast<float>(std::max(0.0, animationTime - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]));
				if (u <= 1.0f) {
					switch (channel.path) {
					case AnimationChannel::PathType::TRANSLATION://平行移動
						sampler.translate(i, animationTime, channel.node);
						break;
					case AnimationChannel::PathType::SCALE://スケール
						sampler.scale(i, animationTime, channel.node);
						break;
					case AnimationChannel::PathType::ROTATION://回転
						sampler.rotate(i, animationTime, channel.node);
						break;
					}
					updated = true;
				}
			}
		}
	}

	if (updated) {
		size_t updatedIndex = 0;
		updateAllNodes(root,jointMatrices,updatedIndex);
	}
}

//AABBの計算
void GltfModel::calculateBoundingBox(GltfNode* node,GltfNode* parent)
{
	BoundingBox parentBvh = parent ? parent->bvh : BoundingBox(glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX));

	if (node->mesh) {
		if (node->mesh->bb.valid) {
			node->aabb = node->mesh->bb.getAABB(node->getMatrix());
			if (node->children.size() == 0) {
				node->bvh.min = node->aabb.min;
				node->bvh.max = node->aabb.max;
				node->bvh.valid = true;
			}
		}
	}

	parentBvh.min = glm::min(parentBvh.min, node->bvh.min);
	parentBvh.max = glm::min(parentBvh.max, node->bvh.max);

	for (auto& child : node->children) {
		calculateBoundingBox(child, node);
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
	for (std::shared_ptr<Material> material:materials)
	{
		vkDestroyBuffer(device, material->sMaterialMappedBuffer.uniformBuffer, nullptr);
		vkFreeMemory(device, material->sMaterialMappedBuffer.uniformBufferMemory, nullptr);
		material->sMaterialMappedBuffer.uniformBufferMapped = nullptr;
	}

	for (int i = 0; i < textureDatas.size(); i++)
	{
		//テクスチャのデータの破棄
		textureDatas[i]->destroy(device);
	}

	deleteNodes(root,device);
}

//メッシュコライダーに頂点とインデックスを渡す
void GltfModel::setModelVertexIndex(std::vector<glm::vec3>& verticesPos, std::vector<MeshColiderVertexOption>& options, std::vector<uint32_t>& indices)
{
	uint32_t globalIndex = 0;
	int vertexCount = 0;

	for (int i = 0; i < getRootNode()->children.size(); i++)
	{
		setModelVertexIndex(getRootNode()->children[i], globalIndex, vertexCount, verticesPos, options, indices);
	}
}

void GltfModel::setModelVertexIndex(GltfNode* node, uint32_t& globalIndex, int& vertexCount, std::vector<glm::vec3>& verticesPos
	, std::vector<MeshColiderVertexOption>& options, std::vector<uint32_t>& indices)
{
	for (int i = 0; i < node->children.size(); i++)
	{
		setModelVertexIndex(node->children[i], globalIndex, vertexCount, verticesPos, options, indices);
	}

	if (node->mesh)
	{
		for (int i = 0; i < node->mesh->vertices.size(); i++)
		{
			//頂点の座標を記録
			verticesPos[i + vertexCount] = node->mesh->vertices[i].pos;
			//その頂点のほかの情報を別の配列に記録
			options[i + vertexCount].boneID = node->mesh->vertices[i].boneID1;
			options[i + vertexCount].weight = node->mesh->vertices[i].weight1;
			options[i + vertexCount].skinIndex = node->skinIndex;
		}

		//インデックスを記録
		for (int i = 0; i < node->mesh->indices.size(); i++)
		{
			indices[i + globalIndex] = node->mesh->indices[i] + vertexCount;
		}

		vertexCount += node->mesh->vertices.size();
		globalIndex += node->mesh->indices.size();
	}
}