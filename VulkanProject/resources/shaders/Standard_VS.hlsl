struct Input
{
    float3 position : POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};
 
struct Output
{
    float3 position : POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

Output main(Input input)
{
    Output output;
 
    output.position = input.position;
    output.uv = input.uv;
    output.normal = input.normal;
    output.tangent = input.tangent;

    return output;
}
 