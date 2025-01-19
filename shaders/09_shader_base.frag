#version 450

layout(binding = 2) uniform sampler2D texSampler;

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec3 vWorldPosition;
layout(location = 2) in vec3 vWorldNormal;
layout(location = 3) in vec3 worldCameraPosition;
layout(location = 4) in vec3 cameraDir;

layout(location = 0) out vec4 outColor;

vec3 Lpos = vec3(30.0, -600.0, 0.0);
vec3 Ldiff = vec3(1.0);
vec3 Kdiff = vec3(0.6,0.6,0.2);
vec3 Kspec = vec3(1.0);
float shininess = 50.0;

void main() {

    Kdiff = texture(texSampler,texCoord).rgb;

    vec3 L = normalize(Lpos - vWorldPosition);
    vec3 V = normalize(worldCameraPosition - vWorldPosition);
    vec3 N = normalize(vWorldNormal);
    vec3 H = normalize(L + V);

    vec3 diffuse = max(dot(L,N),0.0) * Kdiff * Ldiff;

    vec3 specular = pow(max(dot(N,H),0.0),shininess) * Kspec * Ldiff;

    vec3 color = diffuse + specular;

    outColor = vec4(color,1.0);
}