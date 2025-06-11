#version 450

layout(set = 0,binding = 0) uniform UniformBufferObject {
    mat4 transformMatrix;
    mat4 projection;
} matricesUBO;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 outUV;

void main()
{
    vec2 pos2D = pos.xy;

    gl_Position = matricesUBO.projection * matricesUBO.transformMatrix * vec4(pos2D,0.0,1.0);
    gl_Position.y = -gl_Position.y;
    gl_Position.z = pos.z;
    gl_Position.w = 1.0;

    outUV = uv;
}