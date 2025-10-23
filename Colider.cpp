#include"Colider.h"

Colider::Colider(std::shared_ptr<GltfModel> gltfModel,bool isTrigger
	,std::shared_ptr<GpuBufferFactory> buffer,std::shared_ptr<DescriptorSetLayoutFactory> layout
	,std::shared_ptr<DescriptorSetFactory> desc)
{
	bufferFactory = buffer;
	layoutFactory = layout;
	descriptorSetFactory = desc;

	trigger = isTrigger;

	this->min = gltfModel->initPoseMin;
	this->max = gltfModel->initPoseMax;

	this->transformedMin = min;
	this->transformedMax = max;

	scale = glm::vec3(1.0f);
	scaleMat = glm::scale(scale);

	originalVertexPos.resize(8);//���W�ϊ��O�̍��W�z��

	originalVertexPos[0] = glm::vec3(min.x, min.y, min.z);
	originalVertexPos[1] = glm::vec3(max.x, min.y, min.z);
	originalVertexPos[2] = glm::vec3(max.x, min.y, max.z);
	originalVertexPos[3] = glm::vec3(min.x, min.y, max.z);
	originalVertexPos[4] = glm::vec3(min.x, max.y, min.z);
	originalVertexPos[5] = glm::vec3(max.x, max.y, min.z);
	originalVertexPos[6] = glm::vec3(max.x, max.y, max.z);
	originalVertexPos[7] = glm::vec3(min.x, max.y, max.z);

	coliderVertices.resize(8);

	coliderIndices =
	{
		//����
		0,1,3,
		1,2,3,
		//���
		4,7,5,
		7,6,5,
		//�O��
		0,4,1,
		4,5,1,
		//�w��
		3,2,7,
		2,6,7,
		//����
		0,3,4,
		3,7,4,
		//�E��
		1,5,2,
		5,6,2
	};

	drawColiderIndices =
	{
		0, 1,
		1, 3,
		3, 0,
		1, 2,
		2, 3,
		3, 1,
		4, 7,
		7, 5,
		5, 4,
		7, 6,
		6, 5,
		5, 7,
		0, 4,
		4, 1,
		1, 0,
		4, 5,
		5, 1,
		1, 4,
		3, 2,
		2, 7,
		7, 3,
		2, 6,
		6, 7,
		7, 2,
		0, 3,
		3, 4,
		4, 0,
		3, 7,
		7, 4,
		4, 3,
		1, 5,
		5, 2,
		2, 1,
		5, 6,
		6, 2,
		2, 5,
		1,6,
		6,2,
		2,7,
		3,7,
		0,7
	};

	satIndices.resize(3 * 6);
	satIndices = { 0,4,5,1,5,6,6,2,3,3,7,4,2,1,0,6,5,4 };
	satIndices = { 1,0,2,4,5,6,5,6,1,4,0,7,5,4,1,7,6,3 };//�Փ˔���p�̃C���f�b�N�X�z��

	offsetPos = glm::vec3(0.0f);
	offsetScale = glm::vec3(0.0f);
}

//�`��ɕK�v�ȃ��\�[�X���쐬����
void Colider::createBuffer()
{
	vertBuffer = bufferFactory->Create(originalVertexPos.size() * sizeof(glm::vec3), originalVertexPos.data()
		, BufferUsage::VERTEX, BufferTransferType::DST);

	indeBuffer = bufferFactory->Create(coliderIndices.size() * sizeof(uint32_t), coliderIndices.data()
		, BufferUsage::INDEX, BufferTransferType::DST);

	matBuffer = bufferFactory->Create(sizeof(ModelMat)
		, BufferUsage::UNIFORM, BufferTransferType::NONE);
}

void Colider::createDescriptorSet()
{
	const std::shared_ptr<DescriptorSetLayout> layout =
		layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT);

	descriptorSet = descriptorSetFactory->Create(descriptorSetFactory->getBuilder()
		->initProperty()
		->withBindingBuffer(0)
		->withBuffer(matBuffer)
		->withDescriptorSetCount(1)
		->withDescriptorSetLayout(layout)
		->withRange(sizeof(ModelMat))
		->withTypeBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		->addBufferInfo()
		->Build()
	);
}

//�R���C�_�[�̊e�I�t�Z�b�g��ݒ肷��
void Colider::setOffsetPos(const glm::vec3& pos)
{
	offsetPos = pos;
}

void Colider::setOffsetScale(const glm::vec3& scale)
{
	offsetScale = scale;
}

//Model�N���X�̏������W������W�ϊ���K�p����
void Colider::initFrameSettings(glm::vec3 initScale)
{
	scaleMat = glm::scale(initScale);

	for (int i = 0; i < originalVertexPos.size(); i++)
	{
		originalVertexPos[i] = scaleMat * glm::vec4(originalVertexPos[i], 1.0);
	}

	coliderVertices.resize(originalVertexPos.size());
	std::copy(originalVertexPos.begin(), originalVertexPos.end(), coliderVertices.begin());
}

//���W�ϊ����������R���C�_�[�̒��_���擾
glm::vec3* Colider::getColiderVertices()
{
	return coliderVertices.data();
}

//�R���C�_�[�̒��_���擾
glm::vec3* Colider::getColiderOriginalVertices()
{
	return originalVertexPos.data();
}

//�R���C�_�[�̒��_���擾
int Colider::getColiderVerticesSize()
{
	return static_cast<int>(originalVertexPos.size());
}

//�R���C�_�[�̒��_�̃C���f�b�N�X���擾
int* Colider::getColiderIndices()
{
	return (int*)coliderIndices.data();
}

