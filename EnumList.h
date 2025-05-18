#pragma once

//コライダーの形状
enum COLIDER
{
	BOX
};

//オブジェクトのタイプを示す
enum ObjNum
{
	OBJECT = 0,
	MODEL = 1,
	PLAYER,
	BULLET,
	POINTLIGHT = 101,
	DIRECTIONALLIGHT
};

#define TYPEOBJECT 0
#define TYPEMODEL 1
#define TYPELIGHT 101

//オブジェクトの性質を示す
enum Tag
{
	NONE = 0,
	GROUND
};