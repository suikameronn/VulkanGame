#include"FbxModel.h"

void FbxModel::setMeshes(Meshes* meshes)
{
	this->meshes = std::shared_ptr<Meshes>(meshes);
}

void FbxModel::setMaterial(Material* material)
{
	this->material = std::shared_ptr<Material>(material);
}

Meshes* FbxModel::getMeshes()
{
	return meshes.get();
}

Material* FbxModel::setMaterial()
{
	return material.get();
}