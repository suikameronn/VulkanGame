#pragma once
#include"Object.h"
#include"Material.h"

#include <glm/gtx/hash.hpp>
#include <functional>

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

class ImageData
{
private:
	uint32_t id;
	int width;
	int height;
	int texChannels;
	std::vector<unsigned char> pixels;

public:
	ImageData(uint32_t id, uint32_t width, uint32_t height,
		uint32_t texChannels, std::vector<unsigned char>& pixels)
	{
		this->id = id;
		this->width = width;
		this->height = height;
		this->texChannels = texChannels;
		this->pixels = pixels;
	}

	uint32_t getId()
	{
		return this->id;
	}

	int getWidth()
	{
		return this->width;
	}

	int getHeight()
	{
		return this->height;
	}

	int getTexChannels()
	{
		return this->texChannels;
	}

	unsigned char* getPixelsData()
	{
		return this->pixels.data();
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

class Model:Object
{
protected:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	glm::vec3 avePosition;

	std::shared_ptr<ImageData> texture;
	std::shared_ptr<Material> material;

public:
	void setPosition(std::vector<glm::vec3>* positions);
	glm::vec3* getPosition();

	void pushBackVertex(Vertex* v);
	void pushBackIndex(uint32_t i);
	
	std::vector<Vertex>::iterator getVertItr();
	std::vector<uint32_t>::iterator getIndiItr();

	Vertex* getVertexPoint();
	uint32_t* getIndexPoint();

	uint32_t getVerticesSize();
	uint32_t getIndicesSize();

	void setImageData(std::shared_ptr<ImageData> image);
	ImageData* getImageData();

	void setMaterial(glm::vec3* diffuse, glm::vec3* ambient, glm::vec3* specular, glm::vec3* emissive
		, float* shininess, glm::vec3* transmissive);
	std::shared_ptr<Material> getMaterial();
};