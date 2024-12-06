#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 local;
    mat4 view;
    mat4 proj;
    mat4[128] boneMatrix;
    vec3 diffuse;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 4) in ivec4 boneID1;
layout(location = 5) in vec4 weight1;

layout(location = 0) out vec3 fragColor;

layout( push_constant ) uniform push_constant
{
    mat4 modelMatrix;
} PushConstants;

void main() {
    mat4 boneMat = mat4(1.0);

    boneMat = ubo.boneMatrix[boneID1[0]] * weight1[0];
    boneMat += ubo.boneMatrix[boneID1[1]] * weight1[1];
    boneMat += ubo.boneMatrix[boneID1[2]] * weight1[2];
    boneMat += ubo.boneMatrix[boneID1[3]] * weight1[3];

    if(boneMat == mat4(0.0))
    {
	boneMat = mat4(1.0);
    }

    vec4 P = ubo.view * ubo.local * PushConstants.modelMatrix * boneMat * vec4(inPosition,1.0);
    
    gl_Position = ubo.proj * P;
    gl_Position.y = -gl_Position.y;
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
    fragColor = ubo.diffuse;
}