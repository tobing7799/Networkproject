#version 330 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec4 in_Color;
layout (location = 2) in vec3 vNormal;
layout(location = 3) in vec2 vTexCoord;

out vec4 out_Color;
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 modelTransform;
uniform mat4 cameraTransform;
uniform mat4 projectionTransform;

void main()
{
gl_Position = projectionTransform *cameraTransform *modelTransform* vec4(vPos, 1.0);
FragPos = vec3(modelTransform*vec4(vPos,1.0));
Normal = vec3(modelTransform*vec4(vNormal,0.0));
out_Color = in_Color;
TexCoord = vTexCoord;
}