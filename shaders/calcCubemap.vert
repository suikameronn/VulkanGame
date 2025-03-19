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
    mat4 viewMat = mat4(mat3(matricesUBO.view));

    gl_Position = matricesUBO.proj * viewMat * vec4(inPosition,1.0);

    localPos = inPosition;
    localPos.y = -localPos.y;
}