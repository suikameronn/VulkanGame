#version 450

layout(set = 0,binding = 0) uniform Camera
{
    vec3 position;
    mat4 view;
    mat4 proj;
} camera;

layout(location = 0) in vec3 inPosition;
//IBLのspecularのBRDFの事前計算以外では使わない
layout(location = 1) in vec2 inUV;

layout (location = 0) out vec3 localPos;
layout (location = 1) out vec2 outUV;

void main() 
{
    mat4 viewMat = mat4(mat3(camera.view));

    gl_Position = camera.proj * viewMat * vec4(inPosition,1.0);

    localPos = inPosition;

    outUV = inUV;
}