#include "PBRLit.hlsli"


PS_Input main(VS_Input input)
{
	PS_Input output = (PS_Input)0;
	
	float4 pos = float4(input.positionOS.xyz, 1);
	
	pos = mul(pos, m_matWorld);
	output.positionWS = pos.xyz;
	pos = mul(pos, m_matView);
	pos = mul(pos, m_matProj);
	output.positionCS = pos;
	
	output.uv = input.uv;
	
#ifdef MODELTYPE_CMO
	output.uv.y *= -1;
#else
	output.uv.x *= -1;
#endif
	
	output.normalWS = mul(float4(input.normalOS.xyz, 0), m_matWorld).xyz;
	output.tangentWS = mul(float4(input.tangentOS.xyz, 0), m_matWorld).xyz;
	output.binormalWS = cross(output.tangentWS, output.normalWS);

	
	return output;
}