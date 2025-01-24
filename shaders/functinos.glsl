const float PI = 3.141592653589793;

vec4 srgbToLinear(vec4 srgbIn)//sRGBからリニアに変換
{
	return vec4(pow(srgbIn.xyz,vec3(2.2)),srgbIn.w);
}

vec4 linearToSrgb(vec4 srgbIn)//リニアからsRGBに変換
{
	vec3 linOut = pow(srgbIn.xyz,vec3(1.0/2.2));
}

//法線マップから法線を取得
vec3 getNormal(ShaderMaterail shaderMaterail)
{
    vec3 tangentNormal = texture(normalMap,shaderMaterial.normalTextureIndex == 0 ? inUV0 : inUV1).xyz * 2.0 - 1.0;

    vec3 q1 = dFbx(inPos);
    vec3 q2 = dFby(inPos);
    vec2 st1 = dFbx(inUV0);
    vec2 st2 = dFby(inUV0);

    vec3 N = normalize(inNormal);
    vec3 T = normalize(q1 * st2.t - q2 * st1.t);
    vec3 B = -normalize(cross(N,T));
    mat3 TBN = mat3(T,B,N);//法線をモデル空間へ変換する行列

    return normalize(TBN * tangentNormal);
}

vec3 specularReflection(vec3 reflectanceR0,vec3 reflectanceR90,float VdotH)
{
	return reflectanceR0 + (reflectanceR90 - reflectanceR0) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);
}

float geometricOcclusion(float alphaRoughness,float NdotL,float NdotV)
{
    float a2 = alphaRoughness * alphaRoughness;

	float attenuationL = 2.0 * NdotL / (NdotL + sqrt(a2 + (1.0 - a2) * (NdotL * NdotL)));
	float attenuationV = 2.0 * NdotV / (NdotV + sqrt(a2 + (1.0 - a2) * (NdotV * NdotV)));
	return attenuationL * attenuationV;
}

float microfacetDistribution(float alphaRoughness,float NdotH)
{
	float a2 = alphaRoughness * alphaRoughness;
	float f = (NdotH * a2 - NdotH) * NdotH + 1.0;
	return a2 / (PI * f * f);
}

vec3 diffuse(vec3 diffuseColor)
{
    return diffuseColor / PI;
}