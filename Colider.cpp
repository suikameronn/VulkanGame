#include"Colider.h"

#include"VulkanBase.h"

Colider::Colider(glm::vec3 pos, float right, float left, float top, float bottom, float front, float back)
{
	coliderVertices.resize(8);

	/*
	coliderVertices[0] = glm::vec3(pos.x + right,pos.y + top,pos.z + front);
	coliderVertices[1] = glm::vec3(pos.x - left,pos.y + top,pos.z + front);
	coliderVertices[2] = glm::vec3(pos.x - left, pos.y + top, pos.z - back);
	coliderVertices[3] = glm::vec3(pos.x + right, pos.y + top, pos.z - back);
	coliderVertices[4] = glm::vec3(pos.x + right, pos.y - bottom, pos.z + front);
	coliderVertices[5] = glm::vec3(pos.x - left, pos.y - bottom, pos.z + front);
	coliderVertices[6] = glm::vec3(pos.x - left, pos.y - bottom, pos.z - back);
	coliderVertices[7] = glm::vec3(pos.x + right, pos.y - bottom, pos.z - back);
	*/

	coliderVertices[0] = glm::vec3( + right,  + top,  + front);
	coliderVertices[1] = glm::vec3( - left,  + top,  + front);
	coliderVertices[2] = glm::vec3( - left,  + top,  - back);
	coliderVertices[3] = glm::vec3( + right,  + top,  - back);
	coliderVertices[4] = glm::vec3( + right,  - bottom,  + front);
	coliderVertices[5] = glm::vec3( - left,  - bottom,  + front);
	coliderVertices[6] = glm::vec3( - left,  - bottom,  - back);
	coliderVertices[7] = glm::vec3( + right,  - bottom,  - back);

	originalVertexPos.resize(8);
	std::copy(coliderVertices.begin(), coliderVertices.end(), originalVertexPos.begin());

	for (auto itr = coliderVertices.begin(); itr != coliderVertices.end(); itr++)
	{
		pivot += *itr;
	}
	float size = coliderVertices.size();
	pivot = glm::vec3(pivot.x / size, pivot.y / size, pivot.z / size);

	coliderIndices.resize(12);

	coliderIndices = { 0,1,1,2,2,3,3,0,0,4,1,5,2,6,3,7,4,5,5,6,6,7,7,4 };

	color = glm::vec4(255.0f, 255.0f, 255.0f, 1.0f);

	descSetData.texCount = 0;
	descSetData.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

	changeUniformBuffer = true;
}

Colider::Colider(glm::vec3 pos, glm::vec3 min, glm::vec3 max)
{
	coliderVertices.resize(8);

	/*
	coliderVertices[0] = glm::vec3(pos.x + max.x, pos.y + max.y, pos.z + max.z);
	coliderVertices[1] = glm::vec3(pos.x + min.x, pos.y + max.y, pos.z + max.z);
	coliderVertices[2] = glm::vec3(pos.x + min.x, pos.y + max.y, pos.z + min.z);
	coliderVertices[3] = glm::vec3(pos.x + max.x, pos.y + max.y, pos.z + min.z);
	coliderVertices[4] = glm::vec3(pos.x + max.x, pos.y + min.y, pos.z + max.z);
	coliderVertices[5] = glm::vec3(pos.x + min.x, pos.y + min.y, pos.z + max.z);
	coliderVertices[6] = glm::vec3(pos.x + min.x, pos.y + min.y, pos.z + min.z);
	coliderVertices[7] = glm::vec3(pos.x + max.x, pos.y + min.y, pos.z + min.z);
	*/

	coliderVertices[0] = glm::vec3( + max.x,  + max.y,  + max.z);
	coliderVertices[1] = glm::vec3( + min.x,  + max.y,  + max.z);
	coliderVertices[2] = glm::vec3( + min.x,  + max.y,  + min.z);
	coliderVertices[3] = glm::vec3( + max.x,  + max.y,  + min.z);
	coliderVertices[4] = glm::vec3( + max.x,  + min.y,  + max.z);
	coliderVertices[5] = glm::vec3( + min.x,  + min.y,  + max.z);
	coliderVertices[6] = glm::vec3( + min.x,  + min.y,  + min.z);
	coliderVertices[7] = glm::vec3( + max.x,  + min.y,  + min.z);

	originalVertexPos.resize(8);
	std::copy(coliderVertices.begin(), coliderVertices.end(), originalVertexPos.begin());
	
	for (auto itr = coliderVertices.begin(); itr != coliderVertices.end(); itr++)
	{
		pivot += *itr;
	}
	float size = coliderVertices.size();
	pivot = glm::vec3(pivot.x / size, pivot.y / size, pivot.z / size);

	coliderIndices.resize(12);

	coliderIndices = { 0,1,1,2,2,3,3,0,0,4,1,5,2,6,3,7,4,5,5,6,6,7,7,4 };

	color = glm::vec4(255.0f, 255.0f, 255.0f, 1.0f);

	descSetData.texCount = 0;
	descSetData.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

	changeUniformBuffer = true;
}

glm::vec3* Colider::getColiderVertices()
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
		coliderVertices[i] = glm::vec3(transform * glm::vec4(originalVertexPos[i], 1.0f));
	}

	pivot = glm::vec3(transform * glm::vec4(pivot, 1.0f));

	this->transform = glm::mat4(1.0f);

	changeUniformBuffer = true;
}

bool Colider::Intersect(std::shared_ptr<Colider> oppColider)
{
	glm::vec3 support = getSupportVector(oppColider, pivot - oppColider->getPivot());
	//std::cout << pivot << std::endl;
	//std::cout << "opp" << oppColider->getPivot() << std::endl;

	Simplex simplex;
	simplex.push_front(support);

	glm::vec3 dir = -support;

	int count = 0;

	while (count < 50)
	{
		std::cout << coliderVertices[0] << std::endl;
		std::cout << "OPP" << oppColider->getColiderVertices()[0] << std::endl;

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

glm::vec3 Colider::getFurthestPoint(glm::vec3 dir)
{
	glm::vec3  maxPoint;
	float maxDistance = -1000000000.0f;

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
	descSetData.decriptorSet = descriptorSet;
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

glm::vec3 Colider::getClosestLineToVertex(glm::vec3 lineStart,glm::vec3 lineFinish,glm::vec3 point)
{
	glm::vec3 lineVector = glm::normalize(lineFinish - lineStart);

	float dot = glm::dot(point, lineVector);

	return lineStart + dot * lineVector;
}