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
    vec3 worldPosition = GSInput[0].position.xyz;
    vec3 color = GSInput[0].color.xyz;
    vec2 scale = GSInput[0].scale.xy;

    for (uint i = 0; i < 4; ++i)
    {
        uint x = uint(i == 1 || i == 3);
        uint y = uint(i == 0 || i == 1);
        
        gl_Position.xyz = worldPosition + vec3((x * 2.f - 1.f) * 0.1f, (y * 2.f - 1.f) * 0.1f, 0.5f);
        gl_Position.w = 1.f;
        GSOutput.position = gl_Position;
        GSOutput.worldPosition = gl_Position.xyz;
        GSOutput.color = color;
        GSOutput.uv = vec2(x, 1.f - y);
    
        EmitVertex();
    }
    
    EndPrimitive();

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
    //    output.position = mul(output.position, vpMatrix);
    //    output.color = color;
    //    output.uv = float2(x, 1.f - y);
    
    //    TriStream.Append(output);
    //}
}
