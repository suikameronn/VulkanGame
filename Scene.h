#pragma once

#include <iostream>
#include <vector>
#include<unordered_map>
#include<string>

#include<glm/glm.hpp>

#include"Trimesh.h"
#include"Camera.h"
#include"DirLight.h"

class Scene
{
private:

	std::unordered_map<std::string, std::unique_ptr<Trimesh>> triMeshs;
	std::unique_ptr<Camera> camera;
	std::unordered_map<std::string, std::unique_ptr<DirLight>> dirLights;

public:
	Scene();

	void addObject(std::string* name, std::unique_ptr<Trimesh> trimesh);
	void addObject(std::string* name, std::unique_ptr<DirLight> d);
	Trimesh* accessObj(std::string* name);
	DirLight* accessLight(std::string* name);
};

class SceneElement
{
public:
	virtual ~SceneElement();

protected:

	SceneElement(Scene* s);
};

class Geometry:public SceneElement
{
public:
	Geometry(Scene* scene) :SceneElement(scene) {};
};

class SceneObject : public Geometry
{
	//virtual const Material& getMaterial() = 0;
	//virtual void setMaterial(Material& m) = 0;

protected:
	SceneObject(Scene* scene) :Geometry(scene) {};
};

class MaterialSceneObject :public SceneObject
{
protected:
	//MateriSceneObject(Scene* scene)
	; SceneObject(scene);
};