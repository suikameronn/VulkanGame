#pragma once

#include<vector>

#include<glm/glm.hpp>

class Animation
{
private:
	bool play = false;

	float timeTick;
	float duration;
	float timeSeconds;
	float animationTime;

	std::vector<std::vector<glm::mat4>> transforms;

public:

	Animation(float timeInTick,float duration,int boneNum);

	void StartAnim();
	void PauseAnim();

	void setBoneTransform(std::vector < std::vector<glm::mat4>> transforms);
	std::vector<glm::mat4> &getBoneTransform();
};