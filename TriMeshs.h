#pragma once
#include<iostream>
#include<vector>

#include<glm/glm.hpp>

#include"Model.h"
#include"Material.h"

class TriMeshs:public Model
{
private:

	std::vector<Vertex>::iterator vertexItr;
	std::vector<uint32_t>::iterator indexItr;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<glm::vec3> normals;

	Vertex v;

public:
	TriMeshs();

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

	void calcNormal() override;
};