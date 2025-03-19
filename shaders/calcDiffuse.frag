#version 450

layout (location = 0) in vec3 inPos;

layout(binding = 1) uniform samplerCube hdriTextureMap;

layout (location = 0) out vec4 outColor;

const float PI = 3.1452;

void main()
{
    outColor = vec4(0.0);  

    vec3 localPos = normalize(inPos);

    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, localPos));
    up = normalize(cross(localPos, right));

    float sampleDelta = 0.025;
    float nrSamples = 0.0; 

    //半球の離散した領域をすべて探索
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // 球面座標を接線空間に変換
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // 接戦空間をワールド空間に変換
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * localPos; 

            outColor += texture(hdriTextureMap, sampleVec) * cos(theta) * sin(theta);
            nrSamples++;
        }
    }

    outColor = PI * outColor * (1.0 / float(nrSamples));
    outColor.a = 1.0;
}