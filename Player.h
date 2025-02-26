#include "Model.h"
#include"Camera.h"

class Player : public Model
{
	float cameraDirDeg;

	float moveSpeed;
	float jumpHeight,maxJumpHeight;
	bool controllable;

	glm::vec3 inputMove();

public:

	Player();
	Player(std::string luaScriptPath);

	void registerGlueFunctions() override;

	void setSpeed(float s);
	void setMaxJumpHeight(float height);

	void initFrameSetting() override;

	void customUpdate() override;

	void restart(glm::vec3 startPoint);
};

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