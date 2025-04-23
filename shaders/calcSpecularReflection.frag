#version 450

layout (location = 0) in vec3 inPos;

layout(binding = 1) uniform samplerCube hdriTextureMap;

layout(push_constant) uniform PushConstants
{
    float roughness;
}pushConstant;

layout (location = 0) out vec4 outColor;

const float PI = 3.14159265359;

float RadicalInverse_VdC(uint bits) 
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

//Hammersley関数 昇順の半球状の点へのベクトルを万遍なく作成する
vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

//入射光と視線の半分のベクトルから、より結果の寄与の大きい方向のベクトルを求める
vec3 ImportanceSampleGGX(vec2 Xi,vec3 normal,float roughness)
{
    float a = roughness * roughness;

    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    //球面座標から直交座標
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    //接線空間からワールド空間への変換
    vec3 up = abs(normal.z) < 0.999 ? vec3(0.0,0.0,1.0) : vec3(1.0,0.0,0.0);
    vec3 tangent = normalize(cross(up,normal));
    vec3 bitangent = cross(normal,tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + normal * H.z;
    return normalize(sampleVec);
}

vec4 linearToSrgb(vec4 srgbIn)//リニアからsRGBに変換
{
	return vec4(pow(srgbIn.xyz,vec3(1.0/2.2)),srgbIn.w);
}

void main() 
{
    vec3 normal = normalize(inPos);
    vec3 reflection = normal;
    vec3 view = normal;

    const uint SAMPLE_COUNT = 1024u;
    outColor = vec4(0.0);
    float totalWeight = 0.0;

    //半球状のサンプルされた頂点の数だけ繰り返す
    for(uint i = 0;i < SAMPLE_COUNT;i++)
    {
        //
        vec2 Xi = Hammersley(i,SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi,normal,pushConstant.roughness);
        vec3 L = normalize(2.0 * dot(view,H) * H - view);

        float NdotL = max(dot(normal,L),0.0);
        if(NdotL > 0.0)
        {
            outColor += vec4(texture(hdriTextureMap,L).rgb * NdotL,0.0);
            totalWeight += NdotL;
        }
    }

    outColor = outColor / totalWeight;

    outColor.a = 1.0;
}