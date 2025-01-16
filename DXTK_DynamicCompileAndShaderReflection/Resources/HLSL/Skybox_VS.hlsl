#include "Skybox.hlsli"

PS_Input main(VS_Input input)
{
	PS_Input output = (PS_Input)0;

	float4 pos = float4(input.positionOS, 1);
	pos = mul(pos, m_matWorld);

	matrix matView = m_matView;
    matView._41 = 0;
    matView._42 = 0;
    matView._43 = 0;
	
	pos = mul(pos, matView);
	
	pos = mul(pos, m_matProj);
	pos.z = pos.w;
	output.positionCS = pos;

	output.texcoord = input.positionOS.xyz;
	output.texcoord.x *= -1;
	
	return output;
}