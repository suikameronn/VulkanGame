#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4[251] boneMatrix;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 texCoord;
layout(location = 4) in ivec4 boneID1;
layout(location = 5) in ivec4 boneID2;
layout(location = 6) in vec4 weight1;
layout(location = 7) in vec4 weight2;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {

    mat4 boneMat = mat4(1.0);

    boneMat = ubo.boneMatrix[boneID1[0]] * weight1[0];

    for(int i = 1;i < 4;i++)
    {
        if(weight1[i] != -1.0)
        {
	        boneMat += ubo.boneMatrix[boneID1[i]] * weight1[i];
	    }
    }

    for(int i = 0;i < 4;i++)
    {
        if(weight2[i] != -1.0)
        {
            boneMat += ubo.boneMatrix[boneID2[i]] * weight2[i];
        }
    }

    vec4 P;

    if(boneMat != mat4(0.0))
    {
        P = ubo.view * ubo.model * boneMat * vec4(inPosition,1.0);
    }
    else
    {
        P = ubo.view * ubo.model * vec4(inPosition,1.0);
    }
    
    gl_Position = ubo.proj * P;
    fragColor = vec3(100,100,100);
    fragTexCoord = texCoord;
}