#version 450

layout(early_fragment_tests) in;

// Input.
struct PSInputStruct
{
    vec4 position;
    vec3 worldPosition;
    vec3 color;
    vec2 uv;
};
layout(location = 0) in PSInputStruct PSInput;

// Output.
layout(location = 0) out vec4 PSOutput0;

void main()
{
    float x = PSInput.uv.x - 0.5f;
    float y = PSInput.uv.y - 0.5f;
    float r = sqrt(x * x + y * y);
    float factor = max(1.f - r * 2.f, 0.f); //[1,0]
    float sinFactor = 1.f - sin(3.14159265f / 2.f * (factor + 1.f));

    PSOutput0 = vec4(PSInput.color, sinFactor);
}
