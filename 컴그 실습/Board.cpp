#include "Board.h"


void Board::Initialize()
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
		vertexdata[i] -= glm::vec3(0.5, 0.5, 0.0);
	}

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
void Board::Draw(int programID)
{
	unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
	glBindVertexArray(vao);
	glBindTexture(GL_TEXTURE_2D, texture[28]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Board::Draw1(int programID, int number_10)
{
	unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
	glBindVertexArray(vao);
	switch (number_10) {
	case 0:
		glBindTexture(GL_TEXTURE_2D, texture[29]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 1:
		glBindTexture(GL_TEXTURE_2D, texture[30]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 2:
		glBindTexture(GL_TEXTURE_2D, texture[31]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 3:
		glBindTexture(GL_TEXTURE_2D, texture[32]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 4:
		glBindTexture(GL_TEXTURE_2D, texture[33]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 5:
		glBindTexture(GL_TEXTURE_2D, texture[34]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 6:
		glBindTexture(GL_TEXTURE_2D, texture[35]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 7:
		glBindTexture(GL_TEXTURE_2D, texture[36]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 8:
		glBindTexture(GL_TEXTURE_2D, texture[37]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 9:
		glBindTexture(GL_TEXTURE_2D, texture[38]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	}
}

void Board::Draw2(int programID, int number_1)
{
	unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
	glBindVertexArray(vao);
	switch (number_1) {
	case 0:
		glBindTexture(GL_TEXTURE_2D, texture[29]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 1:
		glBindTexture(GL_TEXTURE_2D, texture[30]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 2:
		glBindTexture(GL_TEXTURE_2D, texture[31]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 3:
		glBindTexture(GL_TEXTURE_2D, texture[32]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 4:
		glBindTexture(GL_TEXTURE_2D, texture[33]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 5:
		glBindTexture(GL_TEXTURE_2D, texture[34]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 6:
		glBindTexture(GL_TEXTURE_2D, texture[35]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 7:
		glBindTexture(GL_TEXTURE_2D, texture[36]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 8:
		glBindTexture(GL_TEXTURE_2D, texture[37]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 9:
		glBindTexture(GL_TEXTURE_2D, texture[38]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	}
}

void Board::Draw3(int programID, int wind_dir)
{
	unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
	glBindVertexArray(vao);
	glBindTexture(GL_TEXTURE_2D, texture[39]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Board::Draw4(int programID, int wind_speed)
{
	unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
	glBindVertexArray(vao);
	switch (wind_speed) {
	case 0:
		glBindTexture(GL_TEXTURE_2D, texture[29]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 1:
		glBindTexture(GL_TEXTURE_2D, texture[30]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 2:
		glBindTexture(GL_TEXTURE_2D, texture[31]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 3:
		glBindTexture(GL_TEXTURE_2D, texture[32]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 4:
		glBindTexture(GL_TEXTURE_2D, texture[33]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 5:
		glBindTexture(GL_TEXTURE_2D, texture[34]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 6:
		glBindTexture(GL_TEXTURE_2D, texture[35]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 7:
		glBindTexture(GL_TEXTURE_2D, texture[36]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 8:
		glBindTexture(GL_TEXTURE_2D, texture[37]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 9:
		glBindTexture(GL_TEXTURE_2D, texture[38]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	}
}

void Board::Draw5(int programID)
{
	unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
	glBindVertexArray(vao);
	glBindTexture(GL_TEXTURE_2D, texture[40]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}