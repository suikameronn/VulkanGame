#version 450

layout(set = 0,binding = 0) uniform Camera
{
    vec3 pos;
    mat4 view;
    mat4 proj;
} camera;

layout(location = 0) in vec3 inPosition;

layout (location = 0) out vec3 outTexCoords;

void main() 
{
    mat4 viewMat = mat4(mat3(camera.view));

    gl_Position = camera.proj * viewMat * vec4(inPosition,1.0);

    outTexCoords = inPosition;
}