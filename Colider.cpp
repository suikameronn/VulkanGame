#include"Colider.h"

#include"VulkanBase.h"

Colider::Colider(glm::vec3 pos, float right, float left, float top, float bottom, float front, float back)
{
	coliderVertices.resize(8);

	coliderVertices[0] = glm::vec3(pos.x + right,pos.y + top,pos.z + front);
	coliderVertices[1] = glm::vec3(pos.x - left,pos.y + top,pos.z + front);
	coliderVertices[2] = glm::vec3(pos.x - left, pos.y + top, pos.z - back);
	coliderVertices[3] = glm::vec3(pos.x + right, pos.y + top, pos.z - back);
	coliderVertices[4] = glm::vec3(pos.x + right, pos.y - bottom, pos.z + front);
	coliderVertices[5] = glm::vec3(pos.x - left, pos.y - bottom, pos.z + front);
	coliderVertices[6] = glm::vec3(pos.x - left, pos.y - bottom, pos.z - back);
	coliderVertices[7] = glm::vec3(pos.x + right, pos.y - bottom, pos.z - back);

	pivot = glm::vec3(((pos.x + right) + (pos.x - left)) / 2, ((pos.y + top) + (pos.y - bottom)) / 2, ((pos.z + front) + (pos.z - back)) / 2);

	coliderIndices.resize(12);

	coliderIndices = { 0,1,1,2,2,3,3,0,0,4,1,5,2,6,3,7,4,5,5,6,6,7,7,4 };

	color = glm::vec4(255.0f, 255.0f, 255.0f, 1.0f);

	descSetData.texCount = 0;
	descSetData.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
}

Colider::Colider(glm::vec3 pos, glm::vec3 min, glm::vec3 max)
{
	coliderVertices.resize(8);

	coliderVertices[0] = glm::vec3(pos.x + max.x, pos.y + max.y, pos.z + max.z);
	coliderVertices[1] = glm::vec3(pos.x + min.x, pos.y + max.y, pos.z + max.z);
	coliderVertices[2] = glm::vec3(pos.x + min.x, pos.y + max.y, pos.z + min.z);
	coliderVertices[3] = glm::vec3(pos.x + max.x, pos.y + max.y, pos.z + min.z);
	coliderVertices[4] = glm::vec3(pos.x + max.x, pos.y + min.y, pos.z + max.z);
	coliderVertices[5] = glm::vec3(pos.x + min.x, pos.y + min.y, pos.z + max.z);
	coliderVertices[6] = glm::vec3(pos.x + min.x, pos.y + min.y, pos.z + min.z);
	coliderVertices[7] = glm::vec3(pos.x + max.x, pos.y + min.y, pos.z + min.z);
	
	pivot = glm::vec3(((pos.x + max.x) + (pos.x + min.x)) / 2, ((pos.y + max.y) + (pos.y + min.y)) / 2, ((pos.z + max.z) + (pos.z + min.z)) / 2);

	coliderIndices.resize(12);

	coliderIndices = { 0,1,1,2,2,3,3,0,0,4,1,5,2,6,3,7,4,5,5,6,6,7,7,4 };

	color = glm::vec4(255.0f, 255.0f, 255.0f, 1.0f);

	descSetData.texCount = 0;
	descSetData.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
}

glm::vec3* Colider::getColiderVertices()
{
	return coliderVertices.data();
}

int Colider::getColiderVerticesSize()
{
	return coliderVertices.size();
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
	for (auto itr = coliderVertices.begin(); itr != coliderVertices.end(); itr++)
	{
		*itr = glm::vec3(glm::vec4(*itr,1.0f) * transform);
	}

	pivot = glm::vec3(glm::vec4(pivot, 1.0f) * transform);
}

bool Colider::Intersect(std::shared_ptr<Colider> oppColider)
{
	glm::vec3 support = getSupportVector(oppColider, glm::vec3(1.0,0.0,0.0));

	Simplex simplex;
	simplex.push_front(support);

	glm::vec3 dir = -support;

	while (true)
	{
		support = getSupportVector(oppColider, dir);

		if (glm::dot(support, dir) <= 0.0f)
		{
			return false;
		}

		simplex.push_front(support);

		if (nextSimplex(simplex, dir))
		{
			return true;
		}
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

glm::vec3 Colider::getFurthestPoint(glm::vec3 dir)
{
	glm::vec3  maxPoint;
	float maxDistance = -1000000000.0f;

	for (int i = 0; i < getColiderVerticesSize();i++) 
	{
		float distance = glm::dot(coliderVertices[i], dir);
		if (distance > maxDistance) {
			maxDistance = distance;
			maxPoint = coliderVertices[i];
		}
	}

	return maxPoint;
}

glm::vec3 Colider::getSupportVector(std::shared_ptr<Colider> oppColider,glm::vec3 dir)
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

	glm::vec3 triangleCross = glm::cross(ab,ac);
	
	if (sameDirection(glm::cross(triangleCross, ac), point))
	{
		if (sameDirection(ac, point))
		{
			simplex = { a,c };
			dir = glm::cross(glm::cross(ac, point), ac);
		}
		else
		{
			return Line(simplex = { a,b }, dir);
		}
	}
	else
	{
		if (sameDirection(glm::cross(ab, triangleCross), point))
		{
			return Line(simplex = { a,b }, dir);
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
		return Triangle(simplex = { a,b,c }, dir);
	}

	if (sameDirection(acd, point))
	{
		return Triangle(simplex = { a,c,d }, dir);
	}

	if (sameDirection(adb, point))
	{
		return Triangle(simplex = { a,d,b }, dir);
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

glm::vec3 Colider::getClosestLineToVertex(glm::vec3 lineStart,glm::vec3 lineFinish,glm::vec3 point)
{
	glm::vec3 lineVector = glm::normalize(lineFinish - lineStart);

	float dot = glm::dot(point, lineVector);

	return lineStart + dot * lineVector;
}