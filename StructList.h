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