#include"Colider.h"

#include"VulkanBase.h"

Colider::Colider(glm::vec3 min, glm::vec3 max)
{
	coliderVertices.resize(8);

	coliderVertices[0] = glm::vec3(min.x, min.y, min.z);
	coliderVertices[1] = glm::vec3(max.x, min.y, min.z);
	coliderVertices[2] = glm::vec3(max.x, min.y, max.z);
	coliderVertices[3] = glm::vec3(min.x, min.y, max.z);
	coliderVertices[4] = glm::vec3(min.x, max.y, min.z);
	coliderVertices[5] = glm::vec3(max.x, max.y, min.z);
	coliderVertices[6] = glm::vec3(max.x, max.y, max.z);
	coliderVertices[7] = glm::vec3(min.x, max.y, max.z);
	for (int i = 0; i < coliderVertices.size(); i++)
	{
		coliderVertices[i] = glm::vec3(glm::vec4(coliderVertices[i], 0.0f));
	}


	originalVertexPos.resize(8);
	std::copy(coliderVertices.begin(), coliderVertices.end(), originalVertexPos.begin());

	coliderIndices.resize(12);

	coliderIndices = { 1,0,1,2,2,3,3,0,0,4,1,5,2,6,3,7,4,5,5,6,6,7,7,4 };

	satIndices.resize(3 * 6);
	//satIndices = { 0,4,5,1,5,6,6,2,3,3,7,4,2,1,0,6,5,4 };
	satIndices = { 1,0,2,4,5,6,5,6,1,4,0,7,5,4,1,7,6,3 };

	color = glm::vec4(255.0f, 255.0f, 255.0f, 1.0f);

	descSetData.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

	changeUniformBuffer = true;
}

glm::vec3* Colider::getColiderVertices()
{
	return coliderVertices.data();
}

glm::vec3* Colider::getColiderOriginalVertices()
{
	return originalVertexPos.data();
}

int Colider::getColiderVerticesSize()
{
	return originalVertexPos.size();
}

int* Colider::getColiderIndices()
{
	return (int*)coliderIndices.data();
}

int Colider::getColiderIndicesSize()
{
	return coliderIndices.size();
}

void Colider::reflectMovement(glm::mat4& transform)
{
	for (int i = 0; i < coliderVertices.size(); i++)
	{
		coliderVertices[i] = transform * glm::vec4(originalVertexPos[i], 1.0f);
	}

	pivot = glm::vec3(transform * glm::vec4(pivot, 1.0f));

	changeUniformBuffer = true;


	/*
	glm::vec3 min = glm::vec3(1000.0f, 1000.0f, 1000.0f);
	glm::vec3 max = glm::vec3(-1000.0f, -1000.0f, -1000.0f);
	for (int i = 0; i < getColiderVerticesSize(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (min[j] > coliderVertices[i][j])
			{
				min[j] = coliderVertices[i][j];
			}
		}
	}

	for (int i = 0; i < getColiderVerticesSize(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (max[j] < coliderVertices[i][j])
			{
				max[j] = coliderVertices[i][j];
			}
		}
	}

	aabb.minPos = min;
	aabb.maxPos = max;
	*/
}

bool Colider::Intersect(std::shared_ptr<Colider> oppColider, float& collisionDepth, glm::vec3& collisionVector)
{
	bool collision = false;

	//GJK(oppColider);

	//collision = AABB(oppColider);

	collision = SAT(oppColider, collisionDepth, collisionVector);

	return collision;
}

