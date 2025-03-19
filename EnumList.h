#pragma once

//gltfモデルの種類を指定する
enum class GLTFOBJECT
{
	gltfTEST = 0,
	ASPHALT,
	LEATHER,
	CUBEMAP
};

//コライダーの形状
enum COLIDER
{
	BOX
};

//オブジェクトのタイプを示す
enum ObjNum
{
	OBJECT = 0,
	MODEL,
	PLAYYER,
	POINTLIGHT,
	DIRECTIONALLIGHT
};

//オブジェクトの性質を示す
enum Tag
{
	NONE = 0,
	GROUND
};