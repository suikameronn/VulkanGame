#include "Model.h"
#include"Camera.h"

class Player : public Model
{
	bool isGround;

	float cameraDirDeg;

	float moveSpeed;
	bool controllable;

	glm::vec3 inputMove();

public:

	Player();
	Player(std::string luaScriptPath);

	void customUpdate() override;
};