#pragma once
#include "Object.h"

const int speedSize = 1000;
const int speedSizeDiv = 10000;
const float minSpeed = 0.01;

class Snow :public Object
{
public:
	glm::vec3* colordata;
	glm::vec3* normaldata;
	glm::vec2* uvdata;
	float speed = (((float)(rand() % speedSize) / speedSizeDiv) + minSpeed);
	void Initialize();
	void Draw(int programID);
};