#pragma once

#include<iostream>

#include"Meshes.h"
#include"Material.h"

class FbxModel
{
private:
	std::shared_ptr<Meshes> meshes;
	std::shared_ptr<Material> material;

public:
	FbxModel() {};

	void setMeshes(Meshes* meshes);
	void setMaterial(Material* material);

	Meshes* getMeshes();
	Material* getMaterial();
};