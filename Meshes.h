#pragma once
#include <array>
#include <functional>
#include"Material.h"

#define MAXBONEINFO 4

const int NUM_BONES_PER_VEREX = 4;

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal;

	uint32_t index;
	std::array<int, MAXBONEINFO> boneID1;

	std::array<float, MAXBONEINFO> weight1;

	Vertex()
	{
		pos = glm::vec3(0.0);
		color = glm::vec3(0.0);
		texCoord = glm::vec2(0.0);
		normal = glm::vec3(0.0);

		index = 0;

		for (uint32_t i = 0; i < MAXBONEINFO; i++)
		{
			boneID1[i] = 0;
			weight1[i] = 0.0f;
		}
	};
	
	void addBoneData(uint32_t boneID, float weight)
	{
		for (int i = 0; i < MAXBONEINFO; ++i)
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

/*
namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}
*/

class Meshes
{
protected:

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	glm::mat4 localTransform;

	std::shared_ptr<Material> material;

	glm::vec3 avePosition;

public:

	~Meshes();

	void pushBackVertex(Vertex& v);
	void pushBackIndex(uint32_t i);
	void setMaterial(std::shared_ptr<Material> material);
	const std::shared_ptr<Material> getMaterial() { return material; }

	std::vector<Vertex>::iterator getVertItr();
	std::vector<uint32_t>::iterator getIndiItr();

	Vertex* getVertexPoint();
	uint32_t* getIndexPoint();

	uint32_t getVerticesSize();
	uint32_t getIndicesSize();

	uint32_t getTextureCount() { material->getImageDataCount(); }

	void vertResize(uint32_t size);
	void indexResize(uint32_t size);

	void setLocalTransform(glm::mat4 transform);
	glm::mat4 getLocalTransform();

	void addBoneData(uint32_t index,uint32_t infoID, float weight);
};