#pragma once
#include "Object.h"

class Grass :public Object
{
public:
	glm::vec3* colordata;
	glm::vec3* normaldata;
	glm::vec2* uvdata;

	void Initialize();
	void Draw(int programID);
};