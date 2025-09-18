#pragma once

#include<iostream>
#include<glm/glm.hpp>

#include"GpuBufferFactory.h"
#include"DescriptorSetFactory.h"
#include"TextureFactory.h"

//各種類のライトの最大個数
#define LIGHT_MAX 10
//ライトの種類の数
#define LIGHT_TYPE_COUNT 2

struct PointLightComp
{
	//シーン全体のライトの配列のインデックス
	//このindexが配列の上の番号
	int index;

	glm::vec3 position;
	glm::vec4 color;
};

struct DirectionLightComp
{
	size_t entityID;

	int index;

	glm::vec3 position;
	glm::vec3 direction;
	glm::vec4 color;

	DirectionLightComp(const size_t& entity)
	{
		entityID = entity;
	}
};

struct PointLightUniform
{
	uint32_t lightCount;
	alignas(16) std::array<glm::vec4, LIGHT_MAX> position;
	alignas(16) std::array<glm::vec4, LIGHT_MAX> color;
	alignas(16) std::array<glm::mat4, LIGHT_MAX> viewProj;
};

struct DirectionLightUniform
{
	uint32_t lightCount;
	alignas(16) std::array<glm::vec4, LIGHT_MAX> position;
	alignas(16) std::array<glm::vec4, LIGHT_MAX> direction;
	alignas(16) std::array<glm::vec4, LIGHT_MAX> color;
	alignas(16) std::array<glm::mat4, LIGHT_MAX> viewProj;
};

struct SceneLight
{
	int dirIndex;
	int pointIndex;
	DirectionLightUniform dirUniform;
	PointLightUniform pointUniform;

	//ライトの種類の数だけ配列の要素が増える
	//各ライトの行列などは、ユニフォームバッファの配列に入れる
	std::array<std::shared_ptr<GpuBuffer>, LIGHT_TYPE_COUNT> uniformBuffer;

	//ライトの種類の数だけ配列の要素数が増える
	std::array<std::shared_ptr<DescriptorSet>, LIGHT_TYPE_COUNT> uniformDescriptorSet;

	//ライトの種類の数とライトの数だけ要素数が増える
	//ただ、実際には同一の種類のライトの場合、同一のテクスチャで管理され
	//各ライトは一つのテクスチャの持つ複数のレイヤを割り当てられ
	//そこに各ライト視点のシャドウマップが記録される
	std::array<std::vector<std::shared_ptr<FrameBuffer>>, LIGHT_TYPE_COUNT> frameBuffer;

	//ライトの種類の数だけ要素を持つ
	//同一種類のライトのシャドウマップは、この一つのテクスチャの
	//割り当てられたレイヤに記録される
	std::array<std::shared_ptr<Texture>, LIGHT_TYPE_COUNT> shadowMap;

	//ライトはまとめて一つのVkDescriptorSetで管理する
	std::shared_ptr<DescriptorSet> texDescriptorSet;

	SceneLight()
	{
		dirIndex = 0;
		pointIndex = 0;

		for (auto buffer : uniformBuffer)
		{
			buffer.reset();
		}

		for (auto desc : uniformDescriptorSet)
		{
			desc.reset();
		}

		for (auto frameArray : frameBuffer)
		{
			for (auto frame : frameArray)
			{
				frame.reset();
			}
		}

		for (auto tex : shadowMap)
		{
			tex.reset();
		}

		texDescriptorSet.reset();
	}

	int getPointLightIndex()
	{
		int index = pointIndex;
		
		pointIndex++;

		return index;
	}

	int getDirectionLightIndex()
	{
		int index = dirIndex;

		dirIndex++;

		return index;
	}
};