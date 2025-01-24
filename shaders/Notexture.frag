#version 450

layout (location = 0) in vec3 outWorldPos;
layout (location = 1) in vec3 outNormal;
layout (location = 2) in vec4 outColor0;

//Textures
layout (set = 1, binding = 0) uniform sampler2D colorMap;
layout (set = 1, binding = 1) uniform sampler2D physicalDescriptorMap;
layout (set = 1, binding = 2) uniform sampler2D normalMap;
layout (set = 1, binding = 3) uniform sampler2D aoMap;
layout (set = 1, binding = 4) uniform sampler2D emissiveMap;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(0.5,0.5,0.5,1.0);
}