#include"Model.h"

#include"RTree.h"

RNode::RNode(RNode* parent, glm::vec3 newMin, glm::vec3 newMax)
{
	this->parent = parent;

	isUpdate = true;
	objCount = 0;
	childNodeCount = 0;

	min = newMin;
	max = newMax;

	std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

RNode::RNode(RNode* parent, std::vector<Model*>& objects)
{
	this->parent = parent;

	isUpdate = true;
	objCount = static_cast<int>(objects.size());

	childNodeCount = 0;

	std::fill(nodeObject.begin(), nodeObject.end(), nullptr);

	//ノードにオブジェクトをコピー
	std::copy(objects.begin(), objects.end(), nodeObject.begin());

	//オブジェクトに所属するノードを設定する
	for (int i = 0; i < objCount; i++)
	{
		nodeObject[i]->setRNode(this);
	}

	std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

RNode::RNode(RNode* parent, std::vector<RNode*>& srcChildren)
{
	this->parent = parent;

	isUpdate = true;
	objCount = 0;
	childNodeCount = static_cast<int>(srcChildren.size());

	std::fill(this->children.begin(), this->children.end(), nullptr);

	//ノードにオブジェクトをコピー
	std::copy(srcChildren.begin(), srcChildren.end(), this->children.begin());

	//分割した片方のノードに分配された親ノードをこれに更新
	for (int i = 0; i < childNodeCount; i++)
	{
		children[i]->parent = this;
	}

	std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

RNode::~RNode()
{
	for (int i = 0; i < childNodeCount; i++)
	{
		if (children[i] == nullptr)
		{
			delete children[i];
		}
	}
}

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
			min[j] = std::min(min[j], nodeObject[i]->getMbrMin()[j]);
			max[j] = std::max(max[j], nodeObject[i]->getMbrMax()[j]);
		}
	}

	//子ノードのAABBをすべて内包するAABBを作る
	for (int i = 0; i < childNodeCount; i++)
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

////オブジェクトからノードの参照を更新
void RNode::updateRefNode()
{
	if (childNodeCount == 0)
	{
		for (int i = 0; i < objCount; i++)
		{
			nodeObject[i]->setRNode(this);
		}
	}
}

void RNode::genAABB(glm::vec3& newMin, glm::vec3& newMax)
{
	for (int i = 0; i < 3; i++)
	{
		newMin[i] = std::min(min[i], newMin[i]);
		newMax[i] = std::max(max[i], newMax[i]);
	}
}

//最も距離の離れたオブジェクトのペアのインデックスを返す(リーフノード版)
std::array<int,2> RNode::calcFarthestNodePair(std::array<Model*, RNodeMAX + 1>& objects,
	glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex)
{
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
		tMin = objects[minIndex[i]]->getMbrMax();
		tMax = objects[maxIndex[i]]->getMbrMin();

		float dist = tMax[i] - tMin[i];
		dist /= distance[i];

		if (farthestDist < dist)
		{
			farthestDist = dist;
			pairNodeIndex[0] = minIndex[i];
			pairNodeIndex[1] = maxIndex[i];
		}
	}

	return pairNodeIndex;
}

//最も距離の離れたオブジェクトのペアのインデックスを返す(中間ノード版)
std::array<int, 2> RNode::calcFarthestNodePair(std::array<RNode*, RNodeMAX + 1>& nodes,
	glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex)
{
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
		tMin = nodes[minIndex[i]]->getMbrMax();
		tMax = nodes[maxIndex[i]]->getMbrMin();

		float dist = tMax[i] - tMin[i];
		dist /= distance[i];

		if (farthestDist < dist)
		{
			farthestDist = dist;
			pairNodeIndex[0] = minIndex[i];
			pairNodeIndex[1] = maxIndex[i];
		}
	}

	return pairNodeIndex;
}

