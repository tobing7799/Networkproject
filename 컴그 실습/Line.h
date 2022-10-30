#pragma once
#include "Object.h"

class Line : public Object
{
public:
	glm::vec4* colordata;
	glm::vec3* normaldata;
	glm::vec2* uvdata;

	float time = 0;
	float line_x = 0;
	float line_y = 0;
	float line_z = 0;

	void Initialize();
	void Update(float line_v, float angle_x, float angle_y);
	void Draw(int programID); 
};