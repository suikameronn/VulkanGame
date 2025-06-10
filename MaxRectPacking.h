#pragma once

#include<iostream>
#include<vector>

struct Rect
{
	float x;
	float y;
	float width;
	float height;

	//回転させたかどうか
	bool rotated;

	Rect()
	{
		x = 0.0f;
		y = 0.0f;
		width = 0.0f;
		height = 0.0f;

		rotated = false;
	}

	//引数のrectがこのrectを完全に含んでいるかどうか
	bool isContain(const Rect& rect) const
	{
		return x >= rect.x && y >= rect.y
			&& x + width <= rect.x + rect.width
			&& y + height <= rect.y + rect.height;
	}
};

class MaxRectPacking
{
private:

	//ビンの幅と高さ
	float binWidth;
	float binHeight;

	//最新の新しい空き領域の数
	size_t newFreeRectLastSize;
	//最新の空き領域
	std::vector<Rect> newFreeRects;

	//使用済みの領域
	std::vector<Rect> usedRects;
	//最新ではない空き領域
	std::vector<Rect> freeRects;

	//Rectをビンに配置する
	void placeRect(const Rect& node);

	//面積を考慮した現在の空き領域の中で最適な領域を選択する
	Rect findPositionNewBAF(float width, float height
		, float& bestAreaFit, float& bestShortSideFit);

	//新しい空き矩形を空きリストに入れる
	void insertNewFreeRect(const Rect& rect);

	//指定された空きノードと使用済みノードから空きノードを分割する
	//分割された場合trueを返す
	bool splitFreeNode(const Rect& freeNode, const Rect& usedNode);

	//空き矩形リストを調べて、冗長なノードを削除する
	void pruneFreeList();

	MaxRectPacking() {};

public:

	MaxRectPacking(float width, float height);

	~MaxRectPacking() {};

	//ビンを挿入する
	Rect insert(bool& packingSuccess,float width, float height);
};