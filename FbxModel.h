#pragma once

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include"Meshes.h"
#include"Material.h"

class FbxModel
{
private:
	std::vector<std::shared_ptr<FbxModel>> fbxModel;

	std::shared_ptr<Meshes> meshes;
	std::shared_ptr<Material> material;

	float startFrame;
	float endFrame;

	bool loopAnim;
	bool finish;

public:

	FbxModel(Meshes* meshes,Material* material);

	void bindFbxModel(FbxModel* model);

	const std::shared_ptr<Meshes> accessMeshes();
	const std::shared_ptr<Material> accessMaterial();

	void setAnimLoop(bool loop);

	//アニメーションを適用させるための行列を取得する
	glm::mat4* getAnimationMatrix(glm::mat4 mat);
};