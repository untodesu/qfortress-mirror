float4 main(float2 texcoord : TEXCOORD0) : SV_TARGET
{
    return float4(texcoord, 1.0f, 1.0f);
}
