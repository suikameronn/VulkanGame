#include"Controller.h"

enum
{
	a = 97,
	b,
	c,
	d,
	e,
	f,
	g,
	h,
	i,
	j,
	k,
	l,
	m,
	n,
	o,
	p,
	q,
	r,
	s,
	t,
	u,
	v,
	w,
	x,
	y,
	z
};

Controller::Controller(GLFWwindow* window)
{
	glfwSetWindowUserPointer(window, this);

	glfwSetMouseButtonCallback(window, mouseButtonCB);
	glfwSetCursorPosCallback(window, mousePosCB);
	glfwSetScrollCallback(window, mouseScrollCB);

	glfwSetKeyCallback(window, keyFunCB);
	glfwSetCharCallback(window, charFunCB);
}

//button どっちのボタンが押されたか 0:左 1:右
//action 押されたか、話されたか 0:離す 1:押す
//追加オプション
void Controller::mouseButton(int button, int action, int mods)
{
	//std::cout << "mouseButton" << " " << button << " " << action << " " << mods << std::endl;
	
}

void Controller::mousePos(double x, double y)
{
	std::cout << "mousePos" << " " << x << " " << y << std::endl;
}

void Controller::mouseScroll(double x, double y)
{
	std::cout << "mouseScroll" << " " << x << " " << y << std::endl;
}

void Controller::keyFun(int key, int scancode, int action, int mods)
{
	std::cout << "keyFun" << " " << key << " " << scancode << " " << action << " " << mods << std::endl;
}

void Controller::charFun(unsigned int charInfo)
{
	std::cout << "charFun" << " " << charInfo << std::endl;
}

void Controller::setMouseButton(int button, int action, int mods)
{
	mouseButton(button, action, mods);
}

void Controller::setMousePos(double x, double y)
{
	mousePos(x, y);
}

void Controller::setMouseScroll(double x, double y)
{
	mouseScroll(x, y);
}

void Controller::setKeyFun(int key, int scancode, int action, int mods)
{
	keyFun(key, scancode, action, mods);
}

void Controller::setCharFun(unsigned int charInfo)
{
	charFun(charInfo);
}