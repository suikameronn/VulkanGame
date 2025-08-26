#include"SkyDomeFactory.h"

SkyDomeFactory::SkyDomeFactory(std::shared_ptr<SkyDomeBuilder> b)
{
	builder = b;
}

//SkyDome‚ğì¬‚µ‚ÄA‚»‚ÌID‚ğ•Ô‚·
uint32_t SkyDomeFactory::Create(const SkyDomeProperty& property)
{
	const uint32_t id = hash(property);

	if (skydomeMap[id])
	{
		return id;
	}

	std::shared_ptr<SkyDome> skydome = builder->Create(property);

	skydomeMap[id] = skydome;

	return id;
}

//ID‚©‚çSkyDome‚ğæ“¾‚·‚é
std::shared_ptr<SkyDome> SkyDomeFactory::GetSkyDome(const uint32_t id)
{
	if (skydomeMap[id])
	{
		return skydomeMap[id];
	}

	throw std::runtime_error("Skydome : fatal id");
}

uint32_t SkyDomeFactory::getCubemapModelID()
{
	return builder->getCubemapModelID();
}