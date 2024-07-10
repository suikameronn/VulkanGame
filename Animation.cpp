#include"Animation.h"

Animation::Animation(float timeTick, float duration,int boneNum)
{
	this->timeTick = timeTick;
	this->duration = duration;
	this->timeSeconds = 0.0f;

	transforms = std::vector<std::vector<glm::mat4>>(boneNum, std::vector<glm::mat4>(4));
}

void Animation::setBoneTransform(std::vector < std::vector<glm::mat4>> transforms)
{
	this->transforms = transforms;
}

void Animation::setAnimationRotKey(std::map<float, aiQuaternion>& keyTimeQuat)
{
	animationRotKey.push_back(keyTimeQuat);
}

void Animation::setAnimationScaleKey(std::map<float, glm::vec3>& keyTimeScale)
{
	animationScaleKey.push_back(keyTimeScale);
}

void Animation::setAnimationPositionKey(std::map<float, glm::vec3>& keyTimePosition)
{
	animationPositionKey.push_back(keyTimePosition);
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