////分割した際のそれぞれのノードに収めるエントリのインデックスを決める(子ノード版)
std::array<std::vector<int>, 2> RNode::linearCostSplit(std::array<Model*, RNodeMAX + 1>& objects,
	glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex)
{
	//もう片方のノードに入れるオブジェクトをまとめる
	std::vector<int> thisNodeObjectIndex;
	std::vector<int> anotherNodeObjectIndex;

	//最も距離の離れたオブジェクトのペアのインデックスを調べる
	std::array<int, 2> pairNodeIndex = calcFarthestNodePair(objects, allObjMax, allObjMin, minIndex, maxIndex);

	//それぞれ最も離れたオブジェクト、別々のノードに入れる
	thisNodeObjectIndex.push_back(pairNodeIndex[0]);
	anotherNodeObjectIndex.push_back(pairNodeIndex[1]);

	//それぞれのノードにオブジェクトを登録するときのインデックス
	//既に両方のノードには最も離れたオブジェクトが一つずつ入っている
	for (int i = 0; i < objects.size(); i++)
	{
		//最大の距離を持つペアは除く
		if (i == pairNodeIndex[0] || i == pairNodeIndex[1])
		{
			continue;
		}

		//そのほかのオブジェクトの分配についてはランダムに分配する
		if (std::rand() % 2)
		{
			thisNodeObjectIndex.push_back(i);
		}
		else
		{
			anotherNodeObjectIndex.push_back(i);
		}
	}

	return { thisNodeObjectIndex,anotherNodeObjectIndex };
}

//分割した際のそれぞれのノードに収めるエントリのインデックスを決める(中間ノード版)
std::array<std::vector<int>, 2> RNode::linearCostSplit(std::array<RNode*, RNodeMAX + 1>& nodes,
	glm::vec3 allObjMax, glm::vec3 allObjMin, glm::ivec3 minIndex, glm::ivec3 maxIndex)
{
	//もう片方のノードに入れるオブジェクトをまとめる
	std::vector<int> thisNodeObjectIndex;
	std::vector<int> anotherNodeObjectIndex;

	//最も距離の離れたオブジェクトのペアのインデックスを調べる
	std::array<int, 2> pairNodeIndex = calcFarthestNodePair(nodes, allObjMax, allObjMin, minIndex, maxIndex);

	//それぞれ最も離れたオブジェクト、別々のノードに入れる
	thisNodeObjectIndex.push_back(pairNodeIndex[0]);
	anotherNodeObjectIndex.push_back(pairNodeIndex[1]);

	//それぞれのノードにオブジェクトを登録するときのインデックス
	//既に両方のノードには最も離れたオブジェクトが一つずつ入っている
	for (int i = 0; i < RNodeMAX; i++)
	{
		//最大の距離を持つペアは除く
		if (i == pairNodeIndex[0] || i == pairNodeIndex[1])
		{
			continue;
		}

		//そのほかのオブジェクトの分配についてはランダムに分配する
		if (std::rand() % 2)
		{
			thisNodeObjectIndex.push_back(i);
		}
		else
		{
			anotherNodeObjectIndex.push_back(i);
		}
	}

	return { thisNodeObjectIndex,anotherNodeObjectIndex };
}

