/**********************************************
 *
 *  PBRLit_PS.hlsl
 *  物理ベースレンダリング
 *  Cook-Torrance(GGX, Height-Correlated Smith, Schlick)
 *  Disney Diffuse
 *  参考：https://light11.hatenadiary.com/entry/2020/03/05/220957
 *
 *  製作者：牛丸 文仁
 *
 **********************************************/
#include "PBRLit.hlsli"

// ベーステクスチャ
Texture2D<float4> g_albedo : register(t1);
// ノーマルマップ
Texture2D<float4> g_normal : register(t2);
// メタリックマップ R値：Metallic, A値：Smoothness
Texture2D<float4> g_metallic : register(t3);


static const float DIELECTRICF0 = 0.04f;


// Cook-TorranceのD項をGGXで求める
float D_GGX(float ndoth, float alpha)
{
	float a2 = alpha * alpha;
	float d = (ndoth * a2 - ndoth) * ndoth + 1.0f;
	return a2 / (d * d + 0.0000001) * (1.0 / F_PI);
}
float D_GGX(float perceptualRoughness, float ndoth, float3 normalWS, float3 halfDir)
{
	float3 NcrossH = cross(normalWS, halfDir);
	float a = ndoth * perceptualRoughness;
	float k = perceptualRoughness / (dot(NcrossH, NcrossH) + a * a);
	float d = k * k * (1.0f / F_PI);
	return min(d, 65504.0);
}

// Cook-TorranceのV項をHeight-Correlated Smithモデルで求める
float V_SmithGGXCorrelated(float ndotl, float ndotv, float alpha)
{
	float lambdaV = ndotl * (ndotv * (1 - alpha) + alpha);
	float lambdaL = ndotv * (ndotl * (1 - alpha) + alpha);
	return 0.5f / (lambdaV + lambdaL + 0.0001);
}

// Cook-TorranceのF項をSchlickの近似式で求める
float3 F_Schlick(float3 f0, float cos)
{
	return f0 + (1 - f0) * pow(1 - cos, 5);
}


// Disneyのモデルで拡散反射を求める
float Fd_Burley(float ndotv, float ndotl, float ldoth, float roughness)
{
	float fd90 = 0.5 + 2 * ldoth * ldoth * roughness;
	float lightScatter = (1 + (fd90 - 1) * pow(1 - ndotl, 5));
	float viewScatter = (1 + (fd90 - 1) * pow(1 - ndotv, 5));

	float diffuse = lightScatter * viewScatter;
	// diffuse /= F_PI;
	return diffuse;
}


// BRDF式
float4 BRDF(
	float3 albedo,
	float metallic,
	float perceptualRoughness,
	float3 normalWS,
	float3 viewDir,
	float3 lightDir,
	float3 lightColor,
	float3 indirectSpecular
)
{
	float3 halfDir = normalize(lightDir + viewDir);
	float NdotV = abs(dot(normalWS, viewDir));
	float NdotL = max(0, dot(normalWS, lightDir));
	float NdotH = max(0, dot(normalWS, halfDir));
	float LdotH = max(0, dot(lightDir, halfDir));
	float reflectivity = lerp(DIELECTRICF0, 1, metallic);
	float3 f0 = lerp(DIELECTRICF0, albedo, metallic);

	// 拡散反射
	float diffuseTerm = Fd_Burley(
		NdotV,
		NdotL,
		LdotH,
		perceptualRoughness
	) * NdotL;
	float3 diffuse = albedo * (1 - reflectivity) * lightColor * diffuseTerm;
	// 関節拡散反射
	diffuse += albedo * (1 - reflectivity) * c_ambientLightColor.rgb * f_ambientLightIntensity;
	
	// 鏡面反射
	float alpha = perceptualRoughness * perceptualRoughness;
	float V = V_SmithGGXCorrelated(NdotL, NdotV, alpha);
	float D = D_GGX(NdotH, alpha);
	float3 F = F_Schlick(f0, LdotH);
	float3 specular = V * D * F * NdotL * lightColor;
	specular *= F_PI;
	specular = max(0, specular);
	// 環境反射光
	float surfaceReduction = 1.0 / (alpha * alpha + 1);
	float f90 = saturate((1 - perceptualRoughness) + reflectivity);
	specular += surfaceReduction * indirectSpecular * lerp(f0, f90, pow(1 - NdotV, 5));
	
	
	float3 color = diffuse + specular;
	return float4(color, 1);
}



float4 main(PS_Input input) : SV_TARGET
{
	// ノーマルマップのサンプリングと算出
	float3 localNormal = g_normal.Sample(g_sampler, input.uv).xyz;
	localNormal = (localNormal - 0.5f) * 2.0f;
	float3 normalWS = input.tangentWS * localNormal.x + input.binormalWS * localNormal.y + input.normalWS * localNormal.z;
	// ノーマルマップを使用しないときは入力のノーマルをそのまま使用
	normalWS = normalize(lerp(input.normalWS, normalWS, t_useNormalMap));
	
	// 視点からのベクトルを取得
	float3 viewDir = normalize(f_cameraPositionWS.xyz - input.positionWS);

	// 拡散反射光の色をサンプリング
	float3 albedo = c_baseColor.rgb;
	albedo *= lerp((float3)1, g_albedo.Sample(g_sampler, input.uv).rgb, t_useAlbedoTexture);
	
	// メタリックマップのサンプリング
	float4 metallicMap = g_metallic.Sample(g_sampler, input.uv);
	float metallic = lerp(f_metallic, metallicMap.r, t_useMetallicMap);
	float smoothness = lerp(f_smoothness, metallicMap.a, t_useMetallicMap);
	
	// metallic = f_metallic;
	// smoothness = f_smoothness;
	
	// 鏡面反射光の色をサンプリング
	float3 refVec = reflect(viewDir, normalWS);
	refVec.y *= -1;
	float3 indirectSpecular = g_skybox.SampleLevel(g_sampler, refVec, smoothness * 12).rgb;

	float4 color = BRDF(
		albedo,
		metallic,
		smoothness,
		normalWS,
		viewDir,
		-f_mainLightDirection,
		c_mainLightColor.rgb,
		indirectSpecular
		);
	
	return color;
}