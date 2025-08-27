#pragma once

#include<vector>

#include"GLFW/glfw3.h"

extern GLFWwindow* window;

//キー入力用のenum
enum class Key
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

struct KeyInput
{
	std::vector<int> key;
	std::vector<int> scancode;
	std::vector<int> action;
	std::vector<int> mods;
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

//コントローラの入力を受け取る
class InputSystem
{
public:

	InputSystem();
	~InputSystem() {};

	int getKey(int key);
	void getMousePos(double& x, double& y);
	int getMouseButton(int mouseButton);
};