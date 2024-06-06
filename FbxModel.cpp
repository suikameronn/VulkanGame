#include"FbxModel.h"

FbxModel::FbxModel(Meshes* meshes, Material* material)
{
	this->meshes = std::shared_ptr<Meshes>(meshes);
	this->material = std::shared_ptr<Material>(material);
}

const std::shared_ptr<Meshes> FbxModel::accessMeshes()
{
	return meshes;
}

const std::shared_ptr<Material> FbxModel::accessMaterial()
{
	return material;
}

//�A�j���[�V�����̍s������߂�
glm::mat4* getAnimationMatrix(glm::mat4 mat)
{

}