#include"Scene.h"

Scene::Scene()
{
}

void Scene::addObject(std::string* name,std::unique_ptr<Trimesh> trimesh)
{
	triMeshs.emplace("ray", std::move(trimesh));
}

void Scene::addObject(std::string* name, std::unique_ptr<DirLight> d)
{
	dirLights.emplace(*name, std::move(d));
}

Trimesh* Scene::accessObj(std::string* name)
{
	return triMeshs["ray"].get();
}

DirLight* Scene::accessLight(std::string* name)
{
	return dirLights[*name].get();
}