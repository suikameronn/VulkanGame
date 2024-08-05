#include"Animation.h"

#include"FbxModel.h"

Animation::Animation(float timeTick, float duration)
{
	this->timeTick = timeTick;
	this->duration = duration;
}

Animation::~Animation()
{
	DeleteAnimTree(this->rootNode);
}

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

void Animation::setFinalTransform(float animationTime, std::vector<glm::mat4>& boneFinalTransforms, AnimNode* node, glm::mat4 parentMatrix, FbxModel* model)
{
	node->getAnimMatrix(animationTime, parentMatrix);

	if (model->containBone(node->getName()))
	{
		boneFinalTransforms[model->getBoneToMap(node->getName())] = parentMatrix;
	}

	for (uint32_t i = 0; i < node->getChildrenCount(); i++)
	{
		setFinalTransform(animationTime,boneFinalTransforms, node->getChildren(i), parentMatrix, model);
	}
}

void Animation::setFinalTransform(float animationTime, std::vector<glm::mat4>& boneFinalTransforms, FbxModel* model)
{
	glm::mat4 identity(1.0f);
	rootNode->getAnimMatrix(animationTime,identity);

	if (model->containBone(rootNode->getName()))
	{
		boneFinalTransforms[model->getBoneToMap(rootNode->getName())] = identity;
	}

	for (uint32_t i = 0; i < rootNode->getChildrenCount(); i++)
	{
		setFinalTransform(animationTime,boneFinalTransforms, rootNode->getChildren(i), identity, model);
	}
}

/*
std::vector<glm::mat4>& Animation::getBoneTransform()
{
	animationTime = fmod(timeSeconds * timeTick,duration);

	timeSeconds += 0.1f;

	if (play)
	{
		return this->transforms[animationTime];
	}

}
*/