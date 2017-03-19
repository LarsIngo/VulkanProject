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
struct MetaData
{
    mat4 vpMatrix;
    vec4 lensPosition;
    vec4 lensUpDirection;
};
// Meta buffer.
layout(binding = 1) buffer GSMetaData { MetaData g_MetaBuffer[]; };

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;
void main()
{
    MetaData metaData = g_MetaBuffer[0];
    mat4 vpMatrix = metaData.vpMatrix;
    vec3 lensPosition = metaData.lensPosition.xyz;
    vec3 lensUpDirection = metaData.lensUpDirection.xyz;

    vec3 worldPosition = GSInput[0].position.xyz;
    vec3 color = GSInput[0].color.xyz;
    vec2 scale = GSInput[0].scale.xy;

    vec3 particleFrontDirection = normalize(lensPosition - worldPosition);
    vec3 paticleSideDirection = cross(particleFrontDirection, lensUpDirection);
    vec3 paticleUpDirection = cross(paticleSideDirection, particleFrontDirection);

    for (uint i = 0; i < 4; ++i)
    {
        uint x = uint(i == 1 || i == 3);
        uint y = uint(i == 0 || i == 1);
        
        gl_Position.xyz = worldPosition + paticleSideDirection * (x * 2.f - 1.f) * scale.x + paticleUpDirection * (y * 2.f - 1.f) * scale.y;
        gl_Position.w = 1.f;
        GSOutput.position = gl_Position;
        GSOutput.worldPosition = gl_Position.xyz;
        GSOutput.color = color;
        GSOutput.uv = vec2(x, 1.f - y);

        gl_Position = gl_Position * vpMatrix;
        gl_Position.y = -gl_Position.y;
    
        EmitVertex();
    }
    
    EndPrimitive();
}
