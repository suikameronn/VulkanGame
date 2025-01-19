#version 450

layout(binding = 2) uniform sampler2D texSampler;

layout(location = 0) in vec3 vertexColor;
layout(location = 1) in vec3 diffuseColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(vertexColor + diffuseColor,1.0);
}