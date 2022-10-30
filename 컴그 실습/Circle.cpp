#include "Circle.h"

void Circle::Initialize()
{
	colordata = (glm::vec4*)malloc(sizeof(glm::vec4) * 360);
	vertexdata = (glm::vec3*)malloc(sizeof(glm::vec3) * 360);
	normaldata = (glm::vec3*)malloc(sizeof(glm::vec3) * 360);
	uvdata = (glm::vec2*)malloc(sizeof(glm::vec2) * 360);

	for (int i = 0; i < 360; i++)
	{
		angle = (float)angle_count / 360 * 2 * PI;
		angle_count += 1;
		x = (float)r * cos(angle);
		y = (float)r * sin(angle);
		z = 0.0;
		vertexdata[i] = glm::vec3(x, y, z);
	}

	for (int i = 0; i < 360; i++)
	{
		colordata[i] = glm::vec4(1.0, 1.0, 1.0, 1.0);
	}

	for (int i = 0; i < 360; i++)
	{
		normaldata[i] = glm::vec3(0.0, 0.0, 0.0);
	}

	for (int i = 0; i < 360; i++)
	{
		uvdata[i] = glm::vec2(1.0, 1.0);
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(4, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 360 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 360 * sizeof(glm::vec4), colordata, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, 360 * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, 360 * sizeof(glm::vec2), uvdata, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);
}
void Circle::Update(float color_r, float color_g, float color_b)
{
	for (int i = 0; i < 360; i++)
	{
		colordata[i] = glm::vec4(color_r, color_g, color_b, 1.0);
	}

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 360 * sizeof(glm::vec4), colordata, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
}
void Circle::Draw(int programID)
{
	unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
	glBindVertexArray(vao);
	glBindTexture(GL_TEXTURE_2D, texture[21]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 360);
}