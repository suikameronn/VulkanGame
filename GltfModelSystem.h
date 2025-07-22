#pragma once

#include"GltfModelComponent.h"
#include"AnimationComponent.h"
#include"ColiderComponent.h"

class GltfModelSystem
{
private:

	std::shared_ptr<GltfModelComponent> gltfModelComponent;
	std::shared_ptr<AnimationComponent> animationComponent;
	std::shared_ptr<ColiderComponent> coliderComponent;
}