#pragma once

#include <iostream>
#include <vector>
#include<unordered_map>
#include<string>

#include<glm/glm.hpp>

#include"TriMeshs.h"
#include"Camera.h"
#include"DirLight.h"

class Scene
{
private:

	std::unordered_map<std::string, std::unique_ptr<TriMeshs>> TriMeshss;
	std::unique_ptr<Camera> camera;
	std::unordered_map<std::string, std::unique_ptr<DirLight>> dirLights;

public:
	Scene();

	void addObject(std::string* name, std::unique_ptr<TriMeshs> TriMeshs);
	void addObject(std::string* name, std::unique_ptr<DirLight> d);
	TriMeshs* accessObj(std::string* name);
	DirLight* accessLight(std::string* name);
};