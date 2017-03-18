#version 450

// Input.
struct GSInputStruct
{
    vec4 position;
    vec4 velocity;
    vec4 color;
    vec4 scale;
};
layout(location = 0) in GSInputStruct GSInput[];

// Output.
struct GSOutputStruct
{
    vec4 position;
    vec3 worldPosition;
    vec3 color;
    vec2 uv;
};
layout(location = 0) out GSOutputStruct GSOutput;

// Meta data.
//struct MetaData
//{
//    float4x4 vpMatrix;
//    float3 lensPosition;
//    float3 lensUpDirection;
//    float pad[2];
//};
// Meta buffer.
//StructuredBuffer<MetaData> g_MetaBuffer : register(t0);

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;
void main()
{
    gl_Position = GSInput[0].position;
    GSOutput.position = GSInput[0].position;
    GSOutput.worldPosition = GSInput[0].position.xyz;
    GSOutput.color = GSInput[0].color.xyz;
    GSOutput.uv = GSInput[0].scale.xy;



    //GSOutput output;

    //MetaData metaData = g_MetaBuffer[0];
    //float4x4 vpMatrix = metaData.vpMatrix;
    //float3 lensPosition = metaData.lensPosition;
    //float3 lensUpDirection = metaData.lensUpDirection;

    //float3 worldPosition = input[0].position.xyz;
    //float3 color = input[0].color.xyz;
    //float2 scale = input[0].scale.xy;

    //float3 particleFrontDirection = normalize(lensPosition - worldPosition);
    //float3 paticleSideDirection = cross(particleFrontDirection, lensUpDirection);
    //float3 paticleUpDirection = cross(paticleSideDirection, particleFrontDirection);

    //for (uint i = 0; i < 4; ++i)
    //{
    //    float x = i == 1 || i == 3;
    //    float y = i == 0 || i == 1;
    //    output.position.xyz = worldPosition + paticleSideDirection * (x * 2.f - 1.f) * scale.x + paticleUpDirection * (y * 2.f - 1.f) * scale.y;
    //    output.position.w = 1.f;
    //    output.worldPosition = output.position.xyz;
    //     output.position = mul(output.position, vpMatrix);
    //    output.color = color;
    //    output.uv = float2(x, 1.f - y);
    
    //    TriStream.Append(output);
    //}
}
