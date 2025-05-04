#version 450

layout(binding = 0) uniform UniformBufferObject {
    vec3 scale;
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 camPos;
    int lightCount;
    mat4[20] lightMVP;
} ubo;

layout(location = 0) in vec3 inPosition;

layout( push_constant ) uniform push_constant
{
    mat4 modelMatrix;
} PushConstants;

layout(location = 0) out vec3 fragColor;

void main() {

    vec4 P = ubo.view * ubo.model * vec4(inPosition,1.0);

    gl_Position = ubo.proj * P;
    gl_Position.y = -gl_Position.y;
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
    fragColor = vec3(100.0,100.0,100.0);
}