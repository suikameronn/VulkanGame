#version 450

layout (location = 0) in vec3 inPos;

layout(binding = 1) uniform sampler2D hdriTexture;

layout(location = 0) out vec4 outColor;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() 
{
	vec2 uv = SampleSphericalMap(normalize(inPos));
    vec3 color = texture(hdriTexture, uv).rgb;
    
    outColor = vec4(color, 1.0);
}