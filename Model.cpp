#include"Model.h"

Model::Model()
{
	meshes = nullptr;
	material = nullptr;
}

Model::Model(Meshes* meshes, Material* material)
{
	this->meshes = meshes;
	this->material = material;
}

void Model::setMeshes(Meshes* meshes)
{
	this->meshes = meshes;
}

void Model::setMaterial(Material* material)
{
	this->material = material;
}

void Model::setImageData(ImageData* image)
{
	this->material->setImageData(image);
}

Meshes* Model::getMeshes()
{
	return this->meshes;
}

Material* Model::getMaterial()
{
	return this->material;
}