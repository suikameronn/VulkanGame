#pragma once
#include<vector>
#include<bitset>
#include <time.h>

#include"EnumList.h"
#include"GltfModel.h"
#include"Colider.h"
#include"RTree.h"

class Scene;

//3Dモデルを持つオブジェクトを担うクラス
class Model:public Object, public std::enable_shared_from_this<Model>
{
protected:

	//R-tree用の初期のAABB
	glm::vec3 initMin, initMax;
	//R-tree用のAABB
	glm::vec3 min, max;

	//現在所属しているRTreeのノード
	RNode* rNode;

	//R-tree用のMBRについて最大値と最小値
	glm::vec3 mbrMin, mbrMax;
	
	//レイキャスト時使用
	Scene* scene;

	//物理挙動用
	std::unique_ptr<PhysicBase> physicBase;

	//gltfモデルへの参照
	std::shared_ptr<GltfModel> gltfModel;

	//mvp行列用のバッファー
	MappedBuffer modelViewMappedBuffer;
	//gltfモデルの頂点関連用のバッファ
	std::vector<BufferObject> pointBuffers;
	//アニメーション用行列のバッファ
	std::vector<MappedBuffer> animationMappedBuffers;
	//アニメーション用行列の配列
	std::vector<std::array<glm::mat4, 128>> jointMatrices;

	//アニメーション開始時間
	clock_t startTime;
	//現在の時間
	clock_t currentTime;
	//アニメーションの再生時間
	double deltaTime;
	//アニメーションの切り替えフラッグ、アニメーション開始時間をリセット
	//同一アニメーションをループさせる際、アニメーションが終了したときもtrueに
	bool animationChange;

	//gltfモデルのマテリアルの上書き用
	std::shared_ptr<Material> material;
	//コライダー
	std::shared_ptr<Colider> colider;

	//アイドル時に再生するアニメーションの名前
	std::string defaultAnimationName;
	//現在再生しているアニメーションの名前
	std::string currentPlayAnimationName;
	//次の再生する予定のアニメーション
	std::string nextPlayAnimationName;
	//gltfモデルの持つアニメーションの名前の配列
	std::vector<std::string> animationNames;

	//自身が床の場合、上に載っているオブジェクトの配列、自身の移動時、それらのオブジェクトも追従させる
	std::vector<Model*> groundingObjects;

	//コライダー用のAABBからMBRを計算
	void calcMBR();

public:

	Model();
	Model(std::string luaScriptPath);
	~Model() {};

	void registerGlueFunctions() override;//glue関数の設定

	float gravity;//重力の強さ
	void setZeroVelocity();//速度のリセット
	void cancelGravity();//重力の打ち消し

	//RTree用のAABBを設定する
	void getMbrMinMax(glm::vec3& min, glm::vec3& max)
	{
		min = this->mbrMin;
		max = this->mbrMax;
	}

	glm::vec3 getMbrMin()
	{
		return min;
	}

	glm::vec3 getMbrMax()
	{
		return max;
	}

	//現在所属しているノードを設定する
	void setRNode(RNode* node) { rNode = node; }

	//スケール
	glm::vec3 scale;

	//自分が床の上に立っているかどうか、立っている場合は重力の影響を無視する
	bool isGrounding;

	void setDefaultAnimationName(std::string name);//デフォルトのアニメーションを設定

	void sendPosToChildren();//子オブジェクトに親の移動を反映

	void setgltfModel(std::shared_ptr<GltfModel> model);//3DCGモデルを登録
	GltfNode* getRootNode() { return gltfModel->getRootNode(); }
	std::shared_ptr<GltfModel> getGltfModel() { return gltfModel; }

	void setBaseColor(glm::vec4 baseColor);//モデルの色を上書き

	void switchPlayAnimation();//アニメーションをデフォルトに切り替える
	void switchPlayAnimation(std::string nextAnimation);//指定のアニメーションに切り替える
	void playAnimation();//アニメーションの再生
	std::array<glm::mat4, 128>& getJointMatrices(int index);//特定のスケルトンのアニメーション行列を計算

	std::vector<DescSetData> descSetDatas;
	BufferObject* getPointBufferData() { return pointBuffers.data(); }//頂点用バッファの取得
	MappedBuffer& getModelViewMappedBuffer() { return modelViewMappedBuffer; }//モデルビュー行列用のバッファの取得
	MappedBuffer* getAnimationMappedBufferData();//アニメーション用の行列のバッファの取得

	//衝突を解消時、このオブジェクトを動かせるかどうか、障害物の場合、このオブジェクトは動かさないため,falseとなる
	bool isMovable;
	//コライダーを持っているかどうか
	bool hasColider();
	void setColider(bool isConvex);//コライダーの設定
	std::shared_ptr<Colider> getColider() { return colider; }

	
	std::shared_ptr<Model> rayCast(glm::vec3 origin,glm::vec3 dir,float maxLength,glm::vec3& normal);

	void updateTransformMatrix() override;//座標変換行列の更新

	void cleanupVulkan();//Vulkanの変数の破棄

	void setPosition(glm::vec3 pos) override;//位置の設定

	void Update() override;//更新処理
	void customUpdate() override;//特殊な更新処理

	glm::vec3 getLastScale();
	void setLastFrameTransform() override;//一つ前のフレームの座標などのデータを設定

	void createTransformTable() override;

	void sendTransformToLua() override;//luaに座標などを送る
	void receiveTransformFromLua() override;//luaから座標などを受け取る

	void initFrameSetting() override;//初期フレームのみの処理

	bool isGround(glm::vec3& normal);//オブジェクトが床に接しているかどうか
	void addGroundingObject(Model* object);//床に接していたらそれを追加
	void clearGroundingObject();
};

/*以下の静的関数はluaスクリプト上から呼び出される*/

namespace glueModelFunction//Modelクラス用のglue関数
{
	//Modelクラスのスケールの設定
	static int glueSetScale(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -1));

		switch (obj->getObjNum())
		{
		case 1:
			Model * model = dynamic_cast<Model*>(obj);
			model->scale.x = static_cast<float>(lua_tonumber(lua, -4));
			model->scale.y = static_cast<float>(lua_tonumber(lua, -3));
			model->scale.z = static_cast<float>(lua_tonumber(lua, -2));
			break;
		}

		return 0;
	}
}