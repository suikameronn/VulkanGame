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
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord1;
layout(location = 3) in vec2 inTexCoord2;
layout(location = 4) in vec3 inNormal;
layout(location = 5) in ivec4 boneID1;
layout(location = 6) in vec4 weight1;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outUV0;

void main() 
{
    vec4 locPos;
    locPos = matricesUBO.model * vec4(inPosition,1.0);
    outNormal = normalize(transpose(inverse(mat3(matricesUBO.model))) * inNormal);

    outUV0 = inPosition;
    outUV0.xy *= -1.0;

    mat4 viewMat = mat4(mat3(matricesUBO.view));

    gl_Position = matricesUBO.proj * viewMat * locPos;
    gl_Position.y = -gl_Position.y;
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}