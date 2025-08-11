#pragma once

#include<iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include<glm/glm.hpp>
#include <glm/gtx/hash.hpp>

struct FragmentParam
{
	//透明度を上書き
	float alphaness;

	FragmentParam()
	{
		//あり得ない値にして
		//この値を使うか使わないかのフラグにする
		alphaness = -1.0f;
	}
};

struct SpecularPushConstant
{
	float roughness;
};

//頂点データ
struct Vertex {
	glm::vec3 pos;//座標
	alignas(16) glm::vec3 color;//頂点カラー
	alignas(16) glm::vec2 texCoord0;//uv座標
	glm::vec2 texCoord1;
	glm::vec3 normal;//法線

	uint32_t index;
	glm::ivec4 boneID1;//影響受けるジョイントの番号

	glm::vec4 weight1;//ジョイントの影響度

	Vertex()
	{
		pos = glm::vec3(0.0);
		color = glm::vec3(0.0);
		texCoord0 = glm::vec2(0.0);
		texCoord1 = glm::vec2(0.0);
		normal = glm::vec3(0.0);

		index = 0;

		for (uint32_t i = 0; i < 4; i++)
		{
			boneID1[i] = 0;
			weight1[i] = 0.0f;
		}
	};

	//ジョイントと影響度を設定
	void addBoneData(uint32_t boneID, float weight)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (weight1[i] == 0.0f)
			{
				boneID1[i] = boneID;
				weight1[i] = weight;

				return;
			}
		}
	}
};

//2D用の頂点構造体
struct Vertex2D
{
	//z値は、UIをレンダイングする際の優先順位に使用
	glm::vec3 pos;
	glm::vec2 uv;
};

struct RaycastPushConstant
{
	uint32_t indexCount;
	uint64_t pointer;
};