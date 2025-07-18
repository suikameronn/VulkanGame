#include"Scene.h"
#include"VulkanBase.h"

#include"Bullet.h"

Bullet::Bullet(float s, float length, glm::vec3 dir, glm::vec3 pos, float limit)
{
	objNum = ObjNum::BULLET;

	speed = 5.0f;

	position = pos;
	shootPos = pos;

	distanceLimit = limit;

	ray.direction = glm::normalize(dir);

	//弾の3Dモデルの向きを計算する
	calculateBulletAngle();

	updateTransformMatrix();
}

void Bullet::calculateBulletAngle()
{
	//y軸
	float yaw = atan2(ray.direction.z, ray.direction.x);
	//x軸
	float pitch = asin(ray.direction.y);
	//z軸は回転させない
	float roll = 0.0f;

	//3Dモデルの回転角度を計算
	rotate.x = pitch * (180.0f / glm::pi<float>());
	rotate.y = yaw * (180.0f / glm::pi<float>());
	rotate.z = roll * (180.0f / glm::pi<float>());
}

Bullet::~Bullet()
{
	VulkanBase* vulkan = VulkanBase::GetInstance();

	vulkan->addDefferedDestructBuffer(ray.mappedBuffer);
}

//初期フレームに、レイキャスト時のレイの長さなどを計算
void Bullet::initFrameSetting()
{
	initFrame = false;

	if (lua)
	{
		luaL_dofile(lua, luaPath.c_str());
		coroutine = lua_newthread(lua);

		createTransformTable();//luaに座標変換用の行列の変数を作成

		lua_getglobal(coroutine, "Update");

		int nresults;
		if (lua_resume(coroutine, nullptr, 0, &nresults) != LUA_YIELD)
		{
			std::cerr << "lua coroutine error" << std::endl;
		}
	}

	if (defaultAnimationName != "none")
	{
		//switchPlayAnimation();
		currentPlayAnimationName = defaultAnimationName;
		nextPlayAnimationName = defaultAnimationName;
	}

	if (hasColiderFlag)
	{
		colider = std::shared_ptr<Colider>(new Colider(gltfModel, trigger));
		colider->initFrameSettings(glm::vec3(1.0f));
	}

	initMin = min;
	initMax = max;

	//アニメーション行列の配列を初期化
	for (auto& matrices : jointMatrices)
	{
		std::fill(matrices.begin(), matrices.end(), glm::mat4(1.0f));
	}

	updateTransformMatrix();

	//R木にこのインスタンスを追加
	scene->addModelToRTree(std::dynamic_pointer_cast<Model>(shared_from_this()));

	//レイキャスト時のレイの長さとレイの始点を設定
	ray.length = abs(glm::dot(mbrMax - mbrMin, ray.direction));
	ray.origin = pivot - ray.direction * (ray.length / 2.0f);

	//gpu上にバッファを作成
	VulkanBase::GetInstance()->createUniformBuffer(&ray.mappedBuffer, ray.getSize());

	//gpuにレイのデータをコピーする
	ray.copyToGpuBuffer();

	//gpu上にバッファを作成
	VulkanBase::GetInstance()->setModelData(std::static_pointer_cast<Model>(shared_from_this()));
}

void Bullet::Update()
{
	if (abs(glm::length(shootPos - getPosition())) >= distanceLimit)
	{
		//この場合、弾を削除予定にする
		exist = false;
	}

	setLastFrameTransform();

	setPosition(getPosition() + ray.direction * speed);
}

void Bullet::updateTransformMatrix()//座標変換行列を更新
{
	transformMatrix = glm::translate(glm::mat4(1.0), position)
		* rotate.getRotateMatrix() * glm::scale(glm::mat4(1.0f), initScale * scale);

	//AABBを更新
	min = transformMatrix * glm::vec4(initMin, 1.0f);
	max = transformMatrix * glm::vec4(initMax, 1.0f);

	//MBRを更新
	calcMBR();

	pivot = (mbrMin + mbrMax) / 2.0f;

	//レイの始点を更新
	ray.origin = pivot - ray.direction * (ray.length / 2.0f);
	//gpuにレイのデータをコピーする
	ray.copyToGpuBuffer();

	if (colider)
	{
		colider->reflectMovement(transformMatrix);
	}

	uniformBufferChange = false;

	if (rNode)
	{
		//R木を更新
		scene->updateObjectPos(std::dynamic_pointer_cast<Model>(shared_from_this()), rNode);
	}
}

void Bullet::collision(std::shared_ptr<Model> model)
{
	if (model->getObjNum() == ObjNum::PLAYER || model->getObjNum() == ObjNum::BULLET)
	{
		return;
	}

	//レイキャスト時に、レイがポリゴンにヒットした場合
	//以下の変数に、衝突したポリゴンとレイの距離と当たったポリゴンのGltfNodeが返ってくる
	float distance;
	glm::vec3 faceNormal;
	GltfNode* node;

	VulkanBase::GetInstance()->startRaycast(ray, model, distance, faceNormal, &node);

	if (node)
	{
		//このインスタンスを削除予定にする
		model->notExist();
		this->notExist();
	}
}