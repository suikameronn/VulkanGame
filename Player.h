#include "Model.h"
#include"Camera.h"

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