//現在のノードを分割して、作成したもう一方のノードを返す(リーフノード版)
void RNode::split(Model* model, glm::vec3 min, glm::vec3 max)
{
	//各軸の最大値と最小値を計算する
	//その値を持つノードのインデックスを記録する
	glm::vec3 allObjMin = glm::vec3(FLT_MAX), allObjMax = glm::vec3(-FLT_MAX);
	glm::ivec3 minIndex , maxIndex;

	//オブジェクトをノードに追加した仮の配列を追加する
	std::array<Model*, RNodeMAX + 1> objects;
	for (int i = 0; i < RNodeMAX; i++)
	{
		objects[i] = nodeObject[i];
	}
	objects[RNodeMAX] = model;

	//いったんこのノードに登録されたオブジェクトを初期化する
	std::fill(nodeObject.begin(), nodeObject.end(), nullptr);

	//配列に各オブジェクトの各軸の最大値と最小値を記録する
	//各軸の数だけ、オブジェクトの数だけ最大値と最小値とそのオブジェクトのインデックスのペアを持つ
	std::array<std::array<std::pair<float,int>, RNodeMAX + 1>, 3> maxMBR;
	std::array<std::array<std::pair<float, int>, RNodeMAX + 1>, 3> minMBR;
	
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < RNodeMAX + 1; j++)
		{
			maxMBR[i][j].first = objects[j]->getMbrMax()[i];
			minMBR[i][j].first = objects[j]->getMbrMin()[i];

			maxMBR[i][j].second = j;
			minMBR[i][j].second = j;
		}
	}

	//ソートする ただこの際最大値と最小値を基準にソートして、元のオブジェクトのインデックスはその動きに付随する
	for (int i = 0; i < 3; i++)
	{
		//降順にする
		std::sort(maxMBR[i].begin(), maxMBR[i].end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b)
			{
				return a.first > b.first;
			});

		//昇順
		std::sort(minMBR[i].begin(), minMBR[i].end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b)
			{
				return a.first < b.first;
			});

		//一番大きい値をもつインデックスを記録
		maxIndex[i] = maxMBR[i][0].second;

		//一番小さい値を持つインデックスを記録
		minIndex[i] = minMBR[i][0].second;

		//ただし、それぞれ同じインデックスを記録する場合は二番目のインデックスを記録する
		if (minIndex[i] == maxIndex[i])
		{
			minIndex[i] = minMBR[i][1].second;
		}
	}

	//LinearCostアルゴリズムでそれぞれのノードに収めるエントリのインデックスを決める
	std::array<std::vector<int>, 2> pairNodeObjectIndex = linearCostSplit(objects, allObjMax, allObjMin, minIndex, maxIndex);

	//もう一方のノードに登録するオブジェクトを記録するための配列
	std::vector<Model*> anotherObjects(pairNodeObjectIndex[1].size());

	//インデックスからそれぞれのノードにオブジェクトを登録する
	for (int i = 0; i < pairNodeObjectIndex[0].size(); i++)
	{
		nodeObject[i] = objects[pairNodeObjectIndex[0][i]];
	}
	for (int i = 0; i < pairNodeObjectIndex[1].size(); i++)
	{
		anotherObjects[i] = objects[pairNodeObjectIndex[1][i]];
	}

	//このノードのオブジェクトの数を設定する
	objCount = static_cast<int>(pairNodeObjectIndex[0].size());

	//オブジェクトに所属するノードを設定する
	for (int i = 0; i < objCount; i++)
	{
		nodeObject[i]->setRNode(this);
	}

	//もう一方のノードを作成し、オブジェクトを登録する
	RNode* anotherNode = new RNode(parent, anotherObjects);

	//お互いのノードのAABBを更新
	updateAABB();
	anotherNode->updateAABB();

	updateRefNode();
	anotherNode->updateRefNode();

	//中間ノードにもう片方のノードを登録する
	if (parent)
	{
		parent->addChildNode(anotherNode);
	}
	else
	{
		throw std::runtime_error("This Node is not Leaf Node!");
	}
}

