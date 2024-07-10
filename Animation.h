#pragma once

#include<vector>
#include<map>
#include <assimp/scene.h>           // Output data structure
#include<glm/glm.hpp>

class Animation
{
private:
	bool play = false;

	float timeTick;
	float duration;
	float timeSeconds;
	float animationTime;

	std::vector<std::map<float, aiQuaternion>> animationRotKey;
	std::vector<std::map<float, glm::vec3>> animationScaleKey;
	std::vector<std::map<float, glm::vec3>> animationPositionKey;

	std::vector<std::vector<glm::mat4>> transforms;

public:

	Animation(float timeInTick,float duration,int boneNum);

	void setAnimationRotKey(std::map<float, aiQuaternion>& keyTimeQuat);
	void setAnimationScaleKey(std::map<float, glm::vec3>& keyTimeScale);
	void setAnimationPositionKey(std::map<float, glm::vec3>& keyTimePosition);

	void StartAnim();
	void PauseAnim();

	void setBoneTransform(std::vector < std::vector<glm::mat4>> transforms);
	std::vector<glm::mat4> &getBoneTransform();
};