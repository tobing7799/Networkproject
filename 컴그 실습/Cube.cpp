#include "Cube.h"

void Cube::Initialize()
{
	colordata = (glm::vec3*)malloc(sizeof(glm::vec3) * 36);
	normaldata = (glm::vec3*)malloc(sizeof(glm::vec3) * 36);
	uvdata = (glm::vec2*)malloc(sizeof(glm::vec2) * 36);

	for (int i = 0; i < 6; i++)
	{
		colordata[i * 6] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		colordata[i * 6 + 1] = colordata[i * 6];
		colordata[i * 6 + 2] = colordata[i * 6];
		colordata[i * 6 + 3] = colordata[i * 6];
		colordata[i * 6 + 4] = colordata[i * 6];
		colordata[i * 6 + 5] = colordata[i * 6];
	}

	for (int i = 0; i < 36; i++)
	{
		vertexdata[i] -= glm::vec3(0.5, 0.5, 0.5);
	}

	for (int i = 0; i < 6; i++)
	{
		glm::vec3 normal = glm::cross(vertexdata[i * 6 + 1] - vertexdata[i * 6 + 0], vertexdata[i * 6 + 2] - vertexdata[i * 6 + 0]);
		normaldata[i * 6 + 0] = normal;
		normaldata[i * 6 + 1] = normal;
		normaldata[i * 6 + 2] = normal;
		normaldata[i * 6 + 3] = normal;
		normaldata[i * 6 + 4] = normal;
		normaldata[i * 6 + 5] = normal;
	}

	for (int i = 0; i < 6; i++)
	{
		uvdata[i * 6 + 0] = glm::vec2(0.0, 0.0);
		uvdata[i * 6 + 1] = glm::vec2(1.0, 0.0);
		uvdata[i * 6 + 2] = glm::vec2(1.0, 1.0);
		uvdata[i * 6 + 3] = glm::vec2(0.0, 0.0);
		uvdata[i * 6 + 4] = glm::vec2(1.0, 1.0);
		uvdata[i * 6 + 5] = glm::vec2(0.0, 1.0);
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(4, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), colordata, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec2), uvdata, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);
}
void Cube::update()
{
}
void Cube::Draw(int programID)
{
	unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
	glBindVertexArray(vao);
	glBindTexture(GL_TEXTURE_2D, texture[41]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindTexture(GL_TEXTURE_2D, texture[41]);
	glDrawArrays(GL_TRIANGLES, 6, 6);
	glBindTexture(GL_TEXTURE_2D, texture[41]);
	glDrawArrays(GL_TRIANGLES, 12, 6);
	glBindTexture(GL_TEXTURE_2D, texture[41]);
	glDrawArrays(GL_TRIANGLES, 18, 6);
	glBindTexture(GL_TEXTURE_2D, texture[41]);
	glDrawArrays(GL_TRIANGLES, 24, 6);
	glBindTexture(GL_TEXTURE_2D, texture[41]);
	glDrawArrays(GL_TRIANGLES, 30, 6);
}