#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;
uniform mat4 modelsky;
uniform mat4 projectionsky;
uniform mat4 viewsky;

void main()
{
    TexCoords = aPos;
    vec4 pos = modelsky * projectionsky * viewsky * vec4(aPos, 0.0);
    gl_Position = pos.xyww;
}  