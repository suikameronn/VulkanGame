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

layout(location = 0) out vec3 vertexColor;
layout(location = 1) out vec3 diffuseColor;

vec4 Lpos = vec4(10.0, -600.0, 3.0,1.0);
vec3 Ldiff = vec3(0.1);
vec3 Kdiff = vec3(0.6,0.6,0.2);

layout( push_constant ) uniform push_constant
{
    mat4 modelMatrix;
} PushConstants;

void main() {

    mat4 MV = ubo.view * ubo.local * PushConstants.modelMatrix;

    vec4 P = MV * vec4(inPosition,1.0);
    
    gl_Position = ubo.proj * P;
    gl_Position.y = -gl_Position.y;
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;

    vertexColor = ubo.diffuse;

    vec3 N = normalize(transpose(mat3(inverse(MV))) * inNormal);
    vec3 L = normalize((Lpos * P.w - P * Lpos.w).xyz);
    diffuseColor = max(dot(N,L),0.0) * Kdiff * Ldiff;
}