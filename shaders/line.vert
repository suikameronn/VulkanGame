#version 450

layout (set = 0,binding = 0) uniform ModelRender 
{
    vec3 scale;
    mat4 matrix;
} modelMatrix;

layout(set = 1,binding = 0) uniform Camera
{
    vec3 pos;
    mat4 view;
    mat4 proj;
} camera;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragColor;

void main() 
{
    gl_Position = camera.proj * camera.view * modelMatrix.matrix * vec4(inPosition,1.0);
    gl_Position.y = -gl_Position.y;
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;

    fragColor = vec3(100.0,100.0,100.0);
}