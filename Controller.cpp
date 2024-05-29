#include"Controller.h"

Controller* Controller::controller = nullptr;

Controller::Controller()
{
	glfwSetWindowUserPointer(window, this);

	glfwSetMouseButtonCallback(window, mouseButtonCB);
	glfwSetCursorPosCallback(window, mousePosCB);
	glfwSetScrollCallback(window, mouseScrollCB);

	glfwSetCharCallback(window, charFunCB);
}

void Controller::initInput()
{
	keyChar.charInfo = 128;
}

//button �ǂ����̃{�^���������ꂽ�� 0:�� 1:�E
//action �����ꂽ���A�b���ꂽ�� 0:���� 1:����
//�ǉ��I�v�V����
void Controller::mouseButton(int button, int action, int mods)
{
#ifdef _DEBUG
	std::cout << "mouseButton" << " " << button << " " << action << " " << mods << std::endl;
#endif

	mButtonStat.button = button;
	mButtonStat.clicked = action;
	mButtonStat.mods = mods;
}

void Controller::mousePos(double x, double y)
{
#ifdef _DEBUG
	std::cout << "mousePos" << " " << x << " " << y << std::endl;
#endif

	mPos.x = x;
	mPos.y = y;
}

void Controller::mouseScroll(double x, double y)
{
#ifdef _DEBUG
	std::cout << "mouseScroll" << " " << x << " " << y << std::endl;
#endif

	mScroll.y = y;
}

void Controller::charFun(unsigned int charInfo)
{
#ifdef _DEBUG
	std::cout << "charFun" << " " << charInfo << std::endl;
#endif

	keyChar.charInfo = charInfo;
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

void Controller::setCharFun(unsigned int charInfo)
{
	charFun(charInfo);
}