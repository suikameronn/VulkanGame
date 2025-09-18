#version 450

layout (set = 0,binding = 0) uniform ModelRender 
{
    vec3 scale;
    mat4 matrix;
} modelMatrix;

layout(set = 0,binding = 1) uniform NodeAnimMatrices
{
    mat4 nodeMatrix;
    mat4 matrix;
    int boneCount;
} nodeAnimMatrices;

layout(set = 0,binding = 2) uniform BoneMatrices
{
    mat4[128] matrices;
} boneMatrices;

layout(set = 1,binding = 0) uniform Camera
{
    vec3 pos;
    mat4 view;
    mat4 proj;
} camera;

const int LIGHT_MAX = 10;

layout(set = 2, binding = 0) uniform PointLightUBO
{
	int lightCount;
	vec4[LIGHT_MAX] pos;
	vec4[LIGHT_MAX] color;
    mat4[LIGHT_MAX] viewProj;
}pointLight;

layout(set = 2,binding = 1) uniform DirectionalLightUBO
{
	int lightCount;
	vec4[LIGHT_MAX] dir;
	vec4[LIGHT_MAX] color;
    mat4[LIGHT_MAX] viewProj;
}directionLight;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord1;
layout(location = 3) in vec2 inTexCoord2;
layout(location = 4) in vec3 inNormal;
layout(location = 5) in ivec4 boneID1;
layout(location = 6) in vec4 weight1;

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outUV0;
layout (location = 3) out vec2 outUV1;
layout (location = 4) out vec4 outColor0;
layout(location = 5) out vec3 camPos;
layout(location = 6) out vec4 outPointLightShadowCoords[LIGHT_MAX];
layout(location = 7 + LIGHT_MAX) out vec4 outDirectionLightShadowCoords[LIGHT_MAX];

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() {
    outColor0 = vec4(inColor,1.0);

    vec4 locPos;
    if(nodeAnimMatrices.boneCount > 0)
    {
        mat4 skinMat = 
        weight1.x * boneMatrices.matrices[boneID1.x] +
        weight1.y * boneMatrices.matrices[boneID1.y] +
        weight1.z * boneMatrices.matrices[boneID1.z] +
        weight1.w * boneMatrices.matrices[boneID1.w];

        locPos = modelMatrix.matrix * nodeAnimMatrices.nodeMatrix * skinMat * vec4(inPosition,1.0);
        outNormal = normalize(transpose(inverse(mat3(modelMatrix.matrix * nodeAnimMatrices.matrix * nodeAnimMatrices.nodeMatrix * skinMat))) * inNormal);
    }
    else
    {
        locPos = modelMatrix.matrix * vec4(inPosition,1.0);

        mat3 a = mat3(modelMatrix.matrix);
        mat3 b = inverse(a);
        mat3 c = transpose(b);

        outNormal = normalize(transpose(inverse(mat3(modelMatrix.matrix))) * inNormal);
    }
    
    for(int i = 0;i < pointLight.lightCount;i++)
    {

    }

    for(int i = 0;i < directionLight.lightCount;i++)
    {
        outDirectionLightShadowCoords[i] = directionLight.viewProj[i] * locPos;
        outDirectionLightShadowCoords[i].z = (outDirectionLightShadowCoords[i].z + outDirectionLightShadowCoords[i].w) / 2.0;
        outDirectionLightShadowCoords[i] = biasMat * outDirectionLightShadowCoords[i];
    }

    outWorldPos = locPos.xyz / locPos.w;

    //outUV0 = inTexCoord1 * modelMatrix.scale.xy;
    //outUV1 = inTexCoord2 * modelMatrix.scale.xy;

    outUV0 = inTexCoord1;
    outUV1 = inTexCoord2;

    camPos = vec3(camera.pos);

    gl_Position = camera.proj * camera.view * locPos;
    gl_Position.y = -gl_Position.y;
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}