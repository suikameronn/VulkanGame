#include"FbxModel.h"

FbxModel::FbxModel()
{
}

void FbxModel::setMinMaxVertexPos(glm::vec3 min, glm::vec3 max)
{
	minPos = min;
	maxPos = max;

	pivot = glm::vec3((min.x + max.x) / 2, (min.y + max.y) / 2, (min.z + max.z) / 2);
}

void FbxModel::getMinMaxVertexPos(glm::vec3& min, glm::vec3& max)
{
	min = minPos;
	max = maxPos;
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

void FbxModel::setAnimation(ACTION action, std::shared_ptr<Animation> animation)
{
	animations[action] = animation;
}

std::array<glm::mat4, 250> FbxModel::getAnimationMatrix()
{
	std::array<glm::mat4, 250> transforms;
	std::fill(transforms.begin(), transforms.end(), glm::mat4(0.0f));

	//std::copy(boneInfo.offsetMatrix.begin(), boneInfo.offsetMatrix.end(), transforms.begin());

	//poses["idle"]->setFinalTransform(transforms);

	return transforms;
}

std::array<glm::mat4,250> FbxModel::getAnimationMatrix(float animationTime,ACTION action)
{
	std::array<glm::mat4, 250> transforms;
	std::fill(transforms.begin(), transforms.end(), glm::mat4(1.0f));

	animations[action]->setFinalTransform(animationTime, transforms,this);

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

		return index;
	}
	else
	{
		return m_BoneNameToIndexMap[boneName];
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