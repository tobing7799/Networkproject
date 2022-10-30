#pragma once
#include "Object.h"

class Cube :public Object
{
public:
	glm::vec3* colordata;
	glm::vec3* normaldata;
	glm::vec2* uvdata;

	void Initialize();
	void update();
	void Draw(int programID);
};