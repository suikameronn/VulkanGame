#pragma once
#include<vector>
#include<bitset>
#include <time.h>

#include"EnumList.h"
#include"GltfModel.h"
#include"Colider.h"

class Scene;

//3Dモデルを持つオブジェクトを担うクラス
class Model:public Object
{
protected:

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
	//gltfモデルの持つアニメーションの名前の配列
	std::vector<std::string> animationNames;
	//自身が床の場合、上に載っているオブジェクトの配列、自身の移動時、それらのオブジェクトも追従させる
	std::vector<std::weak_ptr<Model>> groundingObjects;

public:

	Model();
	Model(std::string luaScriptPath);
	~Model() {};

	void registerGlueFunctions() override;//glue関数の設定

	float gravity;//重力の強さ
	void setZeroVelocity();//速度のリセット
	void cancelGravity();//重力の打ち消し

	//スケール
	glm::vec3 scale;

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

	
	std::shared_ptr<Model> rayCast(glm::vec3 origin,glm::vec3 dir,float maxLength);

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

	bool isGround();//オブジェクトが床に接しているかどうか
	void addGroundingObject(std::weak_ptr<Model> object);//床に接していたらそれを追加
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