#include"Animation.h"

Animation::Animation(float timeTick, float duration,int boneNum)
{
	this->timeTick = timeTick;
	this->duration = duration;
	this->timeSeconds = 0.0f;

	transforms = std::vector<std::vector<glm::mat4>>(boneNum, std::vector<glm::mat4>(4));
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

void Animation::getBoneTransform(float animTime,glm::mat4 transform)
{
	glm::mat4 mat = rootNode->getAnimMatrix(animTime);


}

void Animation::StartAnim()
{
	play = true;
}

void Animation::PauseAnim()
{
	play = false;
}

std::vector<glm::mat4>& Animation::getBoneTransform()
{
	animationTime = fmod(timeSeconds * timeTick,duration);

	timeSeconds += 0.1f;

	if (play)
	{
		return this->transforms[animationTime];
	}

}