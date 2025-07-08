#include<iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include<glm/glm.hpp>
#include <glm/gtx/hash.hpp>

struct FragmentParam
{
	//透明度を上書き
	float alphaness;

	FragmentParam()
	{
		//あり得ない値にして
		//この値を使うか使わないかのフラグにする
		alphaness = -1.0f;
	}
};