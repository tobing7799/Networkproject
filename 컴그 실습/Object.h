#pragma once
#include "global.h"

extern unsigned int texture[44];

class Object {
public:
	GLuint vao, vbo[4];
	glm::vec3* vertexdata;
	glm::vec3* uvdata;
	Transform objectmatrix;
	Transform modelmatrix;
	Object* parent{ nullptr };
	glm::mat4 GetTransform();

	glm::mat4 GetmodelTransform();

	void Readobj(FILE* objFile);

	void Readuv(FILE* objFile);
};