#include<iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include<glm/glm.hpp>
#include <glm/gtx/hash.hpp>

struct FragmentParam
{
	//�����x���㏑��
	float alphaness;

	FragmentParam()
	{
		//���蓾�Ȃ��l�ɂ���
		//���̒l���g�����g��Ȃ����̃t���O�ɂ���
		alphaness = -1.0f;
	}
};

//���_�f�[�^
struct Vertex {
	glm::vec3 pos;//���W
	alignas(16) glm::vec3 color;//���_�J���[
	alignas(16) glm::vec2 texCoord0;//uv���W
	glm::vec2 texCoord1;
	glm::vec3 normal;//�@��

	uint32_t index;
	glm::ivec4 boneID1;//�e���󂯂�W���C���g�̔ԍ�

	glm::vec4 weight1;//�W���C���g�̉e���x

	Vertex()
	{
		pos = glm::vec3(0.0);
		color = glm::vec3(0.0);
		texCoord0 = glm::vec2(0.0);
		texCoord1 = glm::vec2(0.0);
		normal = glm::vec3(0.0);

		index = 0;

		for (uint32_t i = 0; i < 4; i++)
		{
			boneID1[i] = 0;
			weight1[i] = 0.0f;
		}
	};

	//�W���C���g�Ɖe���x��ݒ�
	void addBoneData(uint32_t boneID, float weight)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (weight1[i] == 0.0f)
			{
				boneID1[i] = boneID;
				weight1[i] = weight;

				return;
			}
		}
	}
};