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

	//弾の向きを進行方向に合わせる
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

	//度に変換しておく
	rotate.x = pitch * (180.0f / glm::pi<float>());
	rotate.y = yaw * (180.0f / glm::pi<float>());
	rotate.z = roll * (180.0f / glm::pi<float>());
}

Bullet::~Bullet()
{
	VulkanBase* vulkan = VulkanBase::GetInstance();

	vulkan->addDefferedDestructBuffer(ray.mappedBuffer);
}

//初回フレームの設定
void Bullet::initFrameSetting()
{
	if (lua)
	{
		luaL_dofile(lua, luaPath.c_str());
		coroutine = lua_newthread(lua);

		createTransformTable();//オブジェクトのトランスフォームをluaに送る

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

	//アニメーション行列の初期化
	for (auto& matrices : jointMatrices)
	{
		std::fill(matrices.begin(), matrices.end(), glm::mat4(1.0f));
	}

	updateTransformMatrix();

	//シーン全体のR-treeにこのオブジェクトを追加

	/////shared_from_thisでなぜかエラーがでる
	scene->addModelToRTree(std::dynamic_pointer_cast<Model>(shared_from_this()));

	//レイキャスト時のレイの設定
	glm::vec3 d = mbrMax - mbrMin;
	float a = glm::dot(mbrMax - mbrMin, ray.direction);
	ray.length = abs(glm::dot(mbrMax - mbrMin, ray.direction));
	ray.origin = pivot - ray.direction * (ray.length / 2.0f);

	//gpuレイキャスト時のバッファを作成
	VulkanBase::GetInstance()->createUniformBuffer(&ray.mappedBuffer, ray.getSize());

	//gpu上に値をコピー
	ray.copyToGpuBuffer();
}

void Bullet::Update()
{
	if (abs(glm::length(shootPos - getPosition())) >= distanceLimit)
	{
		//弾丸のオブジェクトを削除する
		exist = false;
	}

	setLastFrameTransform();

	setPosition(getPosition() + ray.direction * speed);
}

void Bullet::updateTransformMatrix()//座標変換行列を計算する
{
	transformMatrix = glm::translate(glm::mat4(1.0), position)
		* rotate.getRotateMatrix() * glm::scale(glm::mat4(1.0f), initScale * scale);

	//AABBの更新
	min = transformMatrix * glm::vec4(initMin, 1.0f);
	max = transformMatrix * glm::vec4(initMax, 1.0f);

	//MBRの更新
	calcMBR();

	pivot = (mbrMin + mbrMax) / 2.0f;

	//レイの始点の更新
	ray.origin = pivot - ray.direction * (ray.length / 2.0f);
	//レイの情報の更新
	ray.copyToGpuBuffer();

	if (colider)
	{
		colider->reflectMovement(transformMatrix);
	}

	uniformBufferChange = false;

	if (rNode)
	{
		//Rツリー上のオブジェクトの位置を更新する
		scene->updateObjectPos(std::dynamic_pointer_cast<Model>(shared_from_this()), rNode);
	}
}

void Bullet::collision(std::shared_ptr<Model> model)
{
	if (model->getObjNum() == ObjNum::PLAYER || model->getObjNum() == ObjNum::BULLET)
	{
		return;
	}

	//レイキャスト時に衝突するメッシュまでの距離とメッシュが所属するノードへのポインタ
	float distance;
	GltfNode* node;

	VulkanBase::GetInstance()->startRaycast(ray, model, distance, &node);

	if (node)
	{
		//衝突した二つのモデルを消す
		model->notExist();
		this->notExist();
	}
}