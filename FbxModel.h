#pragma once

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include"Meshes.h"
#include"Material.h"

class FbxModel
{
private:
	std::vector<std::shared_ptr<Meshes>> meshes;

	float startFrame;
	float endFrame;

	bool loopAnim;
	bool finish;

public:

	FbxModel();

	void addMeshes(Meshes* mesh);

	std::shared_ptr<Meshes> getMeshes(uint32_t i);

	void cleanupVulkan();

	uint32_t getMeshesSize();

	void setAnimLoop(bool loop);

	//�A�j���[�V������K�p�����邽�߂̍s����擾����
	glm::mat4* getAnimationMatrix(glm::mat4 mat);
};