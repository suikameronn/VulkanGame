#include"Model.h"

#include"RTree.h"

//このノードのAABBを更新
void RNode::updateAABB()
{
	min = glm::vec3(FLT_MAX);
	max = glm::vec3(-FLT_MAX);

	//まずノード内のオブジェクトのAABBを内包するように
	//ノードのAABBを拡張
	for (int i = 0; i < objCount; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			min[j] = std::min(min[j], nodeObject[i].lock()->getMin()[j]);
			max[j] = std::max(max[j], nodeObject[i].lock()->getMax()[j]);
		}
	}

	//子ノードのAABBをすべて内包するAABBを作る
	for (int i = 0; i < children.size(); i++)
	{
		children[i]->genAABB(min, max);
	}
}

//このノードのAABBを更新(中間ノード)
void RNode::updateAABB(int index)
{
	//AABBが変更された子ノードのAABBをも内包するAABBを計算する
	children[index]->genAABB(min, max);
}

void RNode::genAABB(glm::vec3& newMin, glm::vec3& newMax)
{
	for (int i = 0; i < 3; i++)
	{
		newMin[i] = std::min(min[i], newMin[i]);
		newMax[i] = std::max(max[i], newMax[i]);
	}
}

//現在のノードを分割して、作成したもう一方のノードを返す
RNode* RNode::split(std::weak_ptr<Model> model, glm::vec3 min, glm::vec3 max)
{
	//各軸の最大値と最小値を計算する
	//その値を持つノードのインデックスを記録する
	glm::vec3 allObjMin, allObjMax;
	glm::ivec3 minIndex, maxIndex;

	//オブジェクトをノードに追加した仮の配列を追加する
	std::array<std::weak_ptr<Model>, RNodeMAX + 1> objects;
	for (int i = 0; i < RNodeMAX; i++)
	{
		objects[i] = nodeObject[i];
	}
	objects[RNodeMAX + 1] = model;


	//上の配列のオブジェクトのAABBの各軸の最小値と最大値を取得
	for (int i = 0; i < RNodeMAX + 1; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			//ノード内のAABBから各軸の最小値、最大値を取得
			glm::vec3 objMin, objMax;
			objects[i].lock()->getMinMax(objMin, objMax);

			if (allObjMin[j] > objMin[j])
			{
				allObjMin[j] = objMin[j];
				minIndex[j] = i;
			}

			if (allObjMax[j] > objMax[j])
			{
				allObjMax[j] = objMax[j];
				maxIndex[j] = i;
			}
		}
	}

	//LinearCostアルゴリズム

	//分母を計算
	std::array<float, 3> distance;
	for (int i = 0; i < 3; i++)
	{
		distance[i] = allObjMax[i] - allObjMin[i];
	}

	float farthestDist = -FLT_MAX;
	glm::vec3 tMin, tMax;
	std::array<int, 2> pairNodeIndex;

	//正規化された距離が最も大きいノードのペアを探す
	for (int i = 0; i < 3; i++)
	{
		tMin = objects[minIndex[i]].lock()->getMax();
		tMax = objects[maxIndex[i]].lock()->getMin();

		float dist = tMax[i] - tMin[i];
		dist /= distance[i];

		if (farthestDist < dist)
		{
			farthestDist = dist;
			pairNodeIndex[0] = minIndex[i];
			pairNodeIndex[1] = maxIndex[i];
		}
	}

	//自分のノードのAABBを更新 この場合は縮小


	//RNode* node = new RNode();
}

void RNode::insert(std::weak_ptr<Model> model, glm::vec3 min, glm::vec3 max)
{
	if (children.size() == 0)
	{//リーフノードの場合

		if (objCount > RNodeMAX)
		{//ノードのオブジェクトが満杯な時は分割を行う

			//分割を行ったら、中間ノードに分割後のノードを渡す
			parent->addChildNode(split(model, min, max));
		}
		else
		{//空きがあるときは、ノードにオブジェクトを追加し、AABBを拡張する
			nodeObject[objCount] = model;
			objCount++;

			//オブジェクトのAABBを含むようにノードのAABBを拡張する
			expandAABB(min, max);
		}
	}
	else
	{//リーフノードではない場合

		int closestNodeIndex = -1;
		float newAABBVolume = FLT_MAX;
		glm::vec3 newMin, newMax;

		for (int i = 0; i < children.size(); i++)
		{
			newMin = min;
			newMax = max;

			//オブジェクトを追加したときに
			//それを含むAABBを計算する
			children[i]->genAABB(newMin, newMax);

			//オブジェクトを追加したときのAABBの体積の増加量を計算する
			//(体積の増加量) = (追加した後の体積) - (元の体積)
			float volume = calcVolume(newMin, newMax) - children[i]->calcVolume();

			//体積の増加量が最小となるノードを求める
			if (volume > 0.0f && newAABBVolume > volume)
			{
				newAABBVolume = volume;
				closestNodeIndex = i;
			}
		}

		//もし、すべての子ノードがオブジェクトのAABBを完全に内包する場合
		//最も元の体積の小さいノードを選択する
		if (closestNodeIndex == -1)
		{
			for (int i = 0; i < 3; i++)
			{
				float volume = children[i]->calcVolume();

				if (newAABBVolume > volume)
				{
					newAABBVolume = volume;
					closestNodeIndex = i;
				}
			}
		}

		//リーフノード目指して、ノードを下る
		children[closestNodeIndex]->insert(model, min, max);

		//オブジェクトをリーフノードに追加した後
		//子ノードの拡張されたAABBをすべて内包する
		//AABBを計算する
		updateAABB(closestNodeIndex);
	}
}

RTree* RTree::instance = nullptr;

RTree::RTree()
{
	root = new RNode(nullptr, glm::vec3(0.0f), glm::vec3(0.0f));
}

void RTree::insert(std::weak_ptr<Model> model,glm::vec3 min,glm::vec3 max)
{

}