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

void GltfModel::updateAllNodes(GltfNode* parent, std::vector<std::array<glm::mat4, 128>>& jointMatrices)
{
	if (parent->mesh && parent->skin)
	{
		parent->update(jointMatrices[parent->globalHasSkinNodeIndex]);
	}

	for (size_t i = 0; i < parent->children.size(); i++)
	{
		updateAllNodes(parent->children[i],jointMatrices);
	}
}

float GltfModel::animationDuration(std::string animationName)
{
	Animation& animation = animations[animationName];
	return animation.end - animation.start;
}

void GltfModel::updateAnimation(float animationTime,std::string animationName, std::vector<std::array<glm::mat4, 128>>& jointMatrices)
{
	if (animations.empty()) {
		std::cout << ".glTF does not contain animation." << std::endl;
		return;
	}
	Animation& animation = animations[animationName];

	bool updated = false;
	for (auto& channel : animation.channels) {
		AnimationSampler sampler = animation.samplers[channel.samplerIndex];
		if (sampler.inputs.size() > sampler.outputsVec4.size()) {
			continue;
		}

		for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
			if ((animationTime >= sampler.inputs[i]) && (animationTime <= sampler.inputs[i + 1])) {
				float u = std::max(0.0f, animationTime - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
				if (u <= 1.0f) {
					switch (channel.path) {
					case AnimationChannel::PathType::TRANSLATION:
						sampler.translate(i, animationTime, channel.node);
						break;
					case AnimationChannel::PathType::SCALE:
						sampler.scale(i, animationTime, channel.node);
						break;
					case AnimationChannel::PathType::ROTATION:
						sampler.rotate(i, animationTime, channel.node);
						break;
					}
					updated = true;
				}
			}
		}
	}

	if (updated) {
		updateAllNodes(root,jointMatrices);
	}
}

void GltfModel::calculateBoundingBox(GltfNode* node,GltfNode* parent)
{
	BoundingBox parentBvh = parent ? parent->bvh : BoundingBox(glm::vec3(10000000.0f), glm::vec3(-10000000.0f));

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

void GltfModel::getVertexMinMax(GltfNode* node, glm::vec3& min, glm::vec3& max)
{
	if (node->bvh.valid)
	{
		min = glm::min(min, node->bvh.min);
		max = glm::max(max, node->bvh.max);
	}

	for (int i = 0; i < node->children.size(); i++)
	{
		getVertexMinMax(node->children[i], min, max);
	}
}

void GltfModel::setAABBMatrix(glm::vec3 min,glm::vec3 max)
{
	boundingBox.min = min;
	boundingBox.max = max;
}

void GltfModel::cleanUpVulkan(VkDevice& device)
{
	cleanUpVulkan(device, root);
}

void GltfModel::cleanUpVulkan(VkDevice& device, GltfNode* node)
{
	if (node->mesh)
	{
		Mesh* mesh = node->mesh;
		for (size_t i = 0; i < mesh->primitives.size(); i++)
		{
			if (mesh->primitives[i].material)
			{
				if (mesh->primitives[i].material->hasTextureData())
				{
					mesh->primitives[i].material->getTextureData()->destroy(device);
				}
			}
		}
		mesh->descriptorInfo.destroy(device);
	}

	for (size_t i = 0; i < node->children.size(); i++)
	{
		cleanUpVulkan(device, node->children[i]);
	}
}