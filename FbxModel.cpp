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
		for (int i = 0; i < (*itr)->getVerticesSize(); i++)
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

int FbxModel::setBoneToMap(std::string boneName)
{
	if (m_BoneNameToIndexMap.find(boneName) == m_BoneNameToIndexMap.end())
	{
		int index = m_BoneNameToIndexMap.size();
		m_BoneNameToIndexMap[boneName] = index;
		return index;
	}
	else
	{
		return m_BoneNameToIndexMap[boneName];
	}
}

void FbxModel::setBoneInfo(int id,const glm::mat4 mat)
{
	if (id == m_BoneInfo.size())
	{
		BoneInfo bi(mat);
		m_BoneInfo.push_back(bi);
	}
}

void FbxModel::addBoneData(int vertID, int boneID, float weight)
{
	m_Bones[vertID].addBoneData(boneID, weight);
}

int FbxModel::getBoneNum()
{
	return m_BoneInfo.size();
}

void FbxModel::setAnimation(std::string name, std::shared_ptr<Animation> animation)
{
	animations[name] = animation;
}

/*

//アニメーションの行列を求める
glm::mat4* getAnimationMatrix(glm::mat4 mat)
{

}
*/