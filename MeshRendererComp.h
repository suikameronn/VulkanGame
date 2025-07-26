#pragma once

#include"GltfModelComp.h"

struct MeshRendererComp
{
	std::shared_ptr<GpuBuffer> modelMatBuffer;

	std::vector<std::shared_ptr<GpuBuffer>> animMatBuffer;

	std::shared_ptr<DescriptorSet> modelMatDesc;

	std::vector<std::shared_ptr<DescriptorSet>> animMatDesc;
};