//現在のノードを分割して、作成したもう一方のノードを返す(中間ノード版)
void RNode::split(RNode* node, glm::vec3 min, glm::vec3 max)
{
	//各軸の最大値と最小値を計算する
	//その値を持つノードのインデックスを記録する
	glm::vec3 allNodeMin = glm::vec3(FLT_MAX), allNodeMax = glm::vec3(-FLT_MAX);
	glm::ivec3 minIndex, maxIndex;

	//オブジェクトをノードに追加した仮の配列を追加する
	std::array<RNode*, RNodeMAX + 1> nodes;
	for (int i = 0; i < RNodeMAX; i++)
	{
		nodes[i] = children[i];
	}
	nodes[RNodeMAX] = node;

	//いったんこのノードに登録されたオブジェクトを初期化する
	std::fill(children.begin(), children.end(), nullptr);

	//配列に各オブジェクトの各軸の最大値と最小値を記録する
	//各軸の数だけ、オブジェクトの数だけ最大値と最小値とそのオブジェクトのインデックスのペアを持つ
	std::array<std::array<std::pair<float, int>, RNodeMAX + 1>, 3> maxMBR;
	std::array<std::array<std::pair<float, int>, RNodeMAX + 1>, 3> minMBR;

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < RNodeMAX + 1; j++)
		{
			maxMBR[i][j].first = nodes[j]->getMbrMax()[i];
			minMBR[i][j].first = nodes[j]->getMbrMin()[i];

			maxMBR[i][j].second = j;
			minMBR[i][j].second = j;
		}
	}

	//ソートする ただこの際最大値と最小値を基準にソートして、元のオブジェクトのインデックスはその動きに付随する
	for (int i = 0; i < 3; i++)
	{
		//降順にする
		std::sort(maxMBR[i].begin(), maxMBR[i].end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b)
			{
				return a.first > b.first;
			});

		//昇順
		std::sort(minMBR[i].begin(), minMBR[i].end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b)
			{
				return a.first < b.first;
			});

		//一番大きい値をもつインデックスを記録
		maxIndex[i] = maxMBR[i][0].second;

		//一番小さい値を持つインデックスを記録
		minIndex[i] = minMBR[i][0].second;

		//ただし、それぞれ同じインデックスを記録する場合は二番目のインデックスを記録する
		if (minIndex[i] == maxIndex[i])
		{
			minIndex[i] = minMBR[i][1].second;
		}
	}

	//LinearCostアルゴリズムでそれぞれのノードに収めるエントリのインデックスを決める
	std::array<std::vector<int>, 2> pairNodeObjectIndex = linearCostSplit(nodes, allNodeMax, allNodeMin, minIndex, maxIndex);

	//もう一方のノードに登録するオブジェクトを記録するための配列
	std::vector<RNode*> anotherNodeChildren(pairNodeObjectIndex[1].size());

	//自分の子ノードを初期化する
	std::fill(children.begin(), children.end(), nullptr);

	//インデックスからそれぞれのノードにオブジェクトを登録する
	for (int i = 0; i < pairNodeObjectIndex[0].size(); i++)
	{
		children[i] = nodes[pairNodeObjectIndex[0][i]];
	}
	for (int i = 0; i < pairNodeObjectIndex[1].size(); i++)
	{
		anotherNodeChildren[i] = nodes[pairNodeObjectIndex[1][i]];
	}

	//中間ノードにもう片方のノードを登録する
	if (parent)
	{//このノードが中間ノードの場合

		//このノードの子ノードの数を更新する
		childNodeCount = static_cast<int>(pairNodeObjectIndex[0].size());

		//もう一方のノードを作成し、子ノードを登録する
		//子ノードの親はコンストラクタ上で設定する
		RNode* anotherNode = new RNode(parent, anotherNodeChildren);

		//お互いのノードのAABBを更新
		updateAABB();
		anotherNode->updateAABB();

		parent->addChildNode(anotherNode);
	}
	else
	{//このノードがルートノードの場合
	 //このノードを分割するというより、新たに二つのノードを作り、
	 //その片方にこのノードのオブジェクトなどを引き継ぎ、
	 //このノードはデータをリセットして、ルートノードとして使う

		//このノードのデータを新たなノードに引き継ぐ
		std::vector<RNode*> tmp(pairNodeObjectIndex[0].size());
		for (int i = 0; i < static_cast<int>(pairNodeObjectIndex[0].size()); i++)
		{
			tmp[i] = children[i];
		}
		RNode* node1 = new RNode(this, tmp);

		//もう一方のノードを作成し、オブジェクトを登録する
		RNode* node2 = new RNode(this, anotherNodeChildren);

		//お互いのノードのAABBを更新
		node1->updateAABB();
		node2->updateAABB();

		//このノードのデータをリセットして、再びルートノードにする
		resetToRootNode();

		//ルートノードに分割したノードを登録する
		this->addChildNode(node1);
		this->addChildNode(node2);

		//ルートノードのAABBを更新
		this->updateAABB();
	}
}

void RNode::insert(Model* model, glm::vec3 min, glm::vec3 max)
{
	if (childNodeCount == 0)
	{//リーフノードの場合

		if (objCount >= RNodeMAX)
		{//ノードのオブジェクトが満杯な時は分割を行う

			//分割を行ったら、中間ノードに分割後のノードを渡す
			split(model, min, max);
		}
		else
		{//空きがあるときは、ノードにオブジェクトを追加し、AABBを拡張する

			nodeObject[objCount] = model;
			model->setRNode(this);

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

		for (int i = 0; i < static_cast<int>(childNodeCount); i++)
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
			for (int i = 0; i < static_cast<int>(childNodeCount); i++)
			{
				float volume = children[i]->calcVolume();

				if (newAABBVolume > volume)
				{
					newAABBVolume = volume;
					closestNodeIndex = i;
				}
			}
		}

		//一番最初のルートノードの子ノードにオブジェクトを入れる際、一つ目のノードにオブジェクトを入れる
		if (closestNodeIndex == -1)
		{
			closestNodeIndex = 0;
		}

		//リーフノード目指して、ノードを下る
		children[closestNodeIndex]->insert(model, min, max);

		//オブジェクトをリーフノードに追加した後
		//子ノードの拡張されたAABBをすべて内包する
		//AABBを計算する

		updateAABB();
	}
}

//データをリセットして、このノードをルートノードに戻す
void RNode::resetToRootNode()
{
	//ノードやオブジェクト数の初期化
	childNodeCount = 0;
	objCount = 0;

	//登録されたデータの初期化
	std::fill(nodeObject.begin(), nodeObject.end(), nullptr);
	std::fill(children.begin(), children.end(), nullptr);

	//ルートノードのAABBを初期化
	min = glm::vec3(-FLT_MAX);
	max = glm::vec3(FLT_MAX);
}

