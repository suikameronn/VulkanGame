#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include<glm/glm.hpp>
#include<glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include"Controller.h"

//仮想的なカメラのクラス
class Camera
{
private:
	//座標
	glm::vec3 position;
	//追従するオブジェクトの座標
	glm::vec3 parentPos;
	
	//球面座標上の半径
	float distance;
	//追従するオブジェクトとの距離
	glm::vec3 offsetPos;

	//画面の比率
	float aspect;
	//視野角
	float viewAngle;
	//カメラの移動速度
	float viewPointSpeed;

	//球面座標上の角度
	float theta, phi;

	//クリップ範囲
	float zNear, zFar;

	//ビュー行列の計算、キューブマッピング用の行列も同時に計算
	void calcViewMat();
public:
	//カメラの正面、右、上方向のベクトル
	glm::vec3 forward, right, up;

	//通常のレンダリング用のビュー行列
	glm::mat4 viewMat;
	//キューブマッピング用のビュー行列、通常のビュー行列のy成分の回転のみ反転している
	glm::mat4 cubemapViewMat;
	//投資投影行列
	glm::mat4 perspectiveMat;

	Camera();

	//座標の設定
	void setPosition(glm::vec3 pos);
	//座標の取得
	glm::vec3 getPosition() { return position; }
	//追従するオブジェクトの位置を設定
	void setParentPos(glm::vec3 position);
	//球面座標上の半径の設定
	void setDistance(float distance);
	//追従するオブジェクトとの距離を設定
	void setOffsetPos(glm::vec3 offset);

	//球面座標の設定 引数は矢印キーから
	void setSpherePos(float theta, float phi);

	//視野角の設定
	void setViewAngle(float f);
	//視野角の取得
	float getViewAngle();
	//x方向の移動の取得
	float getTheta();

	//ファークリップ範囲の取得
	void getzNearFar(float& near, float& far);

	//追従するターゲットの座標を取得
	glm::vec3 getViewTarget();

	//カメラの更新処理
	void Update();
	//矢印キーからの入力を処理
	void customUpdate();
};