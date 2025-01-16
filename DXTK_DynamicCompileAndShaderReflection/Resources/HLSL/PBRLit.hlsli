#include "Common.hlsli"

#define MODEL_SHADER


// 定数バッファ
cbuffer PBRLitConstantBuffer
{
	float4 c_baseColor;
	float f_metallic;
	float f_smoothness;
	float t_useAlbedoTexture;
	float t_useNormalMap;
	float t_useMetallicMap;
};


// 頂点入力（インプットレイアウト）
#ifdef MODELTYPE_CMO
	struct VS_Input
	{
		float3 positionOS : SV_Position;
		float3 normalOS : NORMAL;
		float4 tangentOS : TANGENT;
		float color : COLOR;
		float2 uv : TEXCOORD;
	};
#else
	struct VS_Input
	{
		float3 positionOS 	: SV_POSITION;
		float3 normalOS 	: NORMAL;
		float2 uv 			: TEXCOORD;
		float3 tangentOS 	: TANGENT;
	};
#endif

// ピクセル入力
struct PS_Input
{
	float4 positionCS 	: SV_POSITION;
	float3 normalWS 	: NORMAL;
	float3 tangentWS	: TANGENT;
	float3 binormalWS 	: BINORMAL;
	float4 color 		: COLOR;
	float2 uv 			: TEXCOORD;

	float3 positionWS   : TEXCOORD2;
};