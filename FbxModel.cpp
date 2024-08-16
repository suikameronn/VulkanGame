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

void FbxModel::setPose(std::string name, std::shared_ptr<Pose> pose)
{
	poses[name] = pose;
}

void FbxModel::setAnimation(std::string name, std::shared_ptr<Animation> animation)
{
	animations[name] = animation;
}

std::array<glm::mat4, 251> FbxModel::getAnimationMatrix()
{
	std::array<glm::mat4, 251> transforms;
	std::fill(transforms.begin(), transforms.end(), glm::mat4(0.0f));

	poses["idle"]->setFinalTransform(transforms);

	return transforms;
}

std::array<glm::mat4,251> FbxModel::getAnimationMatrix(float animationTime,std::string animationName)
{
	std::array<glm::mat4, 251> transforms;
	std::fill(transforms.begin(), transforms.end(), glm::mat4(1.0f));

	animations[animationName]->setFinalTransform(animationTime, transforms,this);

	return transforms;
}

bool FbxModel::containBone(std::string nodeName)
{
	if (m_BoneNameToIndexMap.find(nodeName) != m_BoneNameToIndexMap.end())
	{
		return true;
	}

	return false;
}

int FbxModel::getBoneToMap(std::string boneName)
{
	if (m_BoneNameToIndexMap.find(boneName) == m_BoneNameToIndexMap.end())
	{
		int index = m_BoneNameToIndexMap.size();
		m_BoneNameToIndexMap[boneName] = index;
		return index + 1;
	}
	else
	{
		return m_BoneNameToIndexMap[boneName] + 1;
	}
}

void FbxModel::setBoneInfo(int id, const glm::mat4 mat)
{
	if (id == boneInfo.offsetMatrix.size())
	{
		boneInfo.offsetMatrix.push_back(mat);
	}
}

int FbxModel::getBoneNum()
{
	return boneInfo.offsetMatrix.size();
}

glm::mat4 FbxModel::getBoneOffset(int index)
{
	return boneInfo.offsetMatrix[index];
}

/*

//アニメーションの行列を求める
glm::mat4* getAnimationMatrix(glm::mat4 mat)
{

}
*/