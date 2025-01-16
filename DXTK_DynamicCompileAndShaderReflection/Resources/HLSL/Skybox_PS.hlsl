#include "Skybox.hlsli"

float4 main(PS_Input input) : SV_TARGET
{
	return g_skybox.Sample(g_sampler, normalize(input.texcoord));
}