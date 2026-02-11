struct VSOutput {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

VSOutput main(uint vertex_id : SV_VERTEXID)
{
    VSOutput output;

    static const float2 positions[3] = {
        float2(-0.5f, -0.5f),
        float2(+0.5f, -0.5f),
        float2(+0.0f, +0.5f),
    };

    output.position = float4(positions[vertex_id], 0.0f, 1.0f);
    output.texcoord = float2(positions[vertex_id].xy + 0.5f);

    return output;
}