//�R���C�_�[�̒��_�̃C���f�b�N�X�̃T�C�Y���擾
int Colider::getColiderIndicesSize()
{
	return static_cast<int>(coliderIndices.size());
}

//�R���C�_�[�̃X�P�[���s����擾
glm::mat4 Colider::getScaleMat()
{
	return scaleMat;
}

//Model�N���X�̈ړ��Ȃǂ��R���C�_�[�ɂ����f
void Colider::reflectMovement(const glm::vec3& translate, const glm::mat4& rotate, const glm::vec3& scale)
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), translate)
		* rotate * glm::scale(glm::mat4(1.0f), scale);

	const glm::vec3 rotateOffset = rotate * glm::scale(glm::mat4(1.0f), scale) * glm::vec4(offsetPos, 1.0f);

	for (int i = 0; i < coliderVertices.size(); i++)
	{
		coliderVertices[i] = transform * glm::vec4(originalVertexPos[i], 1.0f);
		coliderVertices[i] += rotateOffset;
	}

	transformedMin = glm::vec3(transform * glm::vec4(min, 1.0f)) + rotateOffset;
	transformedMax = glm::vec3(transform * glm::vec4(max, 1.0f)) + rotateOffset;

	ModelMat model{};
	model.scale = glm::vec3(1.0f);
	model.matrix = transform;
	model.offset = rotateOffset;
	memcpy(matBuffer->mappedPtr, &model, sizeof(ModelMat));
}

//�d�S��Ԃ�
glm::vec3 Colider::getCenterPos()
{
	glm::vec3 centerPos = transformedMin + transformedMax;

	centerPos /= 2.0f;

	return centerPos;
}

//SAT�p�����蔻��̎��s
bool Colider::Intersect(const std::unique_ptr<Colider>& oppColider, glm::vec3& collisionVector, glm::vec3& myCollisionPoint, glm::vec3& oppCollisionPoint
	, const TransformComp& myTransform, const TransformComp& oppTransform)
{
	bool collision = false;

	float depth = 0.0f;

	collision = SAT(oppColider, collisionVector, myCollisionPoint, oppCollisionPoint
		, myTransform, oppTransform);

	return collision;
}

//GJK�p�����蔻��̎��s
bool Colider::Intersect(const std::unique_ptr<Colider>& oppColider)
{
	bool collision = false;

	glm::vec3 collisionVector;
	glm::vec3 myCollisionPoint;
	glm::vec3 oppCollisionPoint;

	collision = GJK(oppColider, collisionVector, myCollisionPoint, oppCollisionPoint);

	return collision;
}

//�{�b�N�X���C�L���X�g�p�̓����蔻��̎��s
bool Colider::Intersect(glm::vec3 origin, glm::vec3 dir, float length,glm::vec3& normal)
{
	const float epsilon = 0.0001f;

	for (int i = 0; i < coliderIndices.size(); i += 3)
	{
		glm::vec3 e1 = coliderVertices[coliderIndices[i + 1]] - coliderVertices[coliderIndices[i]];
		glm::vec3 e2 = coliderVertices[coliderIndices[i + 2]] - coliderVertices[coliderIndices[i]];

		glm::vec3 h = glm::cross(dir, e2);

		float dot = glm::dot(h, e1);
		if (dot >= -epsilon && dot <= epsilon)
		{//�����͎O�p�`�ƕ��s
			continue;
		}

		float inv = 1.0f / dot;
		glm::vec3 v = origin - coliderVertices[coliderIndices[i]];
		float u = inv * glm::dot(h, v);

		if (u < 0.0f || u > 1.0f)
		{
			continue;
		}

		glm::vec3 q = glm::cross(v, e1);
		float f = inv * glm::dot(dir, q);
		if (f < 0.0f || u + f > 1.0f)
		{
			continue;
		}

		float t = inv * glm::dot(e2, q);

		if (t > -epsilon && t <= length)
		{
			normal = glm::normalize(glm::cross(e1, e2));

			return true;
		}
	}

	return false;
}

int Colider::intersectRectQuad2(float* h, float* p, float* ret)
{
	// q (and r) contain nq (and nr) coordinate points for the current (and
	// chopped) polygons
	int nq = 4, nr = 0;
	float buffer[16];
	float* q = p;
	float* r = ret;
	for (int dir = 0; dir <= 1; dir++)
	{
		// direction notation: xy[0] = x axis, xy[1] = y axis
		for (int sign = -1; sign <= 1; sign += 2)
		{
			// chop q along the line xy[dir] = sign*h[dir]
			float* pq = q;
			float* pr = r;
			nr = 0;
			for (int i = nq; i > 0; i--)
			{
				// go through all points in q and all lines between adjacent points
				if (sign * pq[dir] < h[dir])
				{
					// this point is inside the chopping line
					pr[0] = pq[0];
					pr[1] = pq[1];
					pr += 2;
					nr++;
					if (nr & 8)
					{
						q = r;
						goto done;
					}
				}
				float* nextq = (i > 1) ? pq + 2 : q;
				if ((sign * pq[dir] < h[dir]) ^ (sign * nextq[dir] < h[dir]))
				{
					// this line crosses the chopping line
					pr[1 - dir] = pq[1 - dir] + (nextq[1 - dir] - pq[1 - dir]) /
													(nextq[dir] - pq[dir]) * (sign * h[dir] - pq[dir]);
					pr[dir] = sign * h[dir];
					pr += 2;
					nr++;
					if (nr & 8)
					{
						q = r;
						goto done;
					}
				}
				pq += 2;
			}
			q = r;
			r = (q == ret) ? buffer : ret;
			nq = nr;
		}
	}
done:
	if (q != ret) memcpy(ret, q, nr * 2 * sizeof(float));
	return nr;
}

//�������藝�𗘗p���������蔻������s�A�Փ˂��������邽�߂̃x�N�g�����v�Z
bool Colider::SAT(const std::unique_ptr<Colider>& oppColider, float& collisionDepth, glm::vec3& collisionNormal)
{
	std::array<glm::vec3, 12> normals;

	collisionDepth = FLT_MAX;
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

bool Colider::SAT(const std::unique_ptr<Colider>& oppColider, glm::vec3& collisionNormal, glm::vec3& myCollisionPoint, glm::vec3& oppCollisionPoint
	,const TransformComp& myTransform,const TransformComp& oppTransform)
{
	const float fudgeFactor = 1.05f;
	glm::vec3 p, pp, normalC(0.0f);
	glm::vec3 normalR(0.0f);

	const glm::mat3 R1 = glm::mat3(myTransform.rotate);
	const glm::mat3 R2 = glm::mat3(oppTransform.rotate);

	glm::vec3 A, B;
	float R11, R12, R13;
	float R21, R22, R23;
	float R31, R32, R33;
	float Q11, Q12, Q13;
	float Q21, Q22, Q23;
	float Q31, Q32, Q33;
	float s, s2, l;
	int i, j, invertNormal, code;

	//���ꂼ��̃R���C�_�[�̒��S�_���v�Z
	glm::vec3 centerA = getCenterPos();
	glm::vec3 centerB = (oppColider->getCenterPos());

	//���ꂼ��̃R���C�_�[�̒��S�̍��̃x�N�g�����v�Z����
	p = centerB - centerA;
	//����ɂ��̃R���C�_�[�̉�]�s��ō��W�ϊ���������
	pp = glm::mat3(R1) * p;

	//�R���C�_�[�̊e�ӂ̒����̔������擾
	A[0] = glm::length(transformedMax.x - transformedMin.x) * 0.5f;
	A[1] = glm::length(transformedMax.y - transformedMin.y) * 0.5f;
	A[2] = glm::length(transformedMax.z - transformedMin.z) * 0.5f;
	B[0] = glm::length(oppColider->transformedMax.x - oppColider->transformedMin.x) * 0.5f;
	B[1] = glm::length(oppColider->transformedMax.y - oppColider->transformedMin.y) * 0.5f;
	B[2] = glm::length(oppColider->transformedMax.z - oppColider->transformedMin.z) * 0.5f;

	//Rij��R1'*R2�A�܂�R1��R2�̊Ԃ̑��Ή�]�ł���B
	//R2��R1�̍��W�n�𒆐S�Ƃ�����]�s��ɕϊ����Ă���

	R11 = glm::dot(R1[0], R2[0]); R12 = glm::dot(R1[0], R2[1]); R13 = glm::dot(R1[0], R2[2]);
	R21 = glm::dot(R1[1], R2[0]); R22 = glm::dot(R1[1], R2[1]); R23 = glm::dot(R1[1], R2[2]);
	R31 = glm::dot(R1[2], R2[0]); R32 = glm::dot(R1[2], R2[1]); R33 = glm::dot(R1[2], R2[2]);

	Q11 = abs(R11); Q12 = abs(R12); Q13 = abs(R13);
	Q21 = abs(R21); Q22 = abs(R22); Q23 = abs(R23);
	Q31 = abs(R31); Q32 = abs(R32); Q33 = abs(R33);

	// 15�̉\�Ȃ��ׂĂ̕������ɂ���:
	// ? ?* ���̎����{�b�N�X�𕪗����邩�ǂ������m�F����B�����������Ă���΁A0��Ԃ��B
	// ? ?* ���̕������ɉ������ђʐ[�� (s2) �����߂�B
	// ? ?* �������ꂪ����܂ł̍ő�̐[���i�܂�A�ђʗʂ��ł��������Ƃ��j�ł���΁A������L�^����B
	// �@���x�N�g���́A�ł��������[�������������ɐݒ肳���B
	// ��: normalR �́A���ꂪ����܂ł̍ŏ��[���̖@���ł���ꍇ�AR1 �܂��� R2 �̗���w���悤�ɐݒ肳���B
	// ����ȊO�̏ꍇ�AnormalR �� 0 �ƂȂ�AnormalC �̓{�f�B1�ɑ΂��鑊�΃x�N�g���ɐݒ肳���B
	// invert_normal �́A�@���̕����𔽓]������ׂ��ꍇ�� 1 �ɂȂ�B

#define TST(expr1,expr2,norm,cc)	   \
	s2 = std::abs(expr1) - (expr2);    \
	if (s2 > 0) return 0;              \
	if (s2 > s)                        \
	{                                  \
		s = s2;                        \
		normalR = norm;                \
		invertNormal = ((expr1) < 0);  \
		code = (cc);                   \
	}

	s = -FLT_MAX;
	invertNormal = 0;
	code = 0;

	// separating axis = u1,u2,u3
	//�������̌������ꂼ�ꎎ���Ă���
	TST(pp[0], (A[0] + B[0] * Q11 + B[1] * Q12 + B[2] * Q13), R1[0], 1);
	TST(pp[1], (A[1] + B[0] * Q21 + B[1] * Q22 + B[2] * Q23), R1[1], 2);
	TST(pp[2], (A[2] + B[0] * Q31 + B[1] * Q32 + B[2] * Q33), R1[2], 3);

	// separating axis = v1,v2,v3
	//�������̌������ꂼ�ꎎ���Ă���
	TST(glm::dot(R2[0], p), (A[0] * Q11 + A[1] * Q21 + A[2] * Q31 + B[0]), R2[0], 4);
	TST(glm::dot(R2[1], p), (A[0] * Q12 + A[1] * Q22 + A[2] * Q32 + B[1]), R2[1], 5);
	TST(glm::dot(R2[2], p), (A[0] * Q13 + A[1] * Q23 + A[2] * Q33 + B[2]), R2[2], 6);

	// ����: s ���v�Z����ۂ́A�O�ώ����X�P�[�����O����K�v������܂��B
	// �@�� (n1,n2,n3) �̓{�b�N�X 1 ����Ƃ��܂��B
#undef TST
#define TST(expr1, expr2, n1, n2, n3, cc)                \
	s2 = std::abs(expr1) - (expr2);                        \
	if (s2 > FLT_EPSILON) return 0;                     \
	l = sqrt((n1) * (n1) + (n2) * (n2) + (n3) * (n3)); \
	if (l > FLT_EPSILON)                                \
	{                                                    \
		s2 /= l;                                         \
		if (s2 * fudgeFactor > s)                       \
		{                                                \
			s = s2;                                      \
			normalR = glm::vec3(0.0f);                                 \
			normalC[0] = (n1) / l;                       \
			normalC[1] = (n2) / l;                       \
			normalC[2] = (n3) / l;                       \
			invertNormal = ((expr1) < 0);               \
			code = (cc);                                 \
		}                                                \
	}

	float fudge2 = 1.0e-5f;

	Q11 += fudge2; Q12 += fudge2; Q13 += fudge2;
	Q21 += fudge2; Q22 += fudge2; Q23 += fudge2;
	Q31 += fudge2; Q32 += fudge2; Q33 += fudge2;

		// separating axis = u1 x (v1,v2,v3)
	TST(pp[2] * R21 - pp[1] * R31, (A[1] * Q31 + A[2] * Q21 + B[1] * Q13 + B[2] * Q12), 0, -R31, R21, 7);
	TST(pp[2] * R22 - pp[1] * R32, (A[1] * Q32 + A[2] * Q22 + B[0] * Q13 + B[2] * Q11), 0, -R32, R22, 8);
	TST(pp[2] * R23 - pp[1] * R33, (A[1] * Q33 + A[2] * Q23 + B[0] * Q12 + B[1] * Q11), 0, -R33, R23, 9);

	// separating axis = u2 x (v1,v2,v3)
	TST(pp[0] * R31 - pp[2] * R11, (A[0] * Q31 + A[2] * Q11 + B[1] * Q23 + B[2] * Q22), R31, 0, -R11, 10);
	TST(pp[0] * R32 - pp[2] * R12, (A[0] * Q32 + A[2] * Q12 + B[0] * Q23 + B[2] * Q21), R32, 0, -R12, 11);
	TST(pp[0] * R33 - pp[2] * R13, (A[0] * Q33 + A[2] * Q13 + B[0] * Q22 + B[1] * Q21), R33, 0, -R13, 12);

	// separating axis = u3 x (v1,v2,v3)
	TST(pp[1] * R11 - pp[0] * R21, (A[0] * Q21 + A[1] * Q11 + B[1] * Q33 + B[2] * Q32), -R21, R11, 0, 13);
	TST(pp[1] * R12 - pp[0] * R22, (A[0] * Q22 + A[1] * Q12 + B[0] * Q33 + B[2] * Q31), -R22, R12, 0, 14);
	TST(pp[1] * R13 - pp[0] * R23, (A[0] * Q23 + A[1] * Q13 + B[0] * Q32 + B[1] * Q31), -R23, R13, 0, 15);

#undef TST

	if (!code) return 0;

	glm::vec3 normal;
	float depth;

	// if we get to this point, the boxes interpenetrate. compute the normal
	// in global coordinates.
	// ���̓_�ɓ��B����ƁA�{�b�N�X�͑��݂Ɋђʂ��܂��B�@�����v�Z���܂��B
	// �O���[�o�����W�ŁB
	if (glm::length(normalR) > 0.0f)
	{
		normal[0] = normalR[0];
		normal[1] = normalR[1];
		normal[2] = normalR[2];
	}
	else
	{
		normal = R1 * normalC;
	}
	if (invertNormal)
	{
		normal[0] = -normal[0];
		normal[1] = -normal[1];
		normal[2] = -normal[2];
	}
	depth = -s;

	//�ڐG�_���v�Z���Ă���
	if (code > 6)
	{
		// an edge from box 1 touches an edge from box 2.
		// find a point pa on the intersecting edge of box 1
		// �{�b�N�X 1 �̃G�b�W���{�b�N�X 2 �̃G�b�W�ɐڂ��܂��B
		// �{�b�N�X 1 �̌����G�b�W��̓_ pa �������܂��B
		glm::vec3 pa;
		float sign;
		for (i = 0; i < 3; i++) pa[i] = centerA[i];
		for (j = 0; j < 3; j++)
		{
			sign = (glm::dot(normal,R1[j]) > 0) ? float(1.0) : float(-1.0);
			for (i = 0; i < 3; i++) pa[i] += sign * A[j] * R1[i][j];
		}

		// find a point pb on the intersecting edge of box 2
		glm::vec3 pb;
		for (i = 0; i < 3; i++) pb[i] = centerB[i];
		for (j = 0; j < 3; j++)
		{
			sign = (glm::dot(normal, R2[j]) > 0) ? float(-1.0) : float(1.0);
			for (i = 0; i < 3; i++) pb[i] += sign * B[j] * R2[i][j];
		}

		//�����܂łŁApa��pb�͂��ꂼ��̃R���C�_�[�̃G�b�W��̓_�������Ă���

		//��������G�b�W�ƃG�b�W�̍ŋߐړ_�����߂Ă���

		//ua,ub�͐����̕����x�N�g��
		float alpha, beta;
		glm::vec3 ua, ub;
		for (i = 0; i < 3; i++) ua[i] = R1[i][((code)-7) / 3];
		for (i = 0; i < 3; i++) ub[i] = R2[i][((code)-7) % 3];

		//�����ŁA���pa,pb�������̓_�Aua,ub�����ꂼ��̐����̕����x�N�g���Ƃ���
		//��̐����̍ŋߐړ_�����߂Ă���
		{
			glm::vec3 pEdge;
			pEdge[0] = pb[0] - pa[0];
			pEdge[1] = pb[1] - pa[1];
			pEdge[2] = pb[2] - pa[2];
			float uaub = glm::dot(ua, ub);
			float q1 = glm::dot(ua, pEdge);
			float q2 = -glm::dot(ub, pEdge);
			float d = 1 - uaub * uaub;
			if (d <= float(0.0001f))
			{
				// @@@ this needs to be made more robust
				alpha = 0;
				beta = 0;
			}
			else
			{
				d = 1.f / d;
				alpha = (q1 + uaub * q2) * d;
				beta = (uaub * q1 + q2) * d;
			}
		}

		//�����ŁA���ꂼ��̃R���C�_�[��̏Փ˓_�����߂Ă���
		for (i = 0; i < 3; i++) pa[i] += ua[i] * alpha;
		for (i = 0; i < 3; i++) pb[i] += ub[i] * beta;

		return true;
	}

	glm::mat3 Ra, Rb;
	glm::vec3 pa, pb, Sa, Sb;
	if (code <= 3)
	{
		Ra = R1;
		Rb = R2;
		pa = centerA;
		pb = centerB;
		Sa = A;
		Sb = B;
	}
	else
	{
		Ra = R2;
		Rb = R1;
		pa = centerB;
		pb = centerA;
		Sa = B;
		Sb = A;
	}

	// nr = normal vector of reference face dotted with axes of incident box.
	// anr = absolute values of nr.
	// nr = ���˃{�b�N�X�̎����_�݂���Q�Ɩʂ̖@���x�N�g���B
	// anr = nr �̐�Βl�B
	glm::vec3 normal2, nr, anr;
	if (code <= 3)
	{
		normal2[0] = normal[0];
		normal2[1] = normal[1];
		normal2[2] = normal[2];
	}
	else
	{
		normal2[0] = -normal[0];
		normal2[1] = -normal[1];
		normal2[2] = -normal[2];
	}

	//�@�������[�J�����W�n�ɕϊ�����
	nr = Rb * normal2;

	//�@���̐�Βl���Ƃ�
	//����́A�@�������˃{�b�N�X�̎��Ƃǂꂭ�炢�X���Ă��邩�𒲂ׂ邽��
	anr[0] = std::abs(nr[0]);
	anr[1] = std::abs(nr[1]);
	anr[2] = std::abs(nr[2]);

		// find the largest compontent of anr: this corresponds to the normal
	// for the indident face. the other axis numbers of the indicent face
	// are stored in a1,a2.

	// anr �̍ő听�������߂܂��B����́A���ږʂ̖@���ɑΉ����܂��B
	// ���ږʂ̑��̎��ԍ��́A
	// a1�Aa2 �Ɋi�[����܂��B
	int lanr, a1, a2;
	if (anr[1] > anr[0])
	{
		if (anr[1] > anr[2])
		{
			a1 = 0;
			lanr = 1;
			a2 = 2;
		}
		else
		{
			a1 = 0;
			a2 = 1;
			lanr = 2;
		}
	}
	else
	{
		if (anr[0] > anr[2])
		{
			lanr = 0;
			a1 = 1;
			a2 = 2;
		}
		else
		{
			a1 = 0;
			a2 = 1;
			lanr = 2;
		}
	}

	// compute center point of incident face, in reference-face coordinates
	//�Q�Ɩʂ̍��W�n�ɂ�������˖ʂ̒��S�_���v�Z����
	glm::vec3 center;
	if (nr[lanr] < 0)
	{
		for (i = 0; i < 3; i++) center[i] = pb[i] - pa[i] + Sb[lanr] * Rb[i][lanr];
	}
	else
	{
		for (i = 0; i < 3; i++) center[i] = pb[i] - pa[i] - Sb[lanr] * Rb[i][lanr];
	}

	// find the normal and non-normal axis numbers of the reference box
	// �Q�ƃ{�b�N�X�̖@�����Ɣ�@�����̔ԍ���������
	//�Q�Ɩʂ��͂�G�b�W�̃x�N�g�������߂�
	int codeN, code1, code2;
	if (code <= 3)
		codeN = code - 1;
	else
		codeN = code - 4;
	if (codeN == 0)
	{
		code1 = 1;
		code2 = 2;
	}
	else if (codeN == 1)
	{
		code1 = 0;
		code2 = 2;
	}
	else
	{
		code1 = 0;
		code2 = 1;
	}

	// find the four corners of the incident face, in reference-face coordinates
	// �Q�Ɩʍ��W�œ��˖ʂ�4����������
	float quad[8];  // 2D coordinate of incident face (x,y pairs) ���˖ʂ�2D���W�ix,y�y�A�j
	float c1, c2, m11, m12, m21, m22;

	//c1,c2�͓��˖ʂ̒��S�_�̎Q�Ɩʍ��W�n�ł̍��W
	c1 = glm::dot(center,Ra[code1]);
	c2 = glm::dot(center, Ra[code2]);
	// optimize this? - we have already computed this data above, but it is not
	// stored in an easy-to-index format. for now it's quicker just to recompute
	// the four dot products.
	// ������œK�����܂����H - ���̃f�[�^�͊��ɏ�Ōv�Z�ς݂ł����A
	// �C���f�b�N�X���₷���`���ŕۑ�����Ă��܂���B���̂Ƃ���́A4�̃h�b�g�ς��Čv�Z������������ł��B

	//mij�́A���˖ʂ̎����Q�Ɩʂ�2D���W�n�ɕϊ����邽�߂̍s��̐���
	m11 = glm::dot(Ra[code1], Rb[a1]);
	m12 = glm::dot(Ra[code1], Rb[a2]);
	m21 = glm::dot(Ra[code2], Rb[a1]);
	m22 = glm::dot(Ra[code2], Rb[a2]);
	{
		//�����ŁA���˖ʂ�4���̎Q�Ɩʍ��W�n�ł�2D���W�����߂Ă���

		float k1 = m11 * Sb[a1];
		float k2 = m21 * Sb[a1];
		float k3 = m12 * Sb[a2];
		float k4 = m22 * Sb[a2];
		quad[0] = c1 - k1 - k3;
		quad[1] = c2 - k2 - k4;
		quad[2] = c1 - k1 + k3;
		quad[3] = c2 - k2 + k4;
		quad[4] = c1 + k1 + k3;
		quad[5] = c2 + k2 + k4;
		quad[6] = c1 + k1 - k3;
		quad[7] = c2 + k2 - k4;
	}

	// find the size of the reference face
	// �Q�Ɩʂ̃T�C�Y��������
	float rect[2];
	rect[0] = Sa[code1];
	rect[1] = Sa[code2];

	// intersect the incident and reference faces
	// ���˖ʂƎQ�Ɩʂ�����������
	float ret[16];
	int n = intersectRectQuad2(rect, quad, ret);
	if (n < 1) return 0;  // this should never happen

	// convert the intersection points into reference-face coordinates,
	// and compute the contact position and depth for each point. only keep
	// those points that have a positive (penetrating) depth. delete points in
	// the 'ret' array as necessary so that 'point' and 'ret' correspond.
	// �����_���Q�Ɩʂ̍��W�ɕϊ����A
	// �e�_�̐ڐG�ʒu�Ɛ[�x���v�Z���܂��B���́i�ђʂ���j�[�x�����_�݂̂�ێ����܂��B
	// �K�v�ɉ����� 'ret' �z����̓_���폜���A 'point' �� 'ret' ���Ή�����悤�ɂ��܂��B
	glm::vec3 point[8];  // penetrating contact points �ђʐڐG�_
	float dep[8];        // depths for those points �����̃|�C���g�̐[��

	//2D�̌�_���G�b�W��̓��˖ʂ̃��[�J�����W�ɕϊ�����t�s��
	float det1 = 1.f / (m11 * m22 - m12 * m21);
	m11 *= det1;
	m12 *= det1;
	m21 *= det1;
	m22 *= det1;
	int cnum = 0;  // number of penetrating contact points found �������ꂽ�ђʐڐG�_�̐�
	for (j = 0; j < n; j++)
	{
		//���˖ʂ̃G�b�W��̃��[�J�����W�����߂�

		float k1 = m22 * (ret[j * 2] - c1) - m12 * (ret[j * 2 + 1] - c2);
		float k2 = -m21 * (ret[j * 2] - c1) + m11 * (ret[j * 2 + 1] - c2);

		//point�́A���[���h���W�n�ł̐ڐG�_�̈ʒu�������Ă���
		for (i = 0; i < 3; i++) point[cnum][i] =
			center[i] + k1 * Rb[i][a1] + k2 * Rb[i][a2];

		//�Q�Ɩʂ܂ł̋�������ڐG�_�܂ł̋����̍������߁A�ђʂ̐[���𒲂ׂĂ���
		dep[cnum] = Sa[codeN] - glm::dot(normal2, point[cnum]);
		if (dep[cnum] >= 0)
		{
			//�[�x��0�ȏ�̏ꍇ�̂ݐڐG�_�Ƃ��č̗p����

			ret[cnum * 2] = ret[j * 2];
			ret[cnum * 2 + 1] = ret[j * 2 + 1];
			cnum++;
		}
	}
	if (cnum < 1) return false;  // this should never happen �����͓{��Ȃ��͂�

	collisionNormal = glm::vec3(0.0f);
	if (glm::length(normal2) > 0.0f)
	{
		collisionNormal = glm::normalize(normal2) * dep[0];
	}

	glm::vec3 totalCollisionPoint(0.0f);
	for (int i = 0; i < cnum; i++)
	{
		totalCollisionPoint += point[i];
	}
	myCollisionPoint = (totalCollisionPoint / (float)cnum) + pa;

	return true;
}

//�T�|�[�g�ʑ������߂�(SAT�p)
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

//GJK�@�ł̓����蔻������s
bool Colider::GJK(const std::unique_ptr<Colider>& oppColider, glm::vec3& collisionDepthVec, glm::vec3& myCollisionPoint, glm::vec3& oppCollisionPoint)
{
	SimplexVertex support = getSupportVector(oppColider, glm::vec3(1.0f, 0.0f, 0.0f));//�K���ȕ����̃T�|�[�g�ʑ������߂�

	Simplex simplex;
	simplex.push_front(support);

	glm::vec3 dir = -support.point;

	int count = 50;

	while (count > 0)//�����񐔂ɐ�����݂���
	{
		support = getSupportVector(oppColider, dir);//�T�|�[�g�ʑ������߂�

		if (glm::dot(support.point, dir) <= 0.0f)//�������߂��T�|�[�g�ʑ������_�̕����ƌ������t��������A�����蔻����I��
		{
			return false;
		}

		simplex.push_front(support);//�x�N�g����ǉ�

		if (nextSimplex(simplex, dir))//�P�̂̍X�V�A�l�p�����Ɍ��_���܂܂�Ă�����AEPA�Ɉڍs
		{
			EPA(oppColider, simplex, collisionDepthVec, myCollisionPoint, oppCollisionPoint);//�Փ˂��������邽�߂̃x�N�g���̌v�Z
			return true;
		}

		count--;
	}

	return false;
}

//�����̕����x�N�g���̌����ōł��������_�����߂�
glm::vec3 Colider::getFurthestPoint(glm::vec3 dir)
{
	glm::vec3  maxPoint = glm::vec3(0.0f);
	float maxDistance = -FLT_MAX;

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

//�T�|�[�g�ʑ������߂�
SimplexVertex Colider::getSupportVector(const std::unique_ptr<Colider>& oppColider, glm::vec3 dir)
{
	SimplexVertex vertex{};
	
	vertex.myPoint = getFurthestPoint(dir);
	vertex.oppPoint = oppColider->getFurthestPoint(-dir);

	vertex.point = vertex.myPoint - vertex.oppPoint;

	return  vertex;
}

//�������m�ł킩��͈͂Ń~���R�t�X�L�[�������_���܂݂��������ׂ�
bool Colider::Line(Simplex& simplex, glm::vec3& dir)
{
	SimplexVertex a = simplex[0];
	SimplexVertex b = simplex[1];

	glm::vec3 ab = b.point - a.point;
	glm::vec3 ao = -a.point;//a0:���_�����̃x�N�g��

	if (sameDirection(ab, ao))//b�����_�����Ɉʒu���Ȃ�=�P�̂��O�p�`�ɂ��Ă����_���܂܂Ȃ�
	{		
		dir = glm::cross(glm::cross(ab, ao), ab);
	}
	else
	{
		simplex = { a };
		dir = ao;
	}

	return false;
}

//�O�p�`�ł킩��͈͂Ń~���R�t�X�L�[�������_���܂݂��������ׂ�
bool Colider::Triangle(Simplex& simplex, glm::vec3& dir)
{
	SimplexVertex a = simplex[0];
	SimplexVertex b = simplex[1];
	SimplexVertex c = simplex[2];

	glm::vec3 ab = b.point - a.point;
	glm::vec3 ac = c.point - a.point;
	glm::vec3 point = -a.point;

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

//�l�p���ł킩��͈͂Ń~���R�t�X�L�[�������_���܂ނ����ׂ�
bool Colider::Tetrahedron(Simplex& simplex, glm::vec3& dir)
{
	SimplexVertex a = simplex[0];
	SimplexVertex b = simplex[1];
	SimplexVertex c = simplex[2];
	SimplexVertex d = simplex[3];

	glm::vec3 ab = b.point - a.point;
	glm::vec3 ac = c.point - a.point;
	glm::vec3 ad = d.point - a.point;
	glm::vec3 point = -a.point;

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

//GJK�̎��̒P�̂����߂�
bool Colider::nextSimplex(Simplex& simplex, glm::vec3& dir)
{
	switch (simplex.size) {
	case 2: return Line(simplex, dir);
	case 3: return Triangle(simplex, dir);
	case 4: return Tetrahedron(simplex, dir);
	}

	return false;
}

//GJK�@���EPA�@�ŏՓ˂��������邽�߂̃x�N�g�����擾
void Colider::EPA(const std::unique_ptr<Colider>& oppColider,Simplex& simplex, glm::vec3& collisionDepthVec, glm::vec3& myCollisionPoint, glm::vec3& oppCollisionPoint)
{
	std::vector<SimplexVertex> polytope;
	simplex.setSimplexVertices(polytope);

	std::vector<int> faces =
	{
		0,1,2,
		0,3,1,
		0,2,3,
		1,3,2
	};

	auto[normals,minFace] = getFaceNormals(polytope, faces);

	glm::vec3 minNormal;
	float minDistance = FLT_MAX;
	int limit = 100;
	while (minDistance == FLT_MAX && limit > 0)
	{
		minNormal = normals[minFace];
		minDistance = normals[minFace].w;

		SimplexVertex support = getSupportVector(oppColider, minNormal);
		float sDistance = glm::dot(minNormal, support.point);

		if (abs(sDistance - minDistance) > 0.001f)
		{
			minDistance = FLT_MAX;
			std::vector<std::pair<int, int>> uniqueEdges;

			for (int i = 0; i < normals.size(); i++)
			{
				if (sameDirection(normals[i], support.point))
				{
					int f = i * 3;

					addIfUniqueEdge(uniqueEdges, faces, f, f + 1);
					addIfUniqueEdge(uniqueEdges, faces, f + 1, f + 2);
					addIfUniqueEdge(uniqueEdges, faces, f + 2, f);

					faces[f + 2] = faces.back();
					faces.pop_back();
					faces[f + 1] = faces.back();
					faces.pop_back();
					faces[f] = faces.back();
					faces.pop_back();

					normals[i] = normals.back();
					normals.pop_back();

					i--;
				}
			}

			if (uniqueEdges.size() == 0)
			{
				break;
			}

			std::vector<int> newFaces;
			for (int i = 0; i < uniqueEdges.size(); i++)
			{
				newFaces.push_back(uniqueEdges[i].first);
				newFaces.push_back(uniqueEdges[i].second);
				newFaces.push_back(static_cast<int>(polytope.size()));
			}
			polytope.push_back(support);

			auto[newNormals,newMinFace] = getFaceNormals(polytope, newFaces);

			float oldMinDistance = FLT_MAX;
			for (int i = 0; i < normals.size(); i++)
			{
				if (normals[i].w < oldMinDistance)
				{
					oldMinDistance = normals[i].w;
					minFace = i;
				}
			}
			
			if (newNormals[newMinFace].w < oldMinDistance)
			{
				minFace = newMinFace + static_cast<int>(normals.size());
			}

			faces.insert(faces.end(), newFaces.begin(), newFaces.end());
			normals.insert(normals.end(), newNormals.begin(), newNormals.end());
		}

		limit--;
	}

	collisionDepthVec = minNormal * minDistance;

	//��̃R���C�_�[�̏Փ˓_�����߂�
	std::array<SimplexVertex, 3> triangle{};
	for (int i = 0; i < 3; i++)
	{
		triangle[i] = polytope[faces[minFace * 3 + i]];
	}
	
	CollisionPoint(triangle, collisionDepthVec, myCollisionPoint, oppCollisionPoint);
}

//����̐������܂܂Ȃ���΂��̒��_��P�̂Ɋ܂߂�
void Colider::addIfUniqueEdge(
	std::vector<std::pair<int, int>>& edges,
	const std::vector<int>& faces,
	int a,
	int b)
{
	auto reverse = std::find(
		edges.begin(),
		edges.end(),
		std::make_pair(faces[b], faces[a])
	);

	if (reverse != edges.end()) {
		edges.erase(reverse);
	}

	else {
		edges.emplace_back(faces[a], faces[b]);
	}
}

//�ʂ̖@�����擾
std::pair<std::vector<glm::vec4>, int> Colider::getFaceNormals(
	std::vector<SimplexVertex>& vertices,
	std::vector<int>& faces)
{
	std::vector<glm::vec4> normals;
	int minTriangle = 0;
	float  minDistance = FLT_MAX;

	for (int i = 0; i < faces.size(); i += 3) {
		glm::vec3 a = vertices[faces[i]].point;
		glm::vec3 b = vertices[faces[i + 1]].point;
		glm::vec3 c = vertices[faces[i + 2]].point;

		glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
		float distance = glm::dot(normal, a);

		if (distance < 0) {
			normal *= -1;
			distance *= -1;
		}

		normals.emplace_back(normal, distance);

		if (distance < minDistance) {
			minTriangle = i / 3;
			minDistance = distance;
		}
	}

	return { normals, minTriangle };
}

glm::vec3 Colider::getClosestLineToVertex(glm::vec3 lineStart, glm::vec3 lineFinish, glm::vec3 point)
{
	glm::vec3 lineVector = glm::normalize(lineFinish - lineStart);

	float dot = glm::dot(point, lineVector);

	return lineStart + dot * lineVector;
}

//�Փ˓_���v�Z����
void Colider::CollisionPoint(const std::array<SimplexVertex, 3>& triangle, const glm::vec3& point, glm::vec3& myCollisionPoint, glm::vec3& oppCollisionPoint)
{
	std::array<glm::vec3, 3> simplexVertices;
	for (int i = 0; i < 3; i++)
	{
		simplexVertices[i] = triangle[i].point;
	}

	std::array<float, 3> centerCoord = CenterCoord(simplexVertices, point);

	myCollisionPoint = centerCoord[0] * triangle[0].myPoint + centerCoord[1] * triangle[1].myPoint + centerCoord[2] * triangle[2].myPoint;

	oppCollisionPoint = centerCoord[0] * triangle[0].oppPoint + centerCoord[1] * triangle[1].oppPoint + centerCoord[2] * triangle[2].oppPoint;

	myCollisionPoint = (myCollisionPoint + oppCollisionPoint) / 2.0f;
}

//�O�p�`��̒��_�̏d�S���W�����߂�
std::array<float, 3> Colider::CenterCoord(const std::array<glm::vec3, 3>& triangle, const glm::vec3& position)
{
	//�O�p�`�̖@��
	glm::vec3 faceNormal = glm::cross(triangle[1] - triangle[0], triangle[2] - triangle[0]);

	//�O�p�`��ɓ��e���ꂽ���_���v�Z����
	glm::vec3 origin = glm::vec3(0.0f) - glm::normalize(faceNormal) * (std::abs(glm::dot(-triangle[0], glm::normalize(faceNormal))));

	//�O�p�`�̑S�̖̂ʐς����߂�
	float totalArea = 0.5f * glm::length(faceNormal);

	//�d�S���W�����߂�
	std::array<float, 3> centerCoord{};

	centerCoord[0] = 0.5f * glm::length(glm::cross(triangle[2] - triangle[1], (origin - triangle[1])));
	centerCoord[1] = 0.5f * glm::length(glm::cross(triangle[0] - triangle[2], (origin - triangle[2])));
	centerCoord[2] = 0.5f * glm::length(glm::cross(triangle[1] - triangle[0], (origin - triangle[0])));

	for (int i = 0; i < 3; i++)
	{
		centerCoord[i] /= totalArea;

		if (centerCoord[i] < 0.0f)
		{
			std::cout << "centerCoord error" << std::endl;
		}
	}

	return centerCoord;
}