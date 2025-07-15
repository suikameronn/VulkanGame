#pragma once

#include"VulkanCore.h"
#include"Cubemap.h"
#include"IBL.h"

class EnvironmentLightSet
{
private:

	std::shared_ptr<Cubemap> cubemap;
	std::shared_ptr<IBL> ibl;

public:



};