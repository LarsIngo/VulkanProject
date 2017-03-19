#version 450

struct Particle
{
    vec4 position;
    vec4 velocity;
    vec4 color;
    vec4 scale;
};
layout(binding = 0) buffer VSInput { Particle g_Input[]; };

layout(location = 0) out Particle VSOutput;

void main()
{
    VSOutput = g_Input[gl_VertexIndex];
}
