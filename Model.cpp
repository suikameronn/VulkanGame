#include"Scene.h"
#include"VulkanBase.h"

#include"Model.h"

Model::Model()//3Dモデルを持つクラス
{
	scene = Scene::GetInstance();

	objNum = ObjNum::MODEL;
	tags.push_back(Tag::GROUND);

	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = 0.0f;

	rotate.x = rotate.getRadian(0.0f);
	rotate.y = rotate.getRadian(0.0f);
	rotate.z = rotate.getRadian(0.0f);
	scale = glm::vec3(1.0f, 1.0f, 1.0f) * 100.0f;

	forward = glm::vec3{ 0,0,1 };
	right = glm::vec3{ 1,0,0 };
	up = glm::vec3{ 0,1,0 };

	childObjects.clear();
	spherePos = false;

	rotateSpeed = 0.1f;

	deltaTime = 0.0;
	animationChange = true;

	isMovable = false;
	colider = nullptr;

	gravity = 0.0f;

	defaultAnimationName = "Idle";
}

Model::Model(std::string luaScriptPath)
{
	tags.push_back(Tag::GROUND);
	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = 0.0f;

	scale = glm::vec3(1.0f, 1.0f, 1.0f);

	forward = glm::vec3{ 0,0,1 };
	right = glm::vec3{ 1,0,0 };

	rotate.x = rotate.getRadian(0.0f);
	rotate.y = rotate.getRadian(0.0f);
	rotate.z = rotate.getRadian(0.0f);

	childObjects.clear();
	spherePos = false;

	rotateSpeed = 0.1f;

	deltaTime = 0.0;

	isMovable = false;
	colider = nullptr;

	gravity = 0.0f;

	defaultAnimationName = "none";
	currentPlayAnimationName = "none";
}

void Model::registerGlueFunctions()//glue関数を設定する
{
	lua_register(lua, "glueSetPos", glueObjectFunction::glueSetPos);
	lua_register(lua, "glueSetRotate", glueObjectFunction::glueSetRotate);
	lua_register(lua, "glueSetScale", glueModelFunction::glueSetScale);
}

void Model::cleanupVulkan()//Vulkanの変数の後処理
{
	VkDevice device = VulkanBase::GetInstance()->GetDevice();

	for (size_t i = 0; i < pointBuffers.size(); i++)//頂点用バッファの解放
	{
		vkDestroyBuffer(device, pointBuffers[i].vertBuffer, nullptr);
		vkFreeMemory(device, pointBuffers[i].vertHandler, nullptr);

		vkDestroyBuffer(device, pointBuffers[i].indeBuffer, nullptr);
		vkFreeMemory(device, pointBuffers[i].indeHandler, nullptr);
	}

	//uniform bufferの解放
	vkDestroyBuffer(device, modelViewMappedBuffer.uniformBuffer, nullptr);
	vkFreeMemory(device, modelViewMappedBuffer.uniformBufferMemory, nullptr);
	modelViewMappedBuffer.uniformBufferMapped = nullptr;

	//アニメーションのユニフォームバッファの解放
	for (int i = 0; i < (int)animationMappedBuffers.size(); i++)
	{
		vkDestroyBuffer(device, animationMappedBuffers[i].uniformBuffer, nullptr);
		vkFreeMemory(device, animationMappedBuffers[i].uniformBufferMemory, nullptr);
		animationMappedBuffers[i].uniformBufferMapped = nullptr;
	}

	if (colider)
	{
		colider->cleanupVulkan();//コライダーの変数の後処理
	}
}

MappedBuffer* Model::getAnimationMappedBufferData()
{
	return animationMappedBuffers.data();
}

void Model::setDefaultAnimationName(std::string name)//初期状態で再生するアニメーションを設定する
{
	defaultAnimationName = name;
	currentPlayAnimationName = name;
}

