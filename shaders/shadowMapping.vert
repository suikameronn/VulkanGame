#version 450

layout (location = 0) in vec3 inPos;

layout (binding = 0) uniform UBO 
{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 worldCameraPos;
} ubo;

layout( push_constant ) uniform push_constant
{
    mat4 modelMatrix;
} PushConstants;
 
void main()
{
	gl_Position =  ubo.proj * ubo.view * ubo.model * PushConstants.modelMatrix * vec4(inPos, 1.0);
}