#include"Animation.h"

#include"GltfModel.h"

Animation::Animation()
{

}

Animation::Animation(float startTime, float endTime,float duration)
{
	this->startTime = startTime;
	this->endTime = endTime;
	this->duration = duration;
}

Animation::~Animation()
{
	if (this->rootNode != nullptr)
	{
		//DeleteAnimTree(this->rootNode);
	}
}

/*

void Animation::DeleteAnimTree(AnimNode* node)
{
	if (node->getChildrenCount() != 0)
	{
		for (int i = 0; i < node->getChildrenCount(); i++)
		{
			DeleteAnimTree(node->getChildren(i));
		}
	}

	delete node;
}

void Animation::setFinalTransform(float animationTime, std::array<glm::mat4, 250>& boneFinalTransforms, AnimNode* node, glm::mat4 parentMatrix,GltfModel* model)
{
	glm::mat4 childMatrix = node->getAnimMatrix(animationTime, parentMatrix);

	if (model->containBone(node->getName()))
	{
		int boneToMap = model->getBoneToMap(node->getName());

		if (boneToMap < 250)
		{
			boneFinalTransforms[boneToMap] = inverseGlobalTransform * childMatrix * model->getBoneOffset(boneToMap);
		}
	}

	for (uint32_t i = 0; i < node->getChildrenCount(); i++)
	{
		setFinalTransform(animationTime,boneFinalTransforms, node->getChildren(i), childMatrix,model);
	}
}

void Animation::setFinalTransform(float animationTime, std::array<glm::mat4, 250>& boneFinalTransforms,GltfModel* model)
{
	animationTime = fmod(animationTime, duration);

	glm::mat4 identity(1.0f);
	glm::mat4 rootNodeMatrix = rootNode->getAnimMatrix(animationTime,identity);

	if (model->containBone(rootNode->getName()))
	{
		int boneToMap = model->getBoneToMap(rootNode->getName());
		if (boneToMap < 250)
		{
			boneFinalTransforms[boneToMap] = inverseGlobalTransform * rootNodeMatrix * model->getBoneOffset(boneToMap);
		}
	}

	for (uint32_t i = 0; i < rootNode->getChildrenCount(); i++)
	{
		setFinalTransform(animationTime,boneFinalTransforms, rootNode->getChildren(i), rootNodeMatrix,model);
	}
}



//ポーズの行列をセットする
void Pose::setPoseMatrix(std::array<glm::mat4, 250>& matrix)
{
	std::copy(matrix.begin(), matrix.end(), boneMatrix.begin());
}

//ポーズの行列を返す
void Pose::setFinalTransform(std::array<glm::mat4, 250>& boneFinalTransforms)
{
	std::copy(boneMatrix.begin(), boneMatrix.end(), boneFinalTransforms.begin());
}

*/