void Model::setgltfModel(std::shared_ptr<GltfModel> model)//gltfモデルを設定する
{
	gltfModel = model;

	min = gltfModel->initPoseMin;
	max = gltfModel->initPoseMax;

	//RTree::GetInstance()->insert(shared_from_this(),min,max);

	animationNames.resize(model->animations.size());
	int i = 0;
	for (auto itr = model->animations.begin(); itr != model->animations.end(); itr++)
	{
		animationNames[i] = itr->first;
		i++;
	}

	//バッファを用意する
	descSetDatas.resize(model->primitiveCount);
	jointMatrices.resize(model->jointNum);
	pointBuffers.resize(model->meshCount);
	animationMappedBuffers.resize(model->primitiveCount);
}

//再生するアニメーションをアイドル時に再生するものにする
void Model::switchPlayAnimation()
{
	currentPlayAnimationName = defaultAnimationName;
}

//アニメーションを切り替える
void Model::switchPlayAnimation(std::string nextAnimation)
{
	nextPlayAnimationName = nextAnimation;
}

void Model::playAnimation()//アニメーション用の行列を計算する
{
	if (currentPlayAnimationName != "none")
	{
		//アニメーションを再生し終えた
		//あるいは、アニメーションが切り替わった場合
		if (deltaTime > gltfModel->animationDuration(currentPlayAnimationName)
			|| currentPlayAnimationName != nextPlayAnimationName)
		{
			currentPlayAnimationName = nextPlayAnimationName;
			//再生時間を再び計測し始める
			animationChange = false;
			startTime = clock();
		}


		currentTime = clock();

		deltaTime = static_cast<double>(currentTime - startTime) / CLOCKS_PER_SEC;

		//アニメーション行列の計算
		gltfModel->updateAnimation(deltaTime, gltfModel->animations[currentPlayAnimationName], jointMatrices);
	}
}

std::array<glm::mat4, 128>& Model::getJointMatrices(int index)
{
	return this->jointMatrices[index];
}


void Model::setPosition(glm::vec3 pos)
{
	if (position == pos)
	{
		return;
	}

	lastPos = position;
	position = pos;

	sendPosToChildren();//子オブジェクトに移動を伝える

	uniformBufferChange = true;
}

void Model::setBaseColor(glm::vec4 baseColor)
{

}

void Model::updateTransformMatrix()//座標変換行列を計算する
{
	transformMatrix = glm::translate(glm::mat4(1.0), position) * rotate.getRotateMatrix() * glm::scale(glm::mat4(1.0f),scale);

	//AABBの更新
	min = transformMatrix * glm::vec4(initMin, 1.0f);
	max = transformMatrix * glm::vec4(initMax, 1.0f);

	//MBRの更新
	calcMBR();

	if (colider)
	{
		if (colider->isConvex)
		{
			colider->reflectMovement(transformMatrix, jointMatrices);//コライダーにオブジェクトのトランスフォームの変更を反映させる
		}
		else
		{
			colider->reflectMovement(transformMatrix);
		}
	}

	uniformBufferChange = false;

	//Rツリー上のオブジェクトの位置を更新する
	scene->updateObjectPos(this, rNode);
}

//コライダー用のAABBからMBRを計算
void Model::calcMBR()
{
	for (int i = 0; i < 3; i++)
	{
		mbrMin[i] = std::min(min[i], mbrMin[i]);
		mbrMin[i] = std::min(max[i], mbrMin[i]);
	}

	for (int i = 0; i < 3; i++)
	{
		mbrMax[i] = std::max(min[i], mbrMax[i]);
		mbrMax[i] = std::max(max[i], mbrMax[i]);
	}
}

//コライダーの設定
void Model::setColider(bool isConvex)
{
	if (isConvex)
	{
		colider = std::shared_ptr<Colider>(new Colider(gltfModel));
	}
	else
	{
		colider = std::shared_ptr<Colider>(new Colider(gltfModel->initPoseMin, gltfModel->initPoseMax));
	}
}

