#pragma once

#include<map>
#include<array>

#include"Object.h"
#include"EnumList.h"

#include<random>

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

	void setSimplexVertices(std::vector<glm::vec3>& polytope)
	{
		polytope.resize(points.size());
		std::copy(points.begin(), points.end(), polytope.begin());
	}
};

class Colider
{
private:

	glm::vec3 min, max;
	glm::vec3 transformedMin, transformedMax;
	float width;
	float height;
	float depth;

	glm::mat4 scaleMat;
	glm::mat4 transform;

	std::map<float, std::pair<glm::vec3, glm::vec3>> animationAABB;
	
	std::vector<glm::vec3> coliderVertices;
	std::vector<glm::vec3> originalVertexPos;
	glm::vec4 color;

	std::vector<uint32_t> coliderIndices;
	std::vector<uint32_t> satIndices;

	BufferObject pointBuffer;
	MappedBuffer mappedBuffer;
	DescriptorInfo descInfo;
	DescSetData descSetData;

	bool sameDirection(glm::vec3 dir, glm::vec3 point) { return glm::dot(dir, point) > 0.0f; }

	bool Line(Simplex& simplex,glm::vec3& dir);
	bool Triangle(Simplex& simplex, glm::vec3& dir);
	bool Tetrahedron(Simplex& simplex, glm::vec3& dir);

	glm::vec3 getFurthestPoint(glm::vec3 dir);
	glm::vec3 getSupportVector(std::shared_ptr<Colider> oppColider, glm::vec3 dir);
	bool nextSimplex(Simplex& simplex, glm::vec3& dir);
	glm::vec3 getClosestLineToVertex(glm::vec3 lineStart, glm::vec3 lineFinish, glm::vec3 point);
	bool GJK(std::shared_ptr<Colider> oppColider,glm::vec3& collisionDepthVec);
	void EPA(std::shared_ptr<Colider> oppColider, Simplex& simplex, glm::vec3& collisionDepthVec);
	bool SAT(std::shared_ptr<Colider> oppColider, float& collisionDepth, glm::vec3& collisionNormal);

	void addIfUniqueEdge(std::vector<std::pair<size_t, size_t>>& edges, const std::vector<size_t>& faces,
						 size_t a, size_t b);

	std::pair<std::vector<glm::vec4>, size_t> getFaceNormals(
		std::vector<glm::vec3>& vertices,
		std::vector<size_t>& faces);

public:
	Colider(glm::vec3 min,glm::vec3 max);

	void initFrameSettings();

	glm::vec3 scale;
	glm::mat4 getScaleMat();

	void reflectMovement(glm::mat4& transform);
	glm::mat4& getTransformMatrix() { return transform; }

	/*
	int getSatIndicesSize() { return static_cast<int>(satIndices.size()); }
	uint32_t* getSatIndicesPointer() { return satIndices.data(); }
	*/

	void projection(float& min, float& max, glm::vec3& minVertex, glm::vec3& maxVertex, glm::vec3& axis);

	void setDescriptorSet(VkDescriptorSet descriptorSet);

	BufferObject* getPointBufferData();
	MappedBuffer* getMappedBufferData();
	DescriptorInfo& getDescInfo();
	DescSetData& getDescSetData() { return descSetData; }

	virtual bool Intersect(std::shared_ptr<Colider> oppColider, glm::vec3& collisionVector);
	virtual bool Intersect(std::shared_ptr<Colider> oppColider);
	virtual bool Intersect(glm::vec3 origin, glm::vec3 dir, float length);

	glm::vec3* getColiderOriginalVertices();
	glm::vec3* getColiderVertices();
	int getColiderVerticesSize();

	int* getColiderIndices();
	int getColiderIndicesSize();

	void cleanupVulkan();
};