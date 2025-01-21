#version 450

layout(binding = 2) uniform sampler2D texSampler;

layout (location = 0) in vec3 outWorldPos;
layout (location = 1) in vec3 outNormal;
layout (location = 2) in vec4 outColor0;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(0.5,0.5,0.5,1.0);
}