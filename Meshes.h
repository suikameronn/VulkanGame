#pragma once
#include <array>
#include <functional>
#include"Material.h"

#define MAXBONEINFO 250

const int NUM_BONES_PER_VEREX = 4;

//’¸“_\‘¢‘Ì‚ªŽ‚Â
struct BoneData
{
	uint32_t infoID;
	float weight;

	BoneData() {};

	BoneData(uint32_t infoID, float weight)
	{
		this->infoID = infoID;
		this->weight = weight;
	}
};

struct BoneInfo
{
	std::vector<glm::mat4> offsetMatrix;
	std::vector<glm::mat4> finalTransform;

	BoneInfo()
	{
		offsetMatrix.push_back(glm::mat4(1.0f));
		finalTransform.push_back(glm::mat4(1.0f));
	}

	void setOffsetMatrix(uint32_t index, glm::mat4 matrix)
	{
		if (index < offsetMatrix.size())
		{
			offsetMatrix[index] = matrix;
		}
	}

	void setFinalTransform(uint32_t index, glm::mat4 matrix)
	{
		if (index < finalTransform.size())
		{
			finalTransform[index] = matrix;
		}
	}

	void resizeBoneInfo(uint32_t size)
	{
		offsetMatrix.resize(size);
		finalTransform.resize(size);
	}
};

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal;

	uint32_t index;
	std::array<uint32_t, MAXBONEINFO> boneIDs;
	std::array<float, MAXBONEINFO> weights;

	Vertex()
	{
		for (uint32_t i = 0; i < MAXBONEINFO; i++)
		{
			boneIDs[i] = 0;
			weights[i] = 0.0f;
		}

		index = 1;
	};
	
	void addBoneData(uint32_t boneID, float weight)
	{
		if (index < MAXBONEINFO)
		{
			boneIDs[index] = boneID;
			weights[index] = weight;

			index++;
		}
	}

	int getBoneDataSize()
	{
		return index - 1;
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