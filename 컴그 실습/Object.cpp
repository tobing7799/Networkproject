#include "Object.h"

glm::mat4 Object::GetTransform()
{
	if (parent)
		return parent->GetTransform() * objectmatrix.GetTransform();
	return objectmatrix.GetTransform();
}

glm::mat4 Object::GetmodelTransform()
{
	return modelmatrix.GetTransform();
}

void Object::Readobj(FILE* objFile)
{
	fseek(objFile, 0, 0);
	char count[1200];
	int vertexNum = 0;
	int faceNum = 0;
	while (!feof(objFile)) {
		fscanf(objFile, "%s", count);
		if (count[0] == 'v' && count[1] == '\0')
			vertexNum += 1;
		else if (count[0] == 'f' && count[1] == '\0')
			faceNum += 1;
		memset(count, '\0', sizeof(count));
	}

	glm::vec3* vertex;
	glm::vec3* face;
	int vertIndex = 0;
	int faceIndex = 0;
	int normalIndex = 0;
	vertex = (glm::vec3*)malloc(sizeof(glm::vec3) * vertexNum);
	vertexdata = (glm::vec3*)malloc(sizeof(glm::vec3) * faceNum * 3);
	face = (glm::vec3*)malloc(sizeof(glm::vec3) * faceNum);

	fseek(objFile, 0, 0);
	while (!feof(objFile) && faceIndex < faceNum) {
		fscanf(objFile, "%s", count);
		if (count[0] == 'v' && count[1] == '\0') {
			fscanf(objFile, "%f %f %f",
				&vertex[vertIndex].x, &vertex[vertIndex].y,
				&vertex[vertIndex].z);
			vertIndex++;
		}
		else if (count[0] == 'f' && count[1] == '\0') {
			fscanf(objFile, "%f %f %f", &face[faceIndex].x, &face[faceIndex].y, &face[faceIndex].z);
			vertexdata[faceIndex * 3 + 0] = vertex[(int)face[faceIndex].x - 1];
			vertexdata[faceIndex * 3 + 1] = vertex[(int)face[faceIndex].y - 1];
			vertexdata[faceIndex * 3 + 2] = vertex[(int)face[faceIndex].z - 1];
			faceIndex++;
		}
	}
}

void Object::Readuv(FILE* objFile)
{
	fseek(objFile, 0, 0);
	char count[1200];
	int uvNum = 0;
	int uvNum2 = 0;
	while (!feof(objFile)) {
		fscanf(objFile, "%s", count);
		if (count[0] == 'v' && count[1] == '\0')
			uvNum += 1;
		else if (count[0] == 'f' && count[1] == '\0')
			uvNum2 += 1;
		memset(count, '\0', sizeof(count));
	}

	glm::vec3* u;
	glm::vec3* uv;
	glm::vec3* uvstore;


	int uvIndex = 0;
	int uvIndex2 = 0;
	uv = (glm::vec3*)malloc(sizeof(glm::vec3) * uvNum);
	uvdata = (glm::vec3*)malloc(sizeof(glm::vec3) * uvNum * 3);
	u = (glm::vec3*)malloc(sizeof(glm::vec3) * uvNum2);

	fseek(objFile, 0, 0);
	while (!feof(objFile)) {
		fscanf(objFile, "%s", count);
		if (count[0] == 'v' && count[1] == '\0') {
			fscanf(objFile, "%f %f %f",
				&uv[uvIndex].x, &uv[uvIndex].y,
				&uv[uvIndex].z);
			uvIndex++;
		}
		else if (count[0] == 'f' && count[1] == '\0') {
			fscanf(objFile, "%f %f %f", &u[uvIndex2].x, &u[uvIndex2].y, &u[uvIndex2].z);
			uvdata[uvIndex2 * 3 + 0] = uv[(int)u[uvIndex2].x - 1];
			uvdata[uvIndex2 * 3 + 1] = uv[(int)u[uvIndex2].y - 1];
			uvdata[uvIndex2 * 3 + 2] = uv[(int)u[uvIndex2].z - 1];
			uvIndex2++;
		}
	}
}