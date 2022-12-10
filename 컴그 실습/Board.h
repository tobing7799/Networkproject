#pragma once
#include "Object.h"

class Board :public Object
{
public:
	glm::vec3* colordata;
	glm::vec3* normaldata;
	glm::vec2* uvdata;

	void Initialize();
	void Draw(int programID);
	void Draw1(int programID, int number_10);
	void Draw2(int programID, int number_1);
	void Draw3(int programID, int wind_dir);
	void Draw4(int programID, int wind_speed);
	void Draw5(int programID);
	void Draw6(int programID);
	void Draw7(int programID);
	void Draw8(int programID);
};