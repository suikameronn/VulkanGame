#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat3 normal;
    vec3 diffuse;
    vec3 ambient;
    vec3 specular;
    vec3 emissive;
    vec3 transmissive;
    float shininess;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 Idiff;
layout(location = 3) out vec3 Ispec;

const vec4 Lpos = vec4(0.0,0.0,5.0,1.0);
const vec3 Lamb = vec3(0.0);
const vec3 Lspec = vec3(1.0);
const vec3 Ldiff = vec3(1.0);

void main() {
    vec4 P = ubo.view * ubo.model * vec4(inPosition,1.0);
    vec3 L = normalize((Lpos * P.w - P * Lpos.w).xyz);
    vec3 N = normalize(ubo.normal * inNormal);
    vec3 Iamb = ubo.ambient * Lamb;
    Idiff = max(dot(N,L),0.0) * ubo.diffuse * Ldiff + Iamb;
    vec3 V = -normalize(P.xyz);
    vec3 R = reflect(-L,N);
    Ispec = pow(max(dot(R,V),0.0),ubo.shininess) * ubo.specular * Lspec + Idiff;
    gl_Position = ubo.proj * P;
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}