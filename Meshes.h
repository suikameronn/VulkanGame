#pragma once

#include <functional>
#include"Material.h"

const int NUM_BONES_PER_VEREX = 4;

//í∏ì_ç\ë¢ëÃÇ™éùÇ¬
struct BoneData
{
	std::vector<uint32_t> ids;
	std::vector<float> weights;

	void addBoneData(uint32_t boneID, float weight)
	{
		for (uint32_t i = 0; i < ids.size(); i++)
		{
			if (weights[i] == 0.0f)
			{
				ids[i] = boneID;
				weights[i] = weight;

				return;
			}
		}
	}
};

struct BoneInfo
{
	glm::mat4 offsetMatrix;
	glm::mat4 finalTransformation;

	BoneInfo(const glm::mat4 offset)
	{
		offsetMatrix = offset;
	}
};

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

class Meshes
{
protected:

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	//std::vector<BoneData> bones;

	glm::mat4 localTransform;

	std::shared_ptr<Material> material;

	glm::vec3 avePosition;

public:

	~Meshes();

	void pushBackVertex(Vertex* v);
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
};