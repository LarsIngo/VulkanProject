struct Output
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

Output main(uint id : SV_VERTEXID)
{
    Output output;

    output.position = float4((id == 0 || id == 1) * 2 - 1, (id == 0 || id == 2) * 2 - 1, 0, 1);
    output.uv = float2((id == 0 || id == 1), (id == 1 || id == 3));

    return output;
}