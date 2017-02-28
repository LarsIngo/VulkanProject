struct Input
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

struct Meta
{
    float4x4 opMatrixINV;
};

TextureCube txSkybox : register(t0);
StructuredBuffer<Meta> g_Meta : register(t1);

SamplerState samp : register(s0);

float4 main(Input input) : SV_TARGET0
{
    Meta meta = g_Meta[0];
    float3 uvw = mul(float4(input.uv.x * 2 - 1, (1 - input.uv.y) * 2 - 1, 0.f, 1.f), meta.opMatrixINV).xyz;
    float3 skybox = txSkybox.Sample(samp, uvw).rgb;
    return float4(skybox, 1.f);
}
