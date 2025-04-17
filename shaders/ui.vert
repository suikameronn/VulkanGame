#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 transformMatrix;
    mat4 projection;
} matricesUBO;

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 outUV;

void main()
{
    gl_Position = matricesUBO.projection * matricesUBO.transformMatrix * vec4(pos,0.0,1.0);
    gl_Position.y = -gl_Position.y;
    gl_Position.z = 0.0;
    gl_Position.w = 1.0;

    outUV = uv;
}