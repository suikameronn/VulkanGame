#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV0;
layout (location = 3) in vec2 inUV1;
layout (location = 4) in vec4 inColor0;
layout (location = 5) in vec3 camPos;
layout(location = 6) in vec4 inShadowCoords;

layout (set = 1, binding = 0) uniform ShaderMaterial
{
	vec4 baseColorFactor;
	vec4 emissiveFactor;
	vec4 diffuseFactor;
	vec4 specularFactor;
	int baseColorTextureIndex;
	int physicalDescriptorTextureIndex;
	int normalTextureIndex;	
	int occlusionTextureIndex;
	int emissiveTextureIndex;
	float metallicFactor;	
	float roughnessFactor;	
	float alphaMask;	
	float alphaMaskCutoff;
	float emissiveStrength;
} shaderMaterial;

//Textures
layout (set = 1, binding = 1) uniform sampler2D colorMap;//色のテクスチャ
layout (set = 1, binding = 2) uniform sampler2D physicalDescriptorMap;//荒さはg、メタリックはbに含まれている
layout (set = 1, binding = 3) uniform sampler2D normalMap;//法線マップ
layout (set = 1, binding = 4) uniform sampler2D aoMap;
layout (set = 1, binding = 5) uniform sampler2D emissiveMap;//光のマップ

layout(set = 2, binding = 0) uniform PointLightUBO
{
	int lightCount;
	vec4[50] pos;
	vec4[50] color;
}pointLight;

layout(set = 3,binding = 0) uniform DirectionalLightUBO
{
	int lightCount;
	vec4[50] dir;
	vec4[50] color;
}directionalLight;

layout(set = 4,binding = 0) uniform sampler2D shadowMap;

layout(set = 5,binding = 0) uniform samplerCube diffuseMap;

layout(set = 6,binding = 0) uniform samplerCube specularReflectionMap;
layout(set = 7,binding = 0) uniform sampler2D specularBRDFMap;

layout(location = 0) out vec4 outColor;

const float directF0 = 0.4;
const float minimumRoughness = 0.04;

const float PI = 3.141592653589793;

vec4 srgbToLinear(vec4 srgbIn)//sRGBからリニアに変換
{
	return vec4(pow(srgbIn.xyz,vec3(2.2)),srgbIn.w);
}

vec4 linearToSrgb(vec4 srgbIn)//リニアからsRGBに変換
{
	return vec4(pow(srgbIn.xyz,vec3(1.0/2.2)),srgbIn.w);
}

//法線マップから法線を取得
vec3 getNormal(int texIndex)
{
    vec3 tangentNormal = texture(normalMap,texIndex == 0 ? inUV0 : inUV1).xyz * 2.0 - 1.0;

    vec3 q1 = dFdx(inPos);
    vec3 q2 = dFdy(inPos);
    vec2 st1 = dFdx(inUV0);
    vec2 st2 = dFdy(inUV0);

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

float shadowCalc(vec4 shadowCoord, vec2 off)
{
	float shadow = 1.0;

	if(shadowCoord.s >= 0.0 && shadowCoord.s <= 1.0 
	&& shadowCoord.t >= 0.0 && shadowCoord.t <= 1.0)
	{
		if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0) 
		{
			float dist = texture( shadowMap, shadowCoord.st + off ).r;
			if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
			{
				shadow = 0.1;
			}
		}
	}
	return shadow;
}

//粗さを考慮したフレネルの式
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

