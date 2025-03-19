#include"Camera.h"

Camera::Camera()
{
	position = { 0,0.0f,5.0f };
	distance = 100.0f;
	offsetPos = glm::vec3(0.0f,20.0f,0.0f);

	forward = glm::vec3{ 0,0,1};
	right = glm::vec3{ 1,0,0};
	up = glm::vec3{ 0.0f,1.0f,0.0f };

	theta = 0.0f;
	phi = 0.0f;

	viewAngle = 45.0f;
	aspect = 900.0f / 600.0f;
	viewPointSpeed = 1.0f;

	setPosition(glm::vec3(0, 0, distance));

	zNear = 0.1f;
	zFar = 1000.0f;

	perspectiveMat = glm::perspective(viewAngle, aspect, zNear,zFar);
}

//座標の設定
void Camera::setPosition(glm::vec3 pos)
{
	if (position == pos)
	{
		return;
	}

	position = pos;
}

//球面座標上の半径の設定
void Camera::setDistance(float distance)
{
	this->distance = distance;
}

//追従するオブジェクトとの距離を設定
void Camera::setOffsetPos(glm::vec3 offset)
{
	this->offsetPos = offset;
}

//視野角の設定
void Camera::setViewAngle(float f)
{
	viewAngle = f;

	perspectiveMat = glm::perspective(viewAngle, aspect, zNear, zFar);
}

//視野角の取得
float Camera::getViewAngle()
{
	return viewAngle;
}

//カメラの更新処理
void Camera::Update()
{
	customUpdate();
}

//矢印キーからの入力を処理
void Camera::customUpdate()
{
	auto controller = Controller::GetInstance();

	//矢印キーから移動する角度を設定 単位は度数法
	if (controller->getKey(GLFW_KEY_LEFT) != GLFW_RELEASE)
	{
		theta -= viewPointSpeed;
	}

	if (controller->getKey(GLFW_KEY_RIGHT) != GLFW_RELEASE)
	{
		theta += viewPointSpeed;
	}

	if (controller->getKey(GLFW_KEY_UP) != GLFW_RELEASE)
	{
		phi += viewPointSpeed;
	}

	if (controller->getKey(GLFW_KEY_DOWN) != GLFW_RELEASE)
	{
		phi -= viewPointSpeed;
	}

	//回転角度の制限
	if (phi >= 90.0f)
	{
		phi = 89.0f;
	}
	else if (phi <= -90.0f)
	{
		phi = -89.0f;
	}

	//球面座標上の座標取得
	setSpherePos(glm::radians(theta), glm::radians(-phi));
	//ビュー行列の計算
	calcViewMat();
}

//追従するオブジェクトの位置を設定
void Camera::setParentPos(glm::vec3 position)
{
	parentPos = position;
}

//球面座標の設定 引数は矢印キーから
void Camera::setSpherePos(float theta, float phi)
{
	glm::vec3 pos;
	pos = { distance * cos(theta) * cos(phi),distance * sin(phi),distance * sin(theta) * cos(phi) };
	pos += parentPos + offsetPos;

	//カメラの回転に合わせて、カメラの正面と右のベクトルも計算
	this->forward = glm::normalize(glm::vec3(pos - parentPos));
	this->right = glm::cross(glm::vec3(0, 1, 0), this->forward);

	setPosition(pos);
}

//ビュー行列の計算、キューブマッピング用の行列も同時に計算
void Camera::calcViewMat()
{
	//追従するオブジェクトの座標の中心あたりを設定
	glm::vec3 pos = glm::vec3(parentPos.x, parentPos.y + 20.0f, parentPos.z);

	viewMat = glm::lookAt(this->position, pos, up);

	//キューブマップの立方体の回転をカメラの動きと合わせるため
	//x,y成分の回転のみ反転させる
	cubemapViewMat = glm::lookAt(this->position * glm::vec3(-1.0f,1.0f,1.0f), pos * glm::vec3(-1.0f, 1.0f, 1.0f), -up);
}

//追従するターゲットの座標を取得
glm::vec3 Camera::getViewTarget()
{
	return parentPos;
}

//x方向の移動の取得
float Camera::getTheta()
{
	return theta;
}

//ファークリップ範囲の取得
void Camera::getzNearFar(float& near, float& far)
{
	near = zNear;
	far = zFar;
}