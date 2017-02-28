struct Input
{
    float3 position : POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};
 
struct Output
{
    float4 position : SV_POSITION;
    float3 worldPosition : WORLDPOSITION;
    float2 uv : UV;
    float3x3 tbn : TBN;
};

struct Meta
{
    float4x4 modelMatrix;
    float4x4 mvpMatrix;
};
StructuredBuffer<Meta> g_Meta : register(t0);

float3x3 CalculateTBN(float3 normal, float3 tangent)
{
    float3 n = normalize(normal);
    float3 t = normalize(tangent);
    t = normalize(t - dot(t, n) * n);
    float3 b = cross(n, t);
    if (dot(cross(n, t), b) < 0.f)
        t = -t;
    return float3x3(t, b, n);
}

[maxvertexcount(3)]
void main(triangle Input input[3], inout TriangleStream<Output> outStream)
{
    Output output;

    Meta meta = g_Meta[0];
 
    for (uint i = 0; i < 3; ++i)
    {
        output.position = mul(float4(input[i].position, 1.f), meta.mvpMatrix);
        output.worldPosition = mul(float4(input[i].position, 1.f), meta.modelMatrix).xyz;
        output.uv = input[i].uv;
        output.tbn = CalculateTBN(input[i].normal, input[i].tangent);
        outStream.Append(output);
    }
}
 