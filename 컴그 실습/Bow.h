#pragma once
#include "Object.h"

class Bow :public Object
{
public:
	glm::vec3* colordata;
	glm::vec3* normaldata;

	void Initialize();
	void update();
	void Draw(int programID);
};