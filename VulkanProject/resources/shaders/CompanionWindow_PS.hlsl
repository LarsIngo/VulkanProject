Texture2D txLeftEye : register(t0);
Texture2D txRightEye : register(t1);

SamplerState samp : register(s0);

struct Input
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

float4 main(Input input) : SV_TARGET0
{
    float2 uv = input.uv;
    float3 color = float4(0.f, 0.f, 0.f, 0.f);
    // Left eye.
    if (input.uv.x < 0.5f)
    {
        uv.x *= 2.f;
        color = txLeftEye.Sample(samp, uv).rgb;
    }
    // Right eye.
    else
    {
        uv.x = (uv.x - 0.5f) * 2.f;
        color = txRightEye.Sample(samp, uv).rgb;
    }

    return float4(color, 1.f);
}
