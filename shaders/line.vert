#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    mat4[250] boneMatrix;
} ubo;

layout(location = 0) in vec3 inPosition;

layout( push_constant ) uniform push_constant
{
    mat4 modelMatrix;
} PushConstants;

layout(location = 0) out vec3 fragColor;

void main() {

    vec4 P = ubo.view * PushConstants.modelMatrix * vec4(inPosition,1.0);

    gl_Position = ubo.proj * P;
    gl_Position.y = -gl_Position.y;
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
    fragColor = vec3(100.0,100.0,100.0);
}