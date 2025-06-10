#version 450

layout (location = 0) in vec2 inUV;
//Textures
layout (set = 1,binding = 0) uniform sampler2D uiTexture;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor.r = texture(uiTexture,inUV).r;

	outColor.a = outColor.r;
}