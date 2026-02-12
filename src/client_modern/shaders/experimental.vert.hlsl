struct VSInput {
    float3 position : POSITION;
    float2 texcoord : TEXCOORD0;
};

struct VSOutput {
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

cbuffer Uniforms : register(b0, space1) {
    float4x4 u_mvp;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    output.position = mul(u_mvp, float4(input.position, 1.0f));
    output.texcoord = input.texcoord;

    return output;
}
