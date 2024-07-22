#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    vec4 P = ubo.view * ubo.model * vec4(inPosition,1.0);
    gl_Position = ubo.proj * P;
    fragColor = vec3(100,100,100);
}