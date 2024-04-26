#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 Idiff;
layout(location = 3) in vec3 Ispec;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(Idiff + Ispec,1.0) + vec4(texture(texSampler, fragTexCoord).xyz * 0.5,1.0) ;
}