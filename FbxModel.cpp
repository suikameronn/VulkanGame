#include"FbxModel.h"

FbxModel::FbxModel()
{
	averageLocalPos = { 0,0,0 };
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

void FbxModel::calcAveragePos()
{
	int vertexCount = 0;
	
	for (auto itr = meshes.begin(); itr != meshes.end(); itr++)
	{
		Vertex* v = (*itr)->getVertexPoint();
		for (int i = 0; i < 3; i++)
		{
			averageLocalPos += v[i].pos;
			vertexCount++;
		}
	}

	averageLocalPos /= vertexCount;
}

glm::vec3 FbxModel::getAverageLocalPos()
{
	return averageLocalPos;
}

void FbxModel::cleanupVulkan()
{
	for (auto itr = meshes.begin(); itr != meshes.end(); itr++)
	{
		std::shared_ptr<Material> material = (*itr)->getMaterial();
		if (material->hasImageData())
		{
			material->cleanUpVulkan();
		}
	}
}

/*

//アニメーションの行列を求める
glm::mat4* getAnimationMatrix(glm::mat4 mat)
{

}
*/