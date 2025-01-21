#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 camPos;
} matricesUBO;

layout(binding = 1) uniform animationUniformBufferObject
{
    mat4 matrix;
    mat4[128] boneMatrix;
    int boneCount;
} animationUBO;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord1;
layout(location = 3) in vec2 inTexCoord2;
layout(location = 4) in vec3 inNormal;
layout(location = 5) in ivec4 boneID1;
layout(location = 6) in vec4 weight1;

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec4 outColor0;

layout( push_constant ) uniform push_constant
{
    mat4 modelMatrix;
} PushConstants;

void main() {

    outColor0 = vec4(inColor,1.0);

    vec4 locPos;
    if(animationUBO.boneCount > 0)
    {
        mat4 skinMat = 
        weight1.x * animationUBO.boneMatrix[boneID1.x] +
        weight1.y * animationUBO.boneMatrix[boneID1.y] +
        weight1.z * animationUBO.boneMatrix[boneID1.z] +
        weight1.w * animationUBO.boneMatrix[boneID1.w];

        locPos = matricesUBO.model * animationUBO.matrix * skinMat * vec4(inPosition,1.0);
        outNormal = normalize(transpose(inverse(mat3(matricesUBO.model * animationUBO.matrix * skinMat))) * inNormal);
    }
    else
    {
        locPos = matricesUBO.model * animationUBO.matrix * vec4(inPosition,1.0);
        outNormal = normalize(transpose(inverse(mat3(matricesUBO.model * animationUBO.matrix))) * inNormal);
    }

    locPos.y = -locPos.y;
    locPos.z = (locPos.z + locPos.w) / 2.0;

    outWorldPos = locPos.xyz / locPos.w;

    gl_Position = matricesUBO.proj * matricesUBO.view * vec4(outWorldPos,1.0);
}