#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 local;
    mat4 view;
    mat4 proj;
    vec3 worldCameraPos;
    vec3 cameraDir;
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

layout( push_constant ) uniform push_constant
{
    mat4 modelMatrix;
} PushConstants;

layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec3 vWorldPosition;
layout(location = 2) out vec3 vWorldNormal;
layout(location = 3) out vec3 worldCameraPosition;
layout(location = 4) out vec3 cameraDir;

void main() {

    mat4 boneMat = mat4(0.0);

    for(int i = 0;i < animationUBO.boneCount;i++)
    {
        boneMat += animationUBO.boneMatrix[boneID1[i]] * weight1[i];
    }

    if(boneMat == mat4(0.0))
    {
	boneMat = mat4(1.0);
    }

    mat4 M = matricesUBO.local *PushConstants.modelMatrix * boneMat;

    gl_Position = matricesUBO.proj * matricesUBO.view * M * vec4(inPosition,1.0);
    gl_Position.y = -gl_Position.y;
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;

    vec4 worldPosition = M * vec4(inPosition,1.0);
    vWorldPosition = worldPosition.xyz / worldPosition.w;
    vWorldNormal = mat3(inverse(M)) * inNormal;

    worldCameraPosition = matricesUBO.worldCameraPos;
    cameraDir = matricesUBO.cameraDir;

    texCoord = inTexCoord1;
}