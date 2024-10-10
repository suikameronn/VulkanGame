#pragma once

#include<array>

#include"Object.h"
#include"EnumList.h"

struct Simplex
{
private:
	std::array<glm::vec3, 4> points;

public:
	int size;

	Simplex()
	{
		size = 0;
		glm::vec3 zero = glm::vec3(0.0f, 0.0f, 0.0f);
		points = { zero };
	}

	void push_front(glm::vec3 v)
	{
		points = { v,points[0],points[1] ,points[2] };
		size = std::min(size + 1, 4);
	}

	Simplex& operator=(std::initializer_list<glm::vec3> list)
	{
		size = 0;

		for (glm::vec3 point : list)
			points[size++] = point;

		return *this;
	}

	glm::vec3& operator[](int i) { return points[i]; }
};

class Colider
{
private:
	glm::vec3 pivot;
	float width;
	float height;
	float depth;
	
	std::vector<glm::vec3> coliderVertices;
	glm::vec4 color;

	std::vector<uint32_t> coliderIndices;

	BufferObject pointBuffer;
	MappedBuffer mappedBuffer;
	DescriptorInfo descInfo;
	DescSetData descSetData;

	bool sameDirection(glm::vec3 point, glm::vec3 dir) { return glm::dot(dir, point) > 0.0f; }

	bool Line(Simplex& simplex,glm::vec3 dir);
	bool Triangle(Simplex& simplex, glm::vec3 dir);
	bool Tetrahedron(Simplex& simplex, glm::vec3 dir);

public:
	Colider(glm::vec3 pos, float right, float left, float top, float bottom, float front, float back);
	Colider(glm::vec3 pos, glm::vec3 min, glm::vec3 max);

	void reflectMovement(glm::mat4& transform);

	glm::vec3 getPivot() { return pivot; }
	glm::vec3 getFurthestPoint(glm::vec3 dir);
	glm::vec3 getSupportVector(std::shared_ptr<Colider> oppColider, glm::vec3 dir);
	bool nextSimplex(Simplex& simplex, glm::vec3 dir);
	glm::vec3 getClosestLineToVertex(glm::vec3 lineStart, glm::vec3 lineFinish, glm::vec3 point);

	void setDescriptorSet(VkDescriptorSet descriptorSet);

	BufferObject* getPointBuffer();
	MappedBuffer* getMappedBuffer();
	DescriptorInfo& getDescInfo();
	DescSetData& getDescSetData() { return descSetData; }

	virtual bool Intersect(std::shared_ptr<Colider> oppColider);

	glm::vec3* getColiderVertices();
	int getColiderVerticesSize();

	int* getColiderIndices();
	int getColiderIndicesSize();

	void cleanupVulkan();
};