#pragma once

#include"GpuPipelineFactory.h"
#include"TextureFactory.h"
#include"Model.h"

class Cubemap : public std::enable_shared_from_this<Cubemap>
{
private:

	std::unique_ptr<Texture> boxTexture;

	std::shared_ptr<Pipeline> pipeline;

public:

	Cubemap();

	Cubemap withPipeline(const std::shared_ptr<Pipeline> p);

	Cubemap withTexture(const std::shared_ptr<Texture> t);


};