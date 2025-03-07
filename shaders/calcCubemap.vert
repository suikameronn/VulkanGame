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

layout (location = 0) out vec3 localPos;

void main() 
{
    localPos = inPosition;

    vec4 pos;
    pos = matricesUBO.model * vec4(inPosition,1.0);

    gl_Position = matricesUBO.proj * matricesUBO.view * pos;
    gl_Position.y = -gl_Position.y;
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}