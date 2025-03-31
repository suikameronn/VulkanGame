#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 camPos;
    int lightCount;
    mat4[20] lightMVP;
} matricesUBO;

layout(location = 0) in vec3 inPosition;
//IBLのspecularのBRDFの事前計算以外では使わない
layout(location = 1) in vec2 inUV;

layout (location = 0) out vec2 outUV;

void main() 
{
    outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);

    gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
}