//IBLのライトの効果を計算
vec4 getIBL(vec3 f0,vec3 normal,vec3 view,vec3 reflection,vec3 baseColor,float roughness,float metallic)
{
	vec3 F = fresnelSchlickRoughness(max(dot(normal,view),0.0),f0,roughness);

	vec3 irradiance = texture(diffuseMap,normal).rgb;
	vec3 diffuse = irradiance * baseColor;

	const float MAX_REFLECTION_LOD = 4.0;
	vec3 prefilteredColor = textureLod(specularReflectionMap,reflection,roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf = texture(specularBRDFMap,vec2(max(dot(normal,view), 0.0),roughness)).rg;
	vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

	float ao = 1.0f;
	if(shaderMaterial.occlusionTextureIndex > -1)
	{
		ao = texture(aoMap,shaderMaterial.baseColorTextureIndex == 0 ? inUV0 : inUV1).r;
	}
	vec3 ambient = (((1.0 - F) * (1.0 - metallic) * diffuse) + specular) * ao;

	return vec4(ambient,0.0f);
}

void main() {
	float roughness;
	float metallic;
	vec3 diffuseColor;
	vec4 baseColor;

	outColor = vec4(0.0);

	vec3 f0 = vec3(0.04);//光が垂直に当たった時の反射率

	if(shaderMaterial.alphaMask == 1.0f)
	{
		//ベースカラーテクスチャから色を取得
		if(shaderMaterial.baseColorTextureIndex > -1)
		{
			baseColor = srgbToLinear(texture(colorMap,inUV0)) * shaderMaterial.baseColorFactor;
		}
		else
		{
			//テクスチャがない場合
			baseColor = shaderMaterial.baseColorFactor;
		}

		//α値が一定以下なら破棄
		if(baseColor.a < shaderMaterial.alphaMaskCutoff)
		{
			discard;//描画しない命令
		}
	}

	roughness = shaderMaterial.roughnessFactor;
	metallic = shaderMaterial.metallicFactor;
	if(shaderMaterial.physicalDescriptorTextureIndex > -1)
	{
		//roughnessはg、metallicはbチャンネルにある
		vec4 roughMetallic = texture(physicalDescriptorMap,shaderMaterial.physicalDescriptorTextureIndex == 0 ? inUV0:inUV1);
		roughness = roughness * roughMetallic.g;
		metallic = metallic * roughMetallic.b;
	}
	else
	{
		//テクスチャがない場合
		roughness = clamp(roughness,minimumRoughness,1.0);
		metallic = clamp(metallic,0.0,1.0);
	}

	// アルベドは、ベーステクスチャまたはフラットカラーから定義できます。
	if (shaderMaterial.baseColorTextureIndex > -1) {
		baseColor = srgbToLinear(texture(colorMap, shaderMaterial.baseColorTextureIndex == 0 ? inUV0 : inUV1)) * shaderMaterial.baseColorFactor;
	}
	else
	{
		baseColor = shaderMaterial.baseColorFactor;
	}

	baseColor *= inColor0;

	diffuseColor = baseColor.rgb * (vec3(1.0) - f0);
	diffuseColor *= (1.0 - metallic);

	float alphaRoughness = roughness * roughness;

	vec3 specularColor = mix(f0,baseColor.rgb,metallic);

	//反射率を計算する
	float reflectance = max(max(specularColor.r,specularColor.g),specularColor.b);

	//計算簡略化のため、ほとんどの場合の反射をグレージング反射とする
	float reflectance90 = clamp(reflectance * 25.0,0.0,1.0);
	vec3 specularEnvironmentR0 = specularColor.rgb;//完全反射色
	vec3 specularEnvironmentR90 = vec3(1.0) * reflectance90;//斜め角からの反射色

	vec3 n = (shaderMaterial.normalTextureIndex > -1) ? getNormal(shaderMaterial.normalTextureIndex) : normalize(inNormal);
	n.y *= -1.0;
	vec3 v = normalize(camPos - inPos);//頂点からカメラへのベクトル
	vec3 reflection = normalize(reflect(-v,n));//鏡面反射の向きを求める

	for(int i = 0;i < pointLight.lightCount;i++)
	{
		vec3 l = normalize(pointLight.pos[i].xyz - inPos);//頂点からライトへのベクトル
		float dis = length(pointLight.pos[i].xyz - inPos);
		vec3 h = normalize(l+v);//lとvの中間のベクトル

		float NdotL = clamp(dot(n, l), 0.001, 1.0);
		float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);
		float NdotH = clamp(dot(n, h), 0.0, 1.0);
		float LdotH = clamp(dot(l, h), 0.0, 1.0);
		float VdotH = clamp(dot(v, h), 0.0, 1.0);

		// マイクロファセット鏡面反射シェーディングモデルのシェーディング項を計算する
		vec3 F = specularReflection(specularEnvironmentR0,specularEnvironmentR90,VdotH);
		float G = geometricOcclusion(alphaRoughness,NdotL,NdotV);
		float D = microfacetDistribution(alphaRoughness,NdotH);

		//拡散と鏡面の計算
		vec3 diffuseReflect = (1.0 - F) * diffuse(diffuseColor) / dis;
		vec3 specularReflect = F * G * D / (4.0 * NdotL * NdotV) / dis;
		// 最終的な強度を、光のエネルギー（余弦則）でスケーリングされた反射率（BRDF）として取得する。
		vec3 color = NdotL * (pointLight.color[i].rgb) * (diffuseReflect + specularReflect);
		outColor += vec4(color,0.0f);
	}

	for(int i = 0;i < directionalLight.lightCount;i++)
	{
		vec3 l = normalize(directionalLight.dir[i].xyz);//頂点からライトへのベクトル
		vec3 h = normalize(l+v);//lとvの中間のベクトル

		float NdotL = clamp(dot(n, l), 0.001, 1.0);
		float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);
		float NdotH = clamp(dot(n, h), 0.0, 1.0);
		float LdotH = clamp(dot(l, h), 0.0, 1.0);
		float VdotH = clamp(dot(v, h), 0.0, 1.0);

		// マイクロファセット鏡面反射シェーディングモデルのシェーディング項を計算する
		vec3 F = specularReflection(specularEnvironmentR0,specularEnvironmentR90,VdotH);
		float G = geometricOcclusion(alphaRoughness,NdotL,NdotV);
		float D = microfacetDistribution(alphaRoughness,NdotH);

		//拡散と鏡面の計算
		vec3 diffuseReflect = (1.0 - F) * diffuse(diffuseColor);
		vec3 specularReflect = F * G * D / (4.0 * NdotL * NdotV);
		// 最終的な強度を、光のエネルギー（余弦則）でスケーリングされた反射率（BRDF）として取得する。
		vec3 color = NdotL * (directionalLight.color[i].rgb) * (diffuseReflect + specularReflect);
		outColor += vec4(color,0.0f);
	}
	
	outColor += getIBL(f0,n,v,reflection,baseColor.rgb,roughness,metallic);

	float shadow = shadowCalc(inShadowCoords / inShadowCoords.w,vec2(0.0));
	outColor *= shadow;

	outColor.a = baseColor.a;
}