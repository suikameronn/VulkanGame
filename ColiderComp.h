#pragma once

//コライダーの種類
enum class ColliderType
{
	Box
};

struct ColiderComp
{
	//コライダーのID
	size_t ID;

	//コライダーのタイプ
	ColliderType type;

	//コライダーのサイズや半径などのパラメータ
	//現在はボックスのみ
	//この値は、3Dモデルのスケールにさらに、掛け合わされる
	//実際の3Dモデルよりもコライダーのみ大きくしたいときよう
	float size;

	ColiderComp()
	{
		ID = 0;
		type = ColliderType::Box;
		size = 1.0f;
	};
};