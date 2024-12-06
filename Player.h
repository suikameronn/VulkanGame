#include "Model.h"

class Player : public Model
{
	float moveSpeed;

	glm::vec3 inputMove();

public:

	void Update() override;
};