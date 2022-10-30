#include "Line.h"


void Line::Initialize()
{
	colordata = (glm::vec4*)malloc(sizeof(glm::vec4) * 100);
	vertexdata = (glm::vec3*)malloc(sizeof(glm::vec3) * 100);
	normaldata = (glm::vec3*)malloc(sizeof(glm::vec3) * 100);
	uvdata = (glm::vec2*)malloc(sizeof(glm::vec2) * 100);

	for (int i = 0; i < 100; i++)
	{
		vertexdata[i] = glm::vec3(0.0, 0.0, 0.0);
	}

	for (int i = 1; i < 100; i++)
	{
		colordata[i] = glm::vec4(1.0, 0.0, 0.0, 1.0);
	}


	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(4, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 100 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 100 * sizeof(glm::vec4), colordata, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, 100 * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, 100 * sizeof(glm::vec2), uvdata, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

}
void Line::Update(float line_v, float angle_x, float angle_y)
{
	time = 0;

	if (angle_x < 0)
	{
		angle_x = 180 + angle_x;
	}
	if (angle_x > 0)
	{
		angle_x = 90 + angle_x;
	}

	angle_x = angle_x * PI / 180.0;

	for (int i = 0; i < 100; i++)
	{
		time += 0.01;
		line_z = line_v * cos(angle_y) * time;
		line_y = line_v * sin(angle_y) * time - (0.5 * 9.8 * time * time);
		if (angle_x != 0)
		{
			line_x = float(line_z / tan(angle_x));
		}
		else
		{
			line_x = 0;
		}

		vertexdata[i] = glm::vec3(line_x, line_y, line_z);
	}
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 100 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
}
void Line::Draw(int programID)
{
	unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
	glBindVertexArray(vao);
	glLineWidth(5);
	glBindTexture(GL_TEXTURE_2D, texture[21]);
	glDrawArrays(GL_LINES, 0, 300);
}