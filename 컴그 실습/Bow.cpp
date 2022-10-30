#include "Bow.h"


void Bow::Initialize()
{
	colordata = (glm::vec3*)malloc(sizeof(glm::vec3) * 468);
	normaldata = (glm::vec3*)malloc(sizeof(glm::vec3) * 468);
	for (int i = 0; i < 78; i++)
	{
		colordata[i * 6] = glm::vec3(1.0, 1.0, 1.0);
		colordata[i * 6 + 1] = colordata[i * 6];
		colordata[i * 6 + 2] = colordata[i * 6];
		colordata[i * 6 + 3] = colordata[i * 6];
		colordata[i * 6 + 4] = colordata[i * 6];
		colordata[i * 6 + 5] = colordata[i * 6];
	}

	for (int i = 0; i < 282; i++)
	{
		normaldata[i] = glm::vec3(0.0, 0.0, 0.0);
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(4, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 468 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 468 * sizeof(glm::vec3), colordata, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, 468 * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, 468 * sizeof(glm::vec3), uvdata, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);
}
void Bow::update()
{
}
void Bow::Draw(int programID)
{
	unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
	glBindVertexArray(vao);
	glBindTexture(GL_TEXTURE_2D, texture[20]);
	glDrawArrays(GL_TRIANGLES, 0, 468);
}