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

bool Object::containTag(Tag tag)//オブジェクトがそのタグを持っているかどうか
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

void Object::setLuaScript(std::string path)//luaスクリプトを設定
{
	luaPath = path;
	lua = luaL_newstate();
	luaL_openlibs(lua);

	registerGlueFunctions();
}

void Object::registerGlueFunctions()
{
	lua_register(lua, "glueSetPos", glueObjectFunction::glueSetPos);
	lua_register(lua, "glueSetRotate", glueObjectFunction::glueSetRotate);
}

void Object::bindObject(Object* obj)//オブジェクトに親子関係を設定する
{
	childObjects.push_back(obj);

	sendPosToChildren();
}

void Object::bindCamera(std::weak_ptr<Camera> camera)//カメラがそのオブジェクトを追従させる
{
	cameraObj = camera;

	sendPosToChildren();
}

glm::vec3 Object::inputMove()//キー入力から移動させる
{
	glm::vec3 moveDirec;
	auto controller = Controller::GetInstance();

	if (cameraObj.expired())
	{
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

	sendPosToChildren();//子オブジェクトがいる場合、それも更新する

	uniformBufferChange = true;
}

glm::vec3 Object::getPosition()
{
	return position;
}

glm::mat4 Object::getTransformMatrix()
{
	return transformMatrix;
}

void Object::sendPosToChildren()//子オブジェクトを追従させる
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
		cameraObj.lock()->setParentPos(lastPos,position);
	}
}

void Object::setParentPos(glm::vec3 lastPos,glm::vec3 currentPos)//親オブジェクトを追従する
{
	position += currentPos - lastPos;
}

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

void Object::initFrameSetting()//初回フレームの設定を行う
{
	if (lua)
	{
		luaL_dofile(lua, luaPath.c_str());
		coroutine = lua_newthread(lua);
		lua_getglobal(coroutine, "Update");

		createTransformTable();
	}
}

glm::vec3 Object::getLastPosition()
{
	return lastPos;
}

Rotate Object::getLastRotate()
{
	return lastRotate;
}

void Object::setLastFrameTransform()
{
	lastPos = position;
	lastRotate = rotate;
}