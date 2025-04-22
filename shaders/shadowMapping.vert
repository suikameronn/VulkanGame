#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in ivec4 boneID1;
layout(location = 2) in vec4 weight1;

layout (set = 0,binding = 0) uniform UBO 
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(set = 1,binding = 0) uniform UniformBufferObject {
    vec3 scale;
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 camPos;
    mat4[20] lightMVP;
} matricesUBO;

layout(set = 1,binding = 1) uniform animationUniformBufferObject
{
    mat4 matrix;
    mat4[128] boneMatrix;
    int boneCount;
} animationUBO;

layout( push_constant ) uniform push_constant
{
    mat4 modelMatrix;
} PushConstants;
 
void main()
{
    if(animationUBO.boneCount > 0)
    {
        mat4 skinMat = 
        weight1.x * animationUBO.boneMatrix[boneID1.x] +
        weight1.y * animationUBO.boneMatrix[boneID1.y] +
        weight1.z * animationUBO.boneMatrix[boneID1.z] +
        weight1.w * animationUBO.boneMatrix[boneID1.w];

        gl_Position = ubo.proj * ubo.view * matricesUBO.model * PushConstants.modelMatrix * skinMat * vec4(inPosition,1.0);
    }
    else
    {
        gl_Position = ubo.proj * ubo.view * matricesUBO.model * PushConstants.modelMatrix * vec4(inPosition,1.0);
    }
    
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}