bool Model::hasColider()
{
	if (colider)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//カメラと子オブジェクト以外にも、自分の真上に載っているオブジェクトも追従させる
void Model::sendPosToChildren()
{
	for (auto itr = childObjects.begin(); itr != childObjects.end(); itr++)
	{
		(*itr)->setParentPos(lastPos,position);
	}

	for (auto itr = groundingObjects.begin(); itr != groundingObjects.end(); itr++)
	{
		(*itr)->setParentPos(lastPos, position);
	}

	if (!cameraObj.expired())
	{
		cameraObj.lock()->setParentPos(position);
	}
}

void Model::createTransformTable()//luaに座標変換用の行列の変数を作成
{
	lua_newtable(coroutine);
	lua_pushstring(coroutine, "x");
	lua_pushnumber(coroutine, position.x);
	lua_settable(coroutine, -3);
	lua_pushstring(coroutine, "y");
	lua_pushnumber(coroutine, position.y);
	lua_settable(coroutine, -3);
	lua_pushstring(coroutine, "z");
	lua_pushnumber(coroutine, position.z);
	lua_settable(coroutine, -3);
	lua_setglobal(coroutine, "Position");

	lua_newtable(coroutine);
	lua_pushstring(coroutine, "x");
	lua_pushnumber(coroutine, rotate.x);
	lua_settable(coroutine, -3);
	lua_pushstring(coroutine, "y");
	lua_pushnumber(coroutine, rotate.y);
	lua_settable(coroutine, -3);
	lua_pushstring(coroutine, "z");
	lua_pushnumber(coroutine, rotate.z);
	lua_settable(coroutine, -3);
	lua_setglobal(coroutine, "Rotate");

	lua_newtable(coroutine);
	lua_pushstring(coroutine, "x");
	lua_pushnumber(coroutine, scale.x);
	lua_settable(coroutine, -3);
	lua_pushstring(coroutine, "y");
	lua_pushnumber(coroutine, scale.y);
	lua_settable(coroutine, -3);
	lua_pushstring(coroutine, "z");
	lua_pushnumber(coroutine, scale.z);
	lua_settable(coroutine, -3);
	lua_setglobal(coroutine, "Scale");
}

void Model::sendTransformToLua()//luaに座標を送る
{
	if (coroutine)
	{
		lua_getglobal(coroutine, "Position");
		lua_pushnumber(coroutine, position.x);
		lua_setfield(coroutine, -2, "x");
		lua_pushnumber(coroutine, position.y);
		lua_setfield(coroutine, -2, "y");
		lua_pushnumber(coroutine, position.z);
		lua_setfield(coroutine, -2, "z");

		lua_getglobal(coroutine, "Rotate");
		lua_pushnumber(coroutine, rotate.x);
		lua_setfield(coroutine, -2, "x");
		lua_pushnumber(coroutine, rotate.y);
		lua_setfield(coroutine, -2, "y");
		lua_pushnumber(coroutine, rotate.z);
		lua_setfield(coroutine, -2, "z");

		lua_getglobal(coroutine, "Scale");
		lua_pushnumber(coroutine, scale.x);
		lua_setfield(coroutine, -2, "x");
		lua_pushnumber(coroutine, scale.y);
		lua_setfield(coroutine, -2, "y");
		lua_pushnumber(coroutine, scale.z);
		lua_setfield(coroutine, -2, "z");
	}
}

void Model::receiveTransformFromLua()//スクリプトで変化した座標などを取得
{
	if (coroutine)
	{
		float x, y, z;

		lua_getglobal(coroutine, "Position");
		lua_pushstring(coroutine, "x");
		lua_gettable(coroutine, -2);
		x = static_cast<float>(lua_tonumber(coroutine,-1));
		lua_pushstring(coroutine, "y");
		lua_gettable(coroutine, -3);
		y = static_cast<float>(lua_tonumber(coroutine,-1));
		lua_pushstring(coroutine, "z");
		lua_gettable(coroutine, -4);
		z = static_cast<float>(lua_tonumber(coroutine,-1));
		lua_pop(lua, -1);
		setPosition(glm::vec3(x, y, z));

		lua_getglobal(coroutine, "Rotate");
		lua_pushstring(coroutine, "x");
		lua_gettable(coroutine, -2);
		x = static_cast<float>(lua_tonumber(coroutine,-1));
		lua_pushstring(coroutine, "y");
		lua_gettable(coroutine, -3);
		y = static_cast<float>(lua_tonumber(coroutine,-1));
		lua_pushstring(coroutine, "z");
		lua_gettable(coroutine, -4);
		z = static_cast<float>(lua_tonumber(coroutine,-1));
		lua_pop(coroutine, -1);
		rotate.x = x;
		rotate.y = y;
		rotate.z = z;

		lua_getglobal(coroutine, "Scale");
		lua_pushstring(coroutine, "x");
		lua_gettable(coroutine, -2);
		x = static_cast<float>(lua_tonumber(coroutine, -1));
		lua_pushstring(coroutine, "y");
		lua_gettable(coroutine, -3);
		y = static_cast<float>(lua_tonumber(coroutine, -1));
		lua_pushstring(coroutine, "z");
		lua_gettable(coroutine, -4);
		z = static_cast<float>(lua_tonumber(coroutine, -1));
		lua_pop(coroutine, -1);
		scale = glm::vec3(x, y, z);

		uniformBufferChange = true;//座標変換行列の更新
	}
}

glm::vec3 Model::getLastScale()
{
	return lastScale;
}

void Model::setLastFrameTransform()
{
	lastPos = position;
	lastRotate = rotate;
	lastScale = scale;
}

void Model::Update()
{
	setLastFrameTransform();

	customUpdate();//オブジェクト固有の更新処理

	if (physicBase)
	{
		physicBase->Update();//物理演算の更新

		setPosition(getPosition() + physicBase->getVelocity());//物理演算の位置を加える
	}

	if (coroutine && passFrameCount >= delayFrameCount)
	{
		sendTransformToLua();//luaに座標などを送る
		
		int nresult;
		lua_resume(coroutine, nullptr, 0, &nresult);//luaのコルーチンの再開

		/*luaスクリプト上で更新されたトランスフォームをc++上にも反映させる*/

		receiveTransformFromLua();//luaから座標などを受け取る
	}

	playAnimation();//アニメーションの再生

	if (passFrameCount < MAXFRAMECOUNT)
	{
		passFrameCount++;
	}
}

void Model::customUpdate()
{

}

void Model::initFrameSetting()//初回フレームの処理
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
		switchPlayAnimation();
	}

	if (colider)
	{
		colider->initFrameSettings();//コライダーの初期設定
	}

	//AABBにスケールを適用する
	min = glm::scale(scale) * glm::vec4(min,1.0f);
	max = glm::scale(scale) * glm::vec4(max,1.0f);

	initMin = min;
	initMax = max;

	calcMBR();

	//シーン全体のR-treeにこのオブジェクトを追加

	/////shared_from_thisでなぜかエラーがでる
	scene->addModelToRTree(this);
}

//ボックスレイキャスト、引数のmaxLengthまで指定の方向に直方体を伸ばして、コライダーとの当たり判定を行う
std::shared_ptr<Model> Model::rayCast(glm::vec3 origin,glm::vec3 dir,float maxLength,glm::vec3& normal)
{
	for (float i = 0.1f; i <= maxLength; i += 0.1f)
	{
		std::shared_ptr<Model> hitModel = scene->raycast(origin,dir,i,this,normal);
		if (hitModel)
		{
			return hitModel;
		}
	}

	return nullptr;
}

//下のほうにあるオブジェクトが床かどうか調べる
bool Model::isGround(glm::vec3& normal)
{
	std::shared_ptr<Model> model = rayCast(position + up * 1.0f, glm::vec3(0.0f, -1.0, 0.0f), 2.0f,normal);
	if (model)
	{
		if (model->containTag(Tag::GROUND))
		{
			return true;
		}
	}

	return false;
}

//当たり判定の結果、真上にいたオブジェクトを自分の移動に追従させるため、配列に追加する
//ただし、この配列は毎フレーム初期化される
void Model::addGroundingObject(Model* object)
{
	groundingObjects.push_back(object);
}

//真上に載っているオブジェクトの配列の初期化
void Model::clearGroundingObject()
{
	groundingObjects.clear();
}

//速度の停止
void Model::setZeroVelocity()
{
	if (physicBase)
	{
		physicBase->setZeroVelocity();
	}
}

//重力の打ち消し
void Model::cancelGravity()
{
	if (physicBase)
	{
		physicBase->cancelGravity();
	}
}