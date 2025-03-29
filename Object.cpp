#include"Scene.h"

#include"Object.h"

Object::Object()//ゲーム内に登場するオブジェクトのすべてが継承するクラス
{
	objNum = ObjNum::OBJECT;

	physicBase = std::make_unique<PhysicBase>();

	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = 0.0f;

	forward = glm::vec3{ 0,0,1 };
	right = glm::vec3{ 1,0,0 };
	
	rotate.x = rotate.getRadian(0.0f);
	rotate.y = rotate.getRadian(0.0f);
	rotate.z = rotate.getRadian(0.0f);

	childObjects.clear();
	spherePos = false;

	rotateSpeed = 0.1f;
	length = 1.0f;
}

bool Object::containTag(Tag tag)//指定されたタグを持っているかどうかを返す
{
	for (auto itr = tags.begin(); itr != tags.end(); itr++)
	{
		if (tag == (*itr))
		{
			return true;
		}
	}

	return false;
}

void Object::setLuaScript(std::string path)//行動パターン用のluaスクリプトを設定する 実行はしない
{
	luaPath = path;
	lua = luaL_newstate();
	luaL_openlibs(lua);

	registerGlueFunctions();
}

//luaスクリプトを実行するのを遅延するフレーム数を設定する
void Object::setDelayFrameCount(int delay)
{
	delayFrameCount = delay;
}

//luaから呼び出される静的関数の登録
void Object::registerGlueFunctions()
{
	lua_register(lua, "glueSetPos", glueObjectFunction::glueSetPos);
	lua_register(lua, "glueSetRotate", glueObjectFunction::glueSetRotate);
}

void Object::bindObject(Object* obj)//引数のオブジェクトを子オブジェクトとして設定する
{
	childObjects.push_back(obj);

	sendPosToChildren();
}

void Object::bindCamera(std::weak_ptr<Camera> camera)//カメラを自分に追従させる
{
	cameraObj = camera;

	sendPosToChildren();
}

glm::vec3 Object::inputMove()//キー入力から移動させる
{
	glm::vec3 moveDirec;
	auto controller = Controller::GetInstance();

	if (cameraObj.expired())//このオブジェクトがカメラを追従させている場合
	{
		//このオブジェクトの正面、右のベクトルをカメラに合わせる
		//これで、オブジェクトが視点のに合わせて進むようになる
		forward = cameraObj.lock()->forward;
		right = cameraObj.lock()->right;
	}

	if (controller->getKey(GLFW_KEY_W))
	{
		moveDirec = -forward;
	}
	else if (controller->getKey(GLFW_KEY_A))
	{
		moveDirec = -right;
	}
	else if (controller->getKey(GLFW_KEY_D))
	{
		moveDirec = right;
	}
	else if (controller->getKey(GLFW_KEY_S))
	{
		moveDirec = forward;
	}
	else
	{
		moveDirec = { 0,0,0 };
	}

	return moveDirec;
}

//luaスクリプトに自身の現在の座標、回転の値を送る
void Object::sendTransformToLua()
{
	if (lua)
	{
		lua_getglobal(lua, "position");
		lua_pushnumber(lua, position.x);
		lua_setfield(lua, -2, "x");
		lua_pushnumber(lua, position.y);
		lua_setfield(lua, -2, "y");
		lua_pushnumber(lua, position.z);
		lua_setfield(lua, -2, "z");
		lua_pop(lua, -1);

		lua_getglobal(lua, "rotate");
		lua_pushnumber(lua, rotate.x);
		lua_setfield(lua, -2, "x");
		lua_pushnumber(lua, rotate.y);
		lua_setfield(lua, -2, "y");
		lua_pushnumber(lua, rotate.z);
		lua_setfield(lua, -2, "z");
		lua_pop(lua, -1);
	}
}

//luaスクリプトから、座標、回転の値を受け取る
void Object::receiveTransformFromLua()
{
	if (lua)
	{
		float x, y, z;

		lua_getglobal(lua, "position");
		x = static_cast<float>(lua_tonumber(lua, -4));
		y = static_cast<float>(lua_tonumber(lua, -3));
		z = static_cast<float>(lua_tonumber(lua, -2));
		lua_pop(lua, -1);

		lua_getglobal(lua, "rotate");
		x = static_cast<float>(lua_tonumber(lua, -4));
		y = static_cast<float>(lua_tonumber(lua, -3));
		z = static_cast<float>(lua_tonumber(lua, -2));
		lua_pop(lua, -1);
	}
}

void Object::Update()//更新処理
{
	customUpdate();
}

void Object::setPosition(glm::vec3 pos)//座標の設定
{
	if (position == pos)
	{
		return;
	}

	position = pos;

	sendPosToChildren();//子オブジェクトがいる場合、追従させる

	uniformBufferChange = true;
}

glm::vec3 Object::getPosition()
{
	return position;
}

//モデル行列を取得
glm::mat4 Object::getTransformMatrix()
{
	return transformMatrix;
}

void Object::sendPosToChildren()//自分の移動を子オブジェクトに送り、子オブジェクトを自分の移動に追従させる
{
	for (auto itr = childObjects.begin(); itr != childObjects.end(); itr++)
	{
		if (*itr)
		{
			(*itr)->setParentPos(lastPos,position);
		}
	}

	if (!cameraObj.expired())
	{
		cameraObj.lock()->setParentPos(position);
	}
}

void Object::setParentPos(glm::vec3 lastPos,glm::vec3 currentPos)//親オブジェクトの移動を受け取り、親オブジェクトの移動に追従する
{
	this->lastPos = position;
	position += currentPos - lastPos;

	sendPosToChildren();
}

//luaの仮想マシンに、自身の座標と回転を記録する変数を作成
void Object::createTransformTable()
{
	lua_newtable(coroutine);
	lua_pushstring(coroutine, "x");
	lua_pushnumber(coroutine, position.x);
	lua_pushstring(coroutine, "y");
	lua_pushnumber(coroutine, position.y);
	lua_pushstring(coroutine,"z");
	lua_pushnumber(coroutine, position.z);
	lua_setglobal(coroutine, "Position");

	lua_newtable(coroutine);
	lua_pushstring(coroutine, "x");
	lua_pushnumber(coroutine, rotate.x);
	lua_pushstring(coroutine, "y");
	lua_pushnumber(coroutine, rotate.y);
	lua_pushstring(coroutine, "z");
	lua_pushnumber(coroutine, rotate.z);
	lua_setglobal(coroutine, "Rotate");
}

void Object::initFrameSetting()//初回フレームのみ実行、luaスクリプトを実行する
{
	if (lua)
	{
		luaL_dofile(lua, luaPath.c_str());
		coroutine = lua_newthread(lua);
		lua_getglobal(coroutine, "Update");

		createTransformTable();//lua上に変数の作成
	}
}

//前回フレームのトランスフォームを取得する
glm::vec3 Object::getLastPosition()
{
	return lastPos;
}

Rotate Object::getLastRotate()
{
	return lastRotate;
}

//一つ前のフレームの座標などのデータを設定
void Object::setLastFrameTransform()
{
	lastPos = position;
	lastRotate = rotate;
}