#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 camPos;
    mat4[20] lightMVP;
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

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outUV0;
layout (location = 3) out vec2 outUV1;
layout (location = 4) out vec4 outColor0;
layout(location = 5) out vec3 camPos;
layout(location = 6) out vec4 outShadowCoords;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() {
    outColor0 = vec4(inColor,1.0);

    mat4 boneMat = mat4(0.0);

    vec4 locPos;
    if(animationUBO.boneCount > 0)
    {
        mat4 skinMat = 
        weight1.x * animationUBO.boneMatrix[boneID1.x] +
        weight1.y * animationUBO.boneMatrix[boneID1.y] +
        weight1.z * animationUBO.boneMatrix[boneID1.z] +
        weight1.w * animationUBO.boneMatrix[boneID1.w];

        locPos = matricesUBO.model * PushConstants.modelMatrix * skinMat * vec4(inPosition,1.0);
        outNormal = normalize(transpose(inverse(mat3(matricesUBO.model * animationUBO.matrix * PushConstants.modelMatrix * skinMat))) * inNormal);
    }
    else
    {
        locPos = matricesUBO.model * PushConstants.modelMatrix * vec4(inPosition,1.0);
        outNormal = normalize(transpose(inverse(mat3(matricesUBO.model * animationUBO.matrix * PushConstants.modelMatrix))) * inNormal);
    }

    outWorldPos = locPos.xyz / locPos.w;

    outUV0 = inTexCoord1;
    outUV1 = inTexCoord2;

    camPos = vec3(matricesUBO.camPos);

    gl_Position = matricesUBO.proj * matricesUBO.view * locPos;
    gl_Position.y = -gl_Position.y;
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;

    outShadowCoords = ( biasMat * matricesUBO.lightMVP[0] * matricesUBO.model ) * vec4(inPosition, 1.0);
}