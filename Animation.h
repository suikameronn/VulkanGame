#pragma once

#include<array>
#include<vector>
#include<map>
#include<memory>
#include <assimp/scene.h>           // Output data structure
#include<glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

class Model;

struct AnimationKeyData
{
	std::map<float, glm::vec3> animationScaleKey;
	std::map<float, aiQuaternion> animationRotKey;
	std::map<float, glm::vec3> animationPositionKey;
};

class AnimNode
{
private:
	bool animNode;

	uint32_t childrenCount;
	std::vector<AnimNode*> children;

	std::string name;
	AnimationKeyData animKeyData;
	glm::mat4 matrix;

public:

	AnimNode(std::string nodeName,AnimationKeyData data, unsigned int cCount)
	{
		animNode = true;
		name = nodeName;
		animKeyData = data;
		children.resize(cCount);
		childrenCount = cCount;
	}

	AnimNode(std::string nodeName, glm::mat4 mat,unsigned int cCount)
	{
		animNode = false;
		name = nodeName;
		matrix = mat;
		childrenCount = cCount;
	}

	std::string getNodeName()
	{
		return name;
	}

	glm::mat4 aiMatrix3x3ToGlm(const aiMatrix3x3& from)
	{
		glm::mat4 to(1.0f);

		to[0][0] = from.a1; to[0][1] = from.b1;  to[0][2] = from.c1;
		to[1][0] = from.a2; to[1][1] = from.b2;  to[1][2] = from.c2;
		to[2][0] = from.a3; to[2][1] = from.b3;  to[2][2] = from.c3;

		return to;
	}

	void resizeChildren(int childNum)
	{
		children.resize(childNum);
	}

	void setChild(int i, AnimNode* child)
	{
		if (i < children.size())
		{
			children[i] = child;
		}
	}

	int getChildrenCount()
	{
		return children.size();
	}

	std::string getName()
	{
		return name;
	}

	AnimNode* getChildren(int i)
	{
		if (i < children.size())
		{
			return children[i];
		}

		return nullptr;
	}

	void findScale(float animTime,float& t1,float& t2)
	{
		auto end = animKeyData.animationScaleKey.end();
		end--;

		for (auto itr = animKeyData.animationScaleKey.begin(); itr != end; itr++)
		{
			auto next = itr;
			next++;

			if (animTime < next->first)
			{
				t1 = itr->first;
				t2 = next->first;
				return;
			}
		}

		t1 = -1.0f;
		t2 = -1.0f;
	}

	void findRotate(float animTime, float& t1, float& t2)
	{
		auto end = animKeyData.animationRotKey.end();
		end--;

		for (auto itr = animKeyData.animationRotKey.begin(); itr != end; itr++)
		{
			auto next = itr;
			next++;

			if (animTime < next->first)
			{
				t1 = itr->first;
				t2 = next->first;
				return;
			}
		}

		t1 = -1.0f;
		t2 = -1.0f;
	}

	void findPosition(float animTime, float& t1, float& t2)
	{
		auto end = animKeyData.animationPositionKey.end();
		end--;

		for (auto itr = animKeyData.animationPositionKey.begin(); itr != end; itr++)
		{
			auto next = itr;
			next++;

			if (animTime < next->first)
			{
				t1 = itr->first;
				t2 = next->first;
				return;
			}
		}

		t1 = -1.0f;
		t2 = -1.0f;
	}

	glm::vec3 calcInterpolatedScaling(float animTime)
	{
		if (animKeyData.animationScaleKey.size() == 1)
		{
			return animKeyData.animationScaleKey.begin()->second;
		}

		float t1, t2;

		findScale(animTime,t1,t2);//���݂̎����ɍł��߂��m�[�h��T��

		if (t1 >= 0.0f)
		{

			float DeltaTime = t2 - t1;//���Ԃ̍������擾
			float Factor = (animTime - t1) / DeltaTime;//���Ԃ̍������Ƃ�A���݂̎��Ԃƒ��O�̃m�[�h�̎��Ԃ��擾���āA���O�̃m�[�h�ƒ���̃m�[�h�̊Ԃ̂ǂꂭ�炢�̏ꏊ�ɂ���̂��𒲂ׂ�

			const glm::vec3 Start = animKeyData.animationScaleKey[t1];//�X�P�[�����O�̒l���Ƃ�
			const glm::vec3 End = animKeyData.animationScaleKey[t2];//�X�P�[�����O�̒l���Ƃ�
			glm::vec3 Delta = End - Start;//�����Ƃ�
			return Start + Factor * Delta;//�䗦����v�Z����
		}
		else//�ŏ��̃L�[�t���[�������O�̎���
		{

			float t2 = animKeyData.animationScaleKey.begin()->first;
			float Factor = 1 - (t2 - animTime) / t2;

			const glm::vec3 Start = glm::vec3(1.0, 1.0, 1.0);
			const glm::vec3 End = animKeyData.animationScaleKey.begin()->second;
			glm::vec3 Delta = End - Start;

			return Start + Factor * Delta;
		}
	}

