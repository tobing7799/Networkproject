#pragma once
#include "Object.h"

class Snow :public Object
{
public:
	glm::vec3* colordata;
	glm::vec3* normaldata;
	glm::vec2* uvdata;
	float speed = (((float)(rand() % 1000) / 10000) + 0.001);
	void Initialize();
	void Draw(int programID);
};