bool Colider::SAT(std::shared_ptr<Colider> oppColider, float& collisionDepth, glm::vec3& collisionNormal)
{
	std::array<glm::vec3, 12> normals;

	collisionDepth = 1000000.0f;
	collisionNormal = { 0.0f,0.0f,0.0f };

	glm::vec3* oppVertices = oppColider->getColiderVertices();

	for (int i = 0; i < 18; i += 3)
	{
		normals[i / 3] = glm::normalize(glm::cross(coliderVertices[satIndices[i + 1]] - coliderVertices[satIndices[i]], coliderVertices[satIndices[i + 2]] - coliderVertices[satIndices[i]]));
	}
	for (int i = 0; i < 18; i += 3)
	{
		normals[(i / 3) + 6] = glm::normalize(glm::cross(oppVertices[satIndices[i + 1]] - oppVertices[satIndices[i]], oppVertices[satIndices[i + 2]] - oppVertices[satIndices[i]]));
	}
	/*
	for (int i = 0; i < 12; i += 2)
	{
		for (int j = 0; j < 12; j += 2)
		{
			normals[i * 3 + (j / 2) + 12] = glm::normalize(glm::cross(coliderVertices[satIndices[i + 1]] - coliderVertices[satIndices[i]], oppVertices[satIndices[j + 1]] - oppVertices[satIndices[j]]));
		}
	}
	*/

	float depthMin = 0, depthMax = 0;

	float min, max, oppMin, oppMax;
	glm::vec3 minVertex, maxVertex, oppMinVertex, oppMaxVertex;
	for (int i = 0; i < 12; i++)
	{
		this->projection(min, max, minVertex, maxVertex, normals[i]);
		oppColider->projection(oppMin, oppMax, oppMinVertex, oppMaxVertex, normals[i]);

		if (!((min <= oppMin && oppMin <= max) || (oppMin <= min && min <= oppMax)))
		{
			return false;
		}
		else
		{
			float tmp = oppMin - max;

			if (abs(collisionDepth) > abs(tmp))
			{
				if (normals[i] != glm::vec3(0.0f))
				{
					depthMin = oppMin;
					depthMax = max;
					collisionDepth = oppMin - max;
					collisionNormal = normals[i];
				}
			}
		}
	}

	return true;
}

void Colider::projection(float& min, float& max, glm::vec3& minVertex, glm::vec3& maxVertex, glm::vec3& axis)
{
	min = glm::dot(this->coliderVertices[0], axis);
	minVertex = this->coliderVertices[0];
	max = min;

	float tmp;
	for (int i = 1; i < this->getColiderVerticesSize(); i++)
	{
		tmp = glm::dot(this->coliderVertices[i], axis);
		if (tmp > max)
		{
			max = tmp;
			maxVertex = this->coliderVertices[i];
		}
		else if (tmp < min)
		{
			min = tmp;
			minVertex = this->coliderVertices[i];
		}
	}
}

/*
bool Colider::GJK(std::shared_ptr<Colider> oppColider)
{
	glm::vec3 support = getSupportVector(oppColider, glm::vec3(1.0f, 0.0f, 0.0f));
	//std::cout << pivot << std::endl;
	//std::cout << "opp" << oppColider->getPivot() << std::endl;

	Simplex simplex;
	simplex.push_front(support);

	glm::vec3 dir = -support;

	int count = 0;

	while (count < 50)
	{
		// std::cout << coliderVertices[0] << std::endl;
		//std::cout << "OPP" << oppColider->getColiderVertices()[0] << std::endl;

		support = getSupportVector(oppColider, dir);

		//std::cout << glm::dot(support, dir) << std::endl;

		if (glm::dot(support, dir) <= 0.0f)
		{
			return false;
		}

		simplex.push_front(support);

		if (nextSimplex(simplex, dir))
		{
			return true;
		}

		count++;
	}

	return false;
}
*/

glm::vec3 Colider::getFurthestPoint(glm::vec3 dir)
{
	glm::vec3  maxPoint;
	float maxDistance = -10000.0f;

	for (int i = 0; i < getColiderVerticesSize(); i++)
	{
		float distance = glm::dot(coliderVertices[i], dir);
		if (distance > maxDistance) {
			maxDistance = distance;
			maxPoint = coliderVertices[i];
		}
	}

	return maxPoint;
}

glm::vec3 Colider::getSupportVector(std::shared_ptr<Colider> oppColider, glm::vec3 dir)
{
	return getFurthestPoint(dir) - oppColider->getFurthestPoint(-dir);
}

