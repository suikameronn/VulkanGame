#include"MaxRectPacking.h"

#include<assert.h>

MaxRectPacking::MaxRectPacking(float width, float height)
{
	binWidth = width;
	binHeight = height;

	Rect n{};
	n.x = 0;
	n.y = 0;
	n.width = width;
	n.height = height;

	usedRects.clear();

	freeRects.clear();
	freeRects.push_back(n);

	newFreeRectLastSize = 0;
}

void MaxRectPacking::placeRect(const Rect& node)
{
	//空いているすべての領域を調べる
	for (size_t i = 0; i < freeRects.size();)
	{
		if (splitFreeNode(freeRects[i], node))
		{
			//もし空き領域とnodeの領域が重なったら
			//そのノードを空きリストから消す
			freeRects[i] = freeRects.back();
			freeRects.pop_back();
		}
		else
		{
			++i;
		}
	}

	pruneFreeList();

	//使用済みノードにnodeを加える
	usedRects.push_back(node);
}

Rect MaxRectPacking::findPositionNewBAF(float width, float height
	, float& bestAreaFit, float& bestShortSideFit)
{
	Rect bestNode = {};

	//面積と辺の長さを最大値で初期化する
	bestAreaFit = FLT_MAX;
	bestShortSideFit = FLT_MAX;

	//すべての空き領域を探索する
	for (size_t i = 0; i < freeRects.size(); i++)
	{
		//空き領域とビンの面積の差を調べる
		int areaFit = freeRects[i].width * freeRects[i].height - width * height;

		//まずはビンを回転させずに配置してみる
		if (freeRects[i].width >= width && freeRects[i].height >= height)
		{
			//空き領域の幅と高さがビンよりも大きく、
			//空き領域にビンが入れられるなら

			//空き領域とビンの幅と高さの差を求める
			int leftOverHoriz = abs(freeRects[i].width - width);
			int leftOverVert = abs(freeRects[i].height - height);
			//次に、その差が小さいほうを調べる
			int shortSideFit = std::min(leftOverHoriz, leftOverVert);

			if (areaFit < bestAreaFit || (areaFit == bestAreaFit && shortSideFit < bestShortSideFit))
			{
				//bestAreaFitよりも面積の差が小さく
				//辺の長さがbestShortSideFitよりも小さかったら
				//ベストの空き領域のデータを更新する
				bestNode.x = freeRects[i].x;
				bestNode.y = freeRects[i].y;
				bestNode.width = width;
				bestNode.height = height;
				bestShortSideFit = shortSideFit;
				bestAreaFit = areaFit;
				bestNode.rotated = false;
			}
		}

		//今度はビンを回転させて、配置をしてみる
		if (freeRects[i].width >= height && freeRects[i].height >= width)
		{
			//空き領域の幅がノードの高さよりも大きく
			//空き領域の高さがノードの幅よりも大きかったら

			//同じく、面積や辺の長さの差を求め、
			//辺の長さの小さいほうを求める
			int leftOverHoriz = abs(freeRects[i].width - height);
			int leftOverVert = abs(freeRects[i].height - width);
			int shortSideFit = std::min(leftOverHoriz, leftOverVert);

			if (areaFit < bestAreaFit || (areaFit == bestAreaFit && shortSideFit < bestShortSideFit))
			{
				//ベストな空き領域のデータを更新するが
				//ノードの幅と高さを逆に設定する
				bestNode.x = freeRects[i].x;
				bestNode.y = freeRects[i].y;
				bestNode.width = height;
				bestNode.height = width;
				bestShortSideFit = shortSideFit;
				bestAreaFit = areaFit;

				bestNode.rotated = true;
			}
		}
	}

	//面積に基づいた最適な空き領域を返す
	return bestNode;
}

//新しい空き矩形を空きリストに入れる
void MaxRectPacking::insertNewFreeRect(const Rect& rect)
{
	for (size_t i = 0; i < newFreeRectLastSize;)
	{
		//この新しい矩形が、指定の空き矩形に完全含まれていないかどうか
		//含まれているなら、newFreeRectをリストに加えなくていい
		if (rect.isContain(newFreeRects[i]))
		{
			return;
		}

		//この時点でnewFreeRects[i]はrectを完全に覆ってはいない
		
		//rectはnewFreeRects[i]を完全に覆っているかどうか
		if (newFreeRects[i].isContain(rect))
		{
			//この時点で、rectはnewFreeRects[i]を完全に覆っている

			//newFreeRects[i]をrectに統合する

			newFreeRects[i] = newFreeRects[--newFreeRectLastSize];
			newFreeRects[newFreeRectLastSize] = newFreeRects.back();
			newFreeRects.pop_back();
		}
		else
		{
			i++;
		}
	}

	newFreeRects.push_back(rect);
}

//指定された空きノードと使用済みノードから空きノードを分割する
//分割された場合trueを返す
bool MaxRectPacking::splitFreeNode(const Rect& freeNode, const Rect& usedNode)
{
	//このノードと空いているノードが重なっていないなら
	//次の空きノードを調べさせる
	if (usedNode.x >= freeNode.x + freeNode.width || usedNode.x + usedNode.width <= freeNode.x
		|| usedNode.y >= freeNode.y + freeNode.height || usedNode.y + usedNode.height <= freeNode.y)
	{
		return false;
	}

	//この時点でusedNodeは空きのノードの一部に重なっている

	//これから、最大で4つの新しい空き矩形を新しい空き矩形リストに加える
	//それらは新しく追加される空き矩形は、お互いに包含しているかを判定はしない
	newFreeRectLastSize = newFreeRects.size();

	if (usedNode.x < freeNode.x + freeNode.width && usedNode.x + usedNode.width > freeNode.x)
	{
		//このノードが少なくとも、x軸で重なっている部分がある場合

		if (usedNode.y > freeNode.y && usedNode.y < freeNode.y + freeNode.height)
		{
			//ノードの上部が重なっている場合

			//新しいノードの高さを空きノードとこのノードの隙間分にする
			Rect newNode = freeNode;
			newNode.rotated = false;
			newNode.height = usedNode.y - newNode.y;
			insertNewFreeRect(newNode);
		}

		if (usedNode.y + usedNode.height < freeNode.y + freeNode.height)
		{
			//ノードの下部が重なっている場合

			//新しいノードの高さをノードとこのノードの隙間分だけにする
			Rect newNode = freeNode;
			newNode.rotated = false;
			newNode.y = usedNode.y + usedNode.height;
			newNode.height = freeNode.y + freeNode.height - (usedNode.y + usedNode.height);
			insertNewFreeRect(newNode);
		}
	}

	if (usedNode.y < freeNode.y + freeNode.height && usedNode.y + usedNode.height > freeNode.y)
	{
		//このノードがy軸で重なっている場合

		if (usedNode.x > freeNode.x && usedNode.x < freeNode.x + freeNode.width)
		{
			//ノードの左側が空き領域と重なっている場合

			//新しいノードの幅を空きノードとこのノードの隙間分だけにする
			Rect newNode = freeNode;
			newNode.rotated = false;
			newNode.width = usedNode.x - newNode.x;
			insertNewFreeRect(newNode);
		}

		if (usedNode.x + usedNode.width < freeNode.x + freeNode.width)
		{
			//ノードの右側が空き領域と重なっている部分がある場合

			//新しいノードの幅を空きノードとこのノードの隙間分だけにする
			Rect newNode = freeNode;
			newNode.rotated = false;
			newNode.x = usedNode.x + usedNode.width;
			newNode.width = freeNode.x + freeNode.width - (usedNode.x + usedNode.width);
			insertNewFreeRect(newNode);
		}
	}

	return true;
}

//空き矩形リストを調べて、冗長なノードを削除する
void MaxRectPacking::pruneFreeList()
{
	//新しい空き領域リスト古い空き領域リストに対して包含をテストする
	for (size_t i = 0; i < freeRects.size(); i++)
	{
		for (size_t j = 0; j < newFreeRects.size();)
		{
			if (newFreeRects[j].isContain(freeRects[i]))
			{
				//新しいほうが古いほうに完全に覆われていたら
				//古いほうに統合され、新しいほうはリストから消す

				newFreeRects[j] = newFreeRects.back();
				newFreeRects.pop_back();
			}
			else
			{
				//古い矩形が新しい矩形に含まれることはない

				j++;
			}
		}
	}

	//古い空き領域の末尾に、新しい空き領域を加える
	freeRects.insert(freeRects.end(), newFreeRects.begin(), newFreeRects.end());
	newFreeRects.clear();
}

//ビンを挿入する
Rect MaxRectPacking::insert(bool& packingSuccess,float width, float height)
{
	float maxArea = FLT_MAX;
	float maxLength = FLT_MAX;

	//最適な空き領域を選択
	Rect newNode = findPositionNewBAF(width, height, maxArea, maxLength);

	if (newNode.height < 1.0)
	{
		std::cerr << "not success" << std::endl;
		packingSuccess = false;
	}

	//ノードを配置する
	placeRect(newNode);

	//配置した領域を返す
	return newNode;
}