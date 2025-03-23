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

//�A�j���[�V�����̊e�m�[�h�̍X�V����
void GltfModel::updateAllNodes(GltfNode* parent, std::vector<std::array<glm::mat4, 128>>& jointMatrices,size_t& updatedIndex)
{
	if (parent->mesh && parent->skin)
	{
		//���̃m�[�h�̏�������W���C���g�̃A�j���[�V�����s����v�Z
		parent->update(jointMatrices[parent->globalHasSkinNodeIndex],updatedIndex);
	}

	for (size_t i = 0; i < parent->children.size(); i++)
	{
		updateAllNodes(parent->children[i],jointMatrices,updatedIndex);
	}
}

//�A�j���[�V�����������擾
float GltfModel::animationDuration(std::string animationName)
{
	Animation& animation = animations[animationName];

	return animation.end - animation.start;
}

//�w�肵���A�j���[�V�����̍s����擾
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
					case AnimationChannel::PathType::TRANSLATION://���s�ړ�
						sampler.translate(i, animationTime, channel.node);
						break;
					case AnimationChannel::PathType::SCALE://�X�P�[��
						sampler.scale(i, animationTime, channel.node);
						break;
					case AnimationChannel::PathType::ROTATION://��]
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

//AABB�̌v�Z
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

//gltf���f���̏����|�[�Y�̒��_�̍��W�̍ŏ��l�ő�l�̎擾
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

//gpu��̃o�b�t�@�Ȃǂ̍폜����
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
		//�e�N�X�`���̃f�[�^�̔j��
		textureDatas[i]->destroy(device);
	}

	deleteNodes(root,device);
}

//���b�V���R���C�_�[�ɒ��_�ƃC���f�b�N�X��n��
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
			//���_�̍��W���L�^
			verticesPos[i + vertexCount] = node->mesh->vertices[i].pos;
			//���̒��_�̂ق��̏���ʂ̔z��ɋL�^
			options[i + vertexCount].boneID = node->mesh->vertices[i].boneID1;
			options[i + vertexCount].weight = node->mesh->vertices[i].weight1;
			options[i + vertexCount].skinIndex = node->skinIndex;
		}

		//�C���f�b�N�X���L�^
		for (int i = 0; i < node->mesh->indices.size(); i++)
		{
			indices[i + globalIndex] = node->mesh->indices[i] + vertexCount;
		}

		vertexCount += node->mesh->vertices.size();
		globalIndex += node->mesh->indices.size();
	}
}