bool Colider::Line(Simplex& simplex, glm::vec3 dir)
{
	glm::vec3 a = simplex[0];
	glm::vec3 b = simplex[1];

	glm::vec3 ab = b - a;
	glm::vec3 ao = -a;

	if (sameDirection(ab, ao)) {
		dir = glm::cross(glm::cross(ab, ao), ab);
	}
	else
	{
		simplex = { a };
		dir = ao;
	}

	return false;
}

bool Colider::Triangle(Simplex& simplex, glm::vec3 dir)
{
	glm::vec3 a = simplex[0];
	glm::vec3 b = simplex[1];
	glm::vec3 c = simplex[2];

	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	glm::vec3 point = -a;

	glm::vec3 triangleCross = glm::cross(ab, ac);

	if (sameDirection(glm::cross(triangleCross, ac), point))
	{
		if (sameDirection(ac, point))
		{
			simplex = { a,c };
			dir = glm::cross(glm::cross(ac, point), ac);
		}
		else
		{

			simplex = { a,b };
			return Line(simplex, dir);
		}
	}
	else
	{

		if (sameDirection(glm::cross(ab, triangleCross), point))
		{
			simplex = { a,b };
			return Line(simplex, dir);
		}
		else
		{
			if (sameDirection(triangleCross, point))
			{
				dir = triangleCross;
			}
			else
			{
				simplex = { a,c,b };
				dir = -triangleCross;
			}
		}
	}

	return false;
}

bool Colider::Tetrahedron(Simplex& simplex, glm::vec3 dir)
{
	glm::vec3 a = simplex[0];
	glm::vec3 b = simplex[1];
	glm::vec3 c = simplex[2];
	glm::vec3 d = simplex[3];

	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	glm::vec3 ad = d - a;
	glm::vec3 point = -a;

	glm::vec3 abc = glm::cross(ab, ac);
	glm::vec3 acd = glm::cross(ac, ad);
	glm::vec3 adb = glm::cross(ad, ab);

	if (sameDirection(abc, point))
	{
		simplex = { a,b,c };
		return Triangle(simplex, dir);
	}

	if (sameDirection(acd, point))
	{
		simplex = { a,c,d };
		return Triangle(simplex, dir);
	}

	if (sameDirection(adb, point))
	{
		simplex = { a,d,b };
		return Triangle(simplex, dir);
	}

	return true;
}

bool Colider::nextSimplex(Simplex& simplex, glm::vec3 dir)
{
	switch (simplex.size) {
	case 2: return Line(simplex, dir);
	case 3: return Triangle(simplex, dir);
	case 4: return Tetrahedron(simplex, dir);
	}

	return false;
}

void Colider::setDescriptorSet(VkDescriptorSet descriptorSet)
{
	descSetData.descriptorSet = descriptorSet;
}

BufferObject* Colider::getPointBuffer()
{
	return &pointBuffer;
}

MappedBuffer* Colider::getMappedBuffer()
{
	return &mappedBuffer;
}

DescriptorInfo& Colider::getDescInfo()
{
	return descInfo;
}

void Colider::cleanupVulkan()
{
	VkDevice device = VulkanBase::GetInstance()->GetDevice();

	vkDestroyBuffer(device, pointBuffer.vertBuffer, nullptr);
	vkFreeMemory(device, pointBuffer.vertHandler, nullptr);

	vkDestroyBuffer(device, pointBuffer.indeBuffer, nullptr);
	vkFreeMemory(device, pointBuffer.indeHandler, nullptr);

	vkDestroyBuffer(device, mappedBuffer.uniformBuffer, nullptr);
	vkFreeMemory(device, mappedBuffer.uniformBufferMemory, nullptr);
	mappedBuffer.uniformBufferMapped = nullptr;
}

glm::vec3 Colider::getClosestLineToVertex(glm::vec3 lineStart, glm::vec3 lineFinish, glm::vec3 point)
{
	glm::vec3 lineVector = glm::normalize(lineFinish - lineStart);

	float dot = glm::dot(point, lineVector);

	return lineStart + dot * lineVector;
}