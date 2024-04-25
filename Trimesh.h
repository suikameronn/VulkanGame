#pragma once
#include<iostream>
#include<vector>

#include<glm/glm.hpp>

#include"Material.h"

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

class Trimesh
{
private:

	std::vector<Vertex>::iterator vertexItr;
	std::vector<uint32_t>::iterator indexItr;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<glm::vec3> normals;

	Vertex v;

	std::shared_ptr<Material> material;

public:
	Trimesh();

	void setItrBeginV();
	void setItrBeginI();

	void goForwardItrV();
	void goForwardItrI();

	std::vector<Vertex>::iterator getItrV();
	std::vector<uint32_t>::iterator getItrI();

	std::vector<Vertex>::iterator getFirstItrV();
	std::vector<uint32_t>::iterator getFirstItrI();
	std::vector<Vertex>::iterator getEndItrV();
	std::vector<uint32_t>::iterator getEndItrI();


	void addVertex(Vertex v);
	void addVertex(glm::vec3 p);
	void pushBackVertex(Vertex v);
	void pushBackIndex(uint32_t u);

	uint32_t getSizeVertex();
	uint32_t getSizeIndex();

	Vertex* getFirstPointerVertex();
	uint32_t* getFirstPointerIndex();

	void setMaterial(glm::vec3* diffuse,glm::vec3* ambient,glm::vec3* specular,glm::vec3* emissive
		,float* shininess,glm::vec3* transmissive);
	std::shared_ptr<Material> getMaterial();

	void calcNormal();
};