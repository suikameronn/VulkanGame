#version 450

layout(binding = 2) uniform sampler2D texSampler;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV0;
layout (location = 3) in vec2 inUV1;
layout (location = 4) in vec4 inColor0;
layout(location = 5) in vec3 camPos;

layout(location = 0) out vec4 outColor;

vec3 Lpos = vec3(30.0, -600.0, 0.0);
vec3 Ldiff = vec3(1.0);
vec3 Kdiff = vec3(0.7765, 0.2667, 0.2667);
vec3 Kspec = vec3(1.0);
float shininess = 50.0;

void main() {

    Kdiff = texture(texSampler,inUV0).rgb;

    vec3 L = normalize(Lpos - inPos);
    vec3 V = normalize(camPos - inPos);
    vec3 N = normalize(inNormal);
    vec3 H = normalize(L + V);

    vec3 diffuse = max(dot(L,N),0.0) * Kdiff * Ldiff;

    vec3 specular = pow(max(dot(N,H),0.0),shininess) * Kspec * Ldiff;

    vec3 color = diffuse + specular;

    outColor = vec4(color,1.0);
}