	aiQuaternion calcInterpolatedQuat(float animTime)
	{
		// ��Ԃɂ͍Œ�ł��Q�̒l���K�v
		if (animKeyData.animationRotKey.size() == 1) {
			return animKeyData.animationRotKey.begin()->second;
		}

		float t1, t2;

		findRotate(animTime,t1,t2);
		
		if (t1 >= 0.0f)
		{
			float DeltaTime = t2 - t1;
			float Factor = (animTime - t1) / DeltaTime;

			const aiQuaternion StartRotationQ = animKeyData.animationRotKey[t1];
			const aiQuaternion EndRotationQ = animKeyData.animationRotKey[t2];

			aiQuaternion Out;
			aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
			Out = Out.Normalize();
			return Out;
		}
		else//�ŏ��̃L�[�t���[�������O�̎���
		{
			float t2 = animKeyData.animationRotKey.begin()->first;
			float Factor = 1 - (t2 - animTime) / t2;

			const aiQuaternion& StartRotationQ = aiQuaternion(1.0,0.0,0.0,0.0);
			const aiQuaternion& EndRotationQ = animKeyData.animationRotKey.begin()->second;
			aiQuaternion Out;
			aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
			Out = Out.Normalize();
			return Out;
		}
	}

	glm::vec3 calcInterpolatedPos(float animTime)
	{
		if (animKeyData.animationPositionKey.size() == 1)
		{
			return animKeyData.animationPositionKey.begin()->second;
		}

		float t1, t2;

		findPosition(animTime,t1,t2);//���݂̎����ɍł��߂��m�[�h��T��
		
		if (t1 >= 0.0f)
		{
			float DeltaTime = t2 - t1;//���Ԃ̍������擾
			float Factor = (animTime - t1) / DeltaTime;//���Ԃ̍������Ƃ�A���݂̎��Ԃƒ��O�̃m�[�h�̎��Ԃ��擾���āA���O�̃m�[�h�ƒ���̃m�[�h�̊Ԃ̂ǂꂭ�炢�̏ꏊ�ɂ���̂��𒲂ׂ�

			const glm::vec3 Start = animKeyData.animationPositionKey[t1];//�X�P�[�����O�̒l���Ƃ�
			const glm::vec3 End = animKeyData.animationPositionKey[t2];//�X�P�[�����O�̒l���Ƃ�
			glm::vec3 Delta = End - Start;//�����Ƃ�
			return Start + Factor * Delta;//�䗦����v�Z����
		}
		else//�ŏ��̃L�[�t���[�������O�̎���
		{
			float t2 = animKeyData.animationPositionKey.begin()->first;
			float Factor = 1 - (t2 - animTime) / t2;

			const glm::vec3 Start = glm::vec3(0.0, 0.0, 0.0);
			const glm::vec3 End = animKeyData.animationPositionKey.begin()->second;
			glm::vec3 Delta = End - Start;
			return Start + Factor * Delta;
		}
	}

	glm::mat4 getAnimMatrix(float animTime,glm::mat4& transform)
	{
		//��ԂȂǂ��v�Z���āA���s�ړ��Ɖ�]�A�X�P�[�����O�̍s������������s���Ԃ�
		if (animNode)
		{
			glm::vec3 scale = calcInterpolatedScaling(animTime);
			glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f),scale);

			aiQuaternion quat = calcInterpolatedQuat(animTime);
			glm::mat4 quatMat = aiMatrix3x3ToGlm(quat.GetMatrix());


			glm::vec3 position = calcInterpolatedPos(animTime);
			glm::mat4 posMat = glm::translate(glm::mat4(1.0f), position);

			return transform * posMat * quatMat * scaleMat;
		}
		else
		{
			return transform * matrix;
		}
	}
};

struct BoneInfo
{
	std::vector<glm::mat4> offsetMatrix;

	BoneInfo()
	{
		offsetMatrix.push_back(glm::mat4(1.0f));
	}

	void setOffsetMatrix(uint32_t index, glm::mat4 matrix)
	{
		if (index < offsetMatrix.size())
		{
			offsetMatrix[index] = matrix;
		}
	}

	void resizeBoneInfo(uint32_t size)
	{
		offsetMatrix.resize(size);
	}
};

class FbxModel;

class Pose
{
private:
	std::array<glm::mat4, 251> boneMatrix;

public:
	void setPoseMatrix(std::array<glm::mat4, 251>& matrix);

	void setFinalTransform(std::array<glm::mat4, 251>& boneFinalTransforms);
};

class Animation
{
private:

	float timeTick;
	float duration;

	AnimNode* rootNode;

	glm::mat4 inverseGlobalTransform;

public:

	Animation();
	Animation(float timeInTick,float duration);
	~Animation();
	void DeleteAnimTree(AnimNode* node);

	void setRootNode(AnimNode* rootNode)
	{
		this->rootNode = rootNode;
	}

	void setGlobalInverseTransform(glm::mat4 mat) { inverseGlobalTransform = mat; }
	void setFinalTransform(float animationTime, std::array<glm::mat4, 251>& boneFinalTransforms, FbxModel* model);
	void setFinalTransform(float animationTime, std::array<glm::mat4, 251>& boneFinalTransforms,AnimNode* node,glm::mat4 parentMatrix, FbxModel* model);
};