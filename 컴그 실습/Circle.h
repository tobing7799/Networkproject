#pragma once
#include "Object.h"

class Circle :public Object
{
public:
	float angle = 0;
	float angle_count = 0;
	float r = 1.0;
	float x = 0;
	float y = 0;
	float z = 0;
	glm::vec4* colordata;
	glm::vec3* normaldata;
	glm::vec2* uvdata;

	void Initialize();
	void Update(float color_r, float color_g, float color_b);
	void Draw(int programID);
};