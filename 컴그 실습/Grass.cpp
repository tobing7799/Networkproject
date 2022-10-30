#include "Grass.h"



void Grass::Initialize()
{
	vertexdata = (glm::vec3*)malloc(sizeof(glm::vec3) * 6);
	colordata = (glm::vec3*)malloc(sizeof(glm::vec3) * 6);
	normaldata = (glm::vec3*)malloc(sizeof(glm::vec3) * 6);
	uvdata = (glm::vec2*)malloc(sizeof(glm::vec2) * 6);

	vertexdata[0] = glm::vec3(0.0, 0.0, 0.0);
	vertexdata[1] = glm::vec3(1.0, 0.0, 0.0);
	vertexdata[2] = glm::vec3(1.0, 1.0, 0.0);
	vertexdata[3] = glm::vec3(0.0, 0.0, 0.0);
	vertexdata[4] = glm::vec3(1.0, 1.0, 0.0);
	vertexdata[5] = glm::vec3(0.0, 1.0, 0.0);

	for (int i = 0; i < 6; i++)
	{
		colordata[i] = glm::vec3(1.0, 1.0, 1.0);
	}

	glm::vec3 normal = glm::cross(vertexdata[1] - vertexdata[0], vertexdata[2] - vertexdata[0]);
	for (int i = 0; i < 6; i++)
	{
		normaldata[i] = normal;
	}


	uvdata[0] = glm::vec2(0.0, 0.0);
	uvdata[1] = glm::vec2(1.0, 0.0);
	uvdata[2] = glm::vec2(1.0, 1.0);
	uvdata[3] = glm::vec2(0.0, 0.0);
	uvdata[4] = glm::vec2(1.0, 1.0);
	uvdata[5] = glm::vec2(0.0, 1.0);


	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(4, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), colordata, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec2), uvdata, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);
}
void Grass::Draw(int programID)
{
	unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
	glBindVertexArray(vao);
	glBindTexture(GL_TEXTURE_2D, texture[7]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}