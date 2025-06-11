#version 450

layout (location = 0) in vec2 inUV;
//Textures
layout (set = 1,binding = 0) uniform sampler2D uiTexture;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = texture(uiTexture,inUV);

	if(outColor.a == 0.0)
	{
		discard;
	}
}