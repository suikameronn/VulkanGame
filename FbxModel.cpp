#include"FbxModel.h"

<<<<<<< HEAD
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
=======
FbxModel::FbxModel()
{
}

void FbxModel::addMeshes(Meshes* mesh)
{
	meshes.push_back(std::shared_ptr<Meshes>(mesh));
}

std::shared_ptr<Meshes> FbxModel::getMeshes(uint32_t i)
{
	return meshes[i];
}

uint32_t FbxModel::getMeshesSize()
{
	return meshes.size();
}


std::shared_ptr<Material> FbxModel::accessMaterial()
{
	return material;
}

/*

//アニメーションの行列を求める
glm::mat4* getAnimationMatrix(glm::mat4 mat)
{

}
*/
>>>>>>> master