//ノード内のオブジェクトを削除する
void RNode::deleteObject(Model* model)
{
	//該当のオブジェクトを削除し、オブジェクトのポインタを先頭に詰める
	for (int i = 0; i < objCount; i++)
	{
		//該当オブジェクトをノードから削除する
		if (nodeObject[i] == model)
		{
			nodeObject[i] = nullptr;
			break;
		}
	}

	//オブジェクト側のノードの登録を削除する
	model->setRNode(nullptr);

	sortNodeObject();

	//オブジェクト数を減らす
	objCount--;

	//ここまででノード内のオブジェクトの配列は整頓され
	//配列は先頭に向けてオブジェクトへの参照が詰められ、間にnullptrが挟まっていない状態に

	//この時、オブジェクト数が0になったら、このノード自体を削除し、上のノードのAABBも調整する
	if (objCount <= 0)
	{
		//親ノードから自分を削除する
		parent->deleteNode(this);

		delete this;
	}
}

//子ノードの登録を削除する
void RNode::deleteNode(RNode* childNode)
{
	for (int i = 0; i < childNodeCount; i++)
	{
		if (children[i] == childNode)
		{
			children[i] = nullptr;

			//この部分を通らないときがある
			std::cout << "AAAAAAAAAAAAAAAAAAAAAA" << std::endl;

			break;
		}
	}

	std::cout << "B" << std::endl;

	//nullptrが前のほうに来ないように整頓
	sortChildNode();

	childNodeCount--;

	int a = 0;
	for (int i = 0; i < 3; i++)
	{
		if (children[i] != nullptr)
		{
			a++;
		}
	}

	if (a != childNodeCount)
	{
		std::cout << a << std::endl;
	}
}

//一部でも重なっていたらtrue
bool RNode::isOverlap(glm::vec3 newMin, glm::vec3 newMax)
{
	bool isOverlap = true;

	for (int i = 0; i < 3; i++)
	{
		if (min[i] > newMax[i] || max[i] < newMin[i])
		{
			isOverlap = false;
		}
	}

	return isOverlap;
}

//一部でも重なっていたらtrue
bool RNode::isOverlap(glm::vec3 newMin, glm::vec3 newMax, Model* model)
{
	bool isOverlap = true;

	for (int i = 0; i < 3; i++)
	{
		if (model->getMbrMin()[i] > newMax[i] || model->getMbrMax()[i] < newMin[i])
		{
			isOverlap = false;
		}
	}

	return isOverlap;
}

//与えられた範囲のすべて、あるいは一部を内包するノードのオブジェクトを配列に入れる
void RNode::searchInRange(std::vector<Model*>& collisionDetectTarget, glm::vec3 min,glm::vec3 max)
{
	//まずそのノードが与えられた範囲の一部かすべてを内包するかどうかを調べる
	if (isOverlap(min, max))
	{
		//より子ノードへ下がる、そこで再び範囲探索を行う
		for (int i = 0; i < childNodeCount; i++)
		{
			children[i]->searchInRange(collisionDetectTarget, min, max);
		}

		//与えられた範囲とノードのMBRが一部でも重なっていたら
		for (int i = 0; i < objCount; i++)
		{
			//オブジェクトのMBRとも重なっていたら
			//のちに詳細な当たり判定を行うために配列にオブジェクトを加える
			if (isOverlap(min, max, nodeObject[i]))
			{
				collisionDetectTarget.push_back(nodeObject[i]);
			}
		}
	}
}

RTree::RTree()
{
	root = new RNode(nullptr, glm::vec3(-FLT_MAX), glm::vec3(FLT_MAX));

	root->addChildNode(new RNode(root, glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX)));

	root->getChild(0)->updateAABB();
}

RTree::~RTree()
{
	delete root;
}

void RTree::insert(Model* model,glm::vec3 min,glm::vec3 max)
{
	root->insert(model, min, max);
}

//オブジェクト移動したときの関数
//オブジェクトから木から削除し、再び追加する
void RTree::reflectMove(Model* model, RNode* currentNode)
{
	//まずオブジェクトの所属するノードから自分を削除する
	currentNode->deleteObject(model);

	//再び適切なノードに挿入する
	//オブジェクトにはinsert関数内で新たに所属するノードを設定される
	insert(model, model->getMbrMin(), model->getMbrMax());
}

//与えられたMBRから詳細な当たり判定を行う必要があるオブジェクトを配列にまとめる
void RTree::broadPhaseCollisionDetection(std::vector<Model*>& collisionDetectTarget, glm::vec3 min, glm::vec3 max)
{
	root->searchInRange(collisionDetectTarget, min, max);
}