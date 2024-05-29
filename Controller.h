#pragma once
#include<iostream>
#include<GLFW/glfw3.h>

#include"Object.h"

extern GLFWwindow* window;

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

struct MButtonStat
{
	int button;
	int clicked;
	int mods;
};

struct MPos
{
	double x;
	double y;
};

struct MScroll
{
	double y;
};

struct KeyChar
{
	unsigned int charInfo;
};

class Controller
{
private:
	static Controller* controller;

	Controller();
	~Controller() {};

public:

	MButtonStat mButtonStat;
	MPos mPos;
	MScroll mScroll;
	KeyChar keyChar;
	
	static Controller* GetInstance()
	{
		if (!controller)
		{
			controller = new Controller();
		}

		return controller;
	}

	void FinishController()
	{
		delete controller;
		controller = nullptr;
	}

	void initInput();

	virtual void mouseButton(int button, int action, int mods);
	virtual void mousePos(double x, double y);
	virtual void mouseScroll(double x, double y);
	virtual void charFun(unsigned int charInfo);

protected:

	void setMouseButton(int button, int action, int mods);
	void setMousePos(double x, double y);
	void setMouseScroll(double x, double y);
	void setCharFun(unsigned int charInfo);

	static Controller* getThisPtr(GLFWwindow* window)
	{
		return static_cast<Controller*>(glfwGetWindowUserPointer(window));
	}

	static void mouseButtonCB(GLFWwindow* window, int button, int action, int mods)
	{
		getThisPtr(window)->setMouseButton(button, action, mods);
	}

	static void mousePosCB(GLFWwindow* window, double x, double y)
	{
		getThisPtr(window)->setMousePos(x, y);
	}

	static void mouseScrollCB(GLFWwindow* window, double x, double y)
	{
		getThisPtr(window)->setMouseScroll(x, y);
	}

	static void charFunCB(GLFWwindow* window, unsigned int charInfo)
	{
		getThisPtr(window)->setCharFun(charInfo);
	}
};