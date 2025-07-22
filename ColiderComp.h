#pragma once

struct ColiderComp
{
	//コライダーの種類
	enum class ColliderType
	{
		Box
	};

	//コライダーのタイプ
	ColliderType type;

	//コライダーのサイズや半径などのパラメータ
	//現在はボックスのみ
	//この値は、3Dモデルのスケールにさらに、掛け合わされる
	//実際の3Dモデルよりもコライダーのみ大きくしたいときよう
	float size;

	ColiderComp()
	{
		type = ColliderType::Box;
		size = 1.0f;
	};
};