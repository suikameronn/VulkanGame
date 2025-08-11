#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in ivec4 boneID1;
layout(location = 2) in vec4 weight1;

layout (set = 0,binding = 0) uniform ModelMatrix 
{
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

layout(set = 1,binding = 0) uniform LightMatrices
{
    mat4 viewProjMatrix;
} lightMat;
 
void main()
{
    if(nodeAnimMatrices.boneCount > 0)
    {
        mat4 skinMat = 
        weight1.x * boneMatrices.matrices[boneID1.x] +
        weight1.y * boneMatrices.matrices[boneID1.y] +
        weight1.z * boneMatrices.matrices[boneID1.z] +
        weight1.w * boneMatrices.matrices[boneID1.w];

        gl_Position = lightMat.viewProjMatrix * modelMatrix.matrix * nodeAnimMatrices.nodeMatrix * skinMat * vec4(inPosition,1.0);
    }
    else
    {
        gl_Position = lightMat.viewProjMatrix * modelMatrix.matrix * nodeAnimMatrices.nodeMatrix * vec4(inPosition,1.0);
    }
    
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}