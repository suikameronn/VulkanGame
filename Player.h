#pragma once

#include"UI.h"
#include "Model.h"
#include"Camera.h"
#include"FileManager.h"

//プレイヤーキャラクターのクラス
class Player : public Model
{
	//カメラのy軸方向の回転角度
	//プレイヤーキャラクターをカメラの正面方向に向かせるためのもの
	float cameraDirDeg;

	//移動速度
	float moveSpeed;
	//ジャンプの高さ
	float maxJumpHeight;
	//キー入力を受け取る状態
	bool controllable;

	//キー入力からプレイヤーを移動させる
	glm::vec3 inputMove();

	//照準のUIについて
	std::shared_ptr<ImageData> targetImage;
	std::shared_ptr<UI> targetUI;

	//狙いを定めた時のカメラ位置
	glm::vec3 aimingCameraOffsetSrc;
	glm::vec3 aimingCameraOffset;

	//弾のステータス
	//弾のスピード
	float bulletSpeed;
	//レイキャスト時のレイの長さ
	float rayCastLength;
	//弾の進行方向
	glm::vec3 bulletDirection;
	//弾の射程距離
	float bulletDistanceLimit;

	bool aiming;
	//弾のGltfモデルのパス
	std::string bulletFilePath;
	//狙いを定める際の処理
	void aim();
	//弾を発射
	void shootBullet();

public:

	Player();
	Player(std::string luaScriptPath);

	//luaから呼び出される関数を登録される
	void registerGlueFunctions() override;

	//移動速度を設定する
	void setSpeed(float s);
	//ジャンプの高さを設定する
	void setMaxJumpHeight(float height);
	//初回フレームのみ実行
	void initFrameSetting() override;
	//キー入力の取得
	void customUpdate() override;
	//リスタート地点へプレイヤーをワープさせる
	void restart(glm::vec3 startPoint);

	//ターゲットUIの画像とサイズを設定
	void setTargetUIImageAndScale(std::string filePath,float scale);
	//狙いを定めた時のカメラの位置の設定
	void setAimingCameraPos(glm::vec3 pos) { aimingCameraOffsetSrc = pos; }

	void Update() override;
	void updateTransformMatrix() override;//座標変換行列の更新

	void collision(std::shared_ptr<Model> model) override {};
};

/*以下の静的関数はluaから呼び出される*/

//移動速度の設定
static int glueSetSpeed(lua_State* lua)
{
	float speed = static_cast<float>(lua_tonumber(lua, -1));
	lua_getglobal(lua, "Data");
	Player* player = static_cast<Player*>(lua_touserdata(lua, -1));
	if (!player)
	{
		return 0;
	}

	player->setSpeed(speed);

	return 0;
}

//ジャンプの高さの設定
static int glueSetJumpHeight(lua_State* lua)
{
	float height = static_cast<float>(lua_tonumber(lua, -1));
	lua_getglobal(lua, "Data");
	Player* player = static_cast<Player*>(lua_touserdata(lua, -1));

	if (!player)
	{
		return 0;
	}

	player->setMaxJumpHeight(height);

	return 0;
}

//照準UIについて設定
static int glueSetTargetUI(lua_State* lua)
{
	std::string filePath = static_cast<std::string>(lua_tostring(lua, -2));
	float scale = static_cast<float>(lua_tonumber(lua, -1));

	lua_getglobal(lua, "Data");
	Player* player = static_cast<Player*>(lua_touserdata(lua, -1));

	if (!player)
	{
		return 0;
	}

	player->setTargetUIImageAndScale(filePath, scale);

	return 0;
}

//狙いを定めた時のカメラの位置の設定
static int glueSetAimingCameraPos(lua_State* lua)
{
	glm::vec3 pos;
	for (int i = 0; i < 3; i++)
	{
		pos[i] = static_cast<float>(lua_tonumber(lua, -3 + i));
	}

	lua_getglobal(lua, "Data");
	Player* player = static_cast<Player*>(lua_touserdata(lua, -1));

	if (!player)
	{
		return 0;
	}

	player->setAimingCameraPos(pos);

	return 0;
}