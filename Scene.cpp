#include"Scene.h"

Scene::Scene()
{
}

void Scene::addObject(std::string* name,std::unique_ptr<TriMeshs> TriMeshs)
{
	TriMeshss.emplace("ray", std::move(TriMeshs));
}

void Scene::addObject(std::string* name, std::unique_ptr<DirLight> d)
{
	dirLights.emplace(*name, std::move(d));
}

TriMeshs* Scene::accessObj(std::string* name)
{
	return TriMeshss["ray"].get();
}

DirLight* Scene::accessLight(std::string* name)
{
	return dirLights[*name].get();
}