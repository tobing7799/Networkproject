#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <time.h>
#include <math.h>
#define SNOW_SIZE 100
#define GRASS_SIZE 100
#define CUBE_SIZE 20
#define PI 3.141592
#pragma comment(lib, "winmm.lib")
#include "Mmsystem.h"
#include "Digitalv.h"

extern unsigned int texture[42];

struct Transform {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0);

	glm::mat4 GetTransform()
	{
		glm::mat4 TR = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 SR = glm::scale(glm::mat4(1.0f), scale);
		glm::mat4 RX = glm::rotate(glm::mat4(1.0f), (float)glm::radians(rotation.x), glm::vec3(1.0, 0.0, 0.0));
		glm::mat4 RY = glm::rotate(glm::mat4(1.0f), (float)glm::radians(rotation.y), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 RZ = glm::rotate(glm::mat4(1.0f), (float)glm::radians(rotation.z), glm::vec3(0.0, 0.0, 1.0));
		return TR * RX * RY * RZ * SR;
	}
};




