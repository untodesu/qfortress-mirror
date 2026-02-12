Texture2D t_diffuse : register(t0, space2);
SamplerState s_diffuse : register(s0, space2);

float4 main(float2 texcoord : TEXCOORD0) : SV_TARGET
{
    return t_diffuse.Sample(s_diffuse, texcoord);
}
