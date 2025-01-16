#include "Common.hlsli"


struct VS_Input
{
	float3 positionOS : SV_POSITION;
};

struct PS_Input
{
	float4 positionCS 	: SV_POSITION;
	float3 texcoord 	: TEXCOORD;
};