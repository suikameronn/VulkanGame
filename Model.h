#pragma once
#include<vector>

#include"Meshes.h"
#include"Material.h"

class Model
{
private:
	Meshes* meshes;

	Material* material;

public:
	Model();
	Model(Meshes* m, Material* material);

	void setMeshes(Meshes* meshes);
	void setImageData(ImageData* image);
	void setMaterial(Material* material);

	Meshes* getMeshes();
	Material* getMaterial();
};