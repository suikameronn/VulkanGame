#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inUV0;

layout(binding = 1) uniform samplerCube skyboxTexture;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = texture(skyboxTexture,inUV0);
}