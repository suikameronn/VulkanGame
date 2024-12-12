#include "Model.h"

class Player : public Model
{
	float moveSpeed;
	bool controllable;

	glm::vec3 inputMove();

public:

	Player();
	Player(std::string luaScriptPath);

	void customUpdate() override;
};