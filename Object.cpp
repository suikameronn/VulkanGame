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
}

void Object::registerGlueFunctions()
{

}

void Object::bindObject(Object* obj)//オブジェクトに親子関係を設定する
{
	parentPos = obj->getPosition();
	childObjects.push_back(obj);

	sendPosToChildren(position);
}

void Object::bindCamera(std::weak_ptr<Camera> camera)//カメラがそのオブジェクトを追従させる
{
	cameraObj = camera;

	sendPosToChildren(position);
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

	sendPosToChildren(position);//子オブジェクトがいる場合、それも更新する

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

void Object::sendPosToChildren(glm::vec3 pos)//子オブジェクトを追従させる
{
	for (auto itr = childObjects.begin(); itr != childObjects.end(); itr++)
	{
		if (*itr)
		{
			(*itr)->setParentPos(pos);
		}
	}

	if (!cameraObj.expired())
	{
		cameraObj.lock()->setParentPos(pos);
	}
}

void Object::setParentPos(glm::vec3 pos)//親オブジェクトを追従する
{
	position += pos - parentPos;

	parentPos = pos;
}

void Object::initFrameSetting()//初回フレームの設定を行う
{
	if (lua)
	{
		lua_pushlightuserdata(lua, this);
		lua_setglobal(lua, "Data");

		luaL_dofile(lua, luaPath.c_str());
	}
}