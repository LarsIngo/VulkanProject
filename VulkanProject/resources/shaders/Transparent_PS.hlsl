struct Input
{
    float4 position : SV_POSITION;
    float3 worldPosition : WORLDPOSITION;
    float2 uv : UV;
    float3x3 tbn : TBN;
};

struct Output
{
    float4 color : SV_TARGET0;
    float depth : SV_TARGET1;
};

Texture2D txAlbedo : register(t0);
Texture2D txNormal : register(t1);
Texture2D txGloss : register(t2);
Texture2D txScreen : register(t3);
Texture2D txScreenDepth : register(t4);

struct Meta
{
    float3 cameraPosition;
	uint skyboxMipLevels;
	float4x4 vpMatrix;
	uint screenWidth;
	uint screenHeight;
	float2 pad;
};
StructuredBuffer<Meta> g_Meta : register(t7);

TextureCube txSkybox : register(t8);

SamplerState samp : register(s0);

Output main(Input input) : SV_TARGET
{
    Meta meta = g_Meta[0];
	float3 cameraPosition = meta.cameraPosition;

    float3 worldPosition = input.worldPosition;
    float2 uv = input.uv;

    float3 color = txAlbedo.Sample(samp, uv).rgb;
    float3 normal = mul(normalize(2.f * txNormal.Sample(samp, uv).rgb - 1.f), input.tbn);
	float gloss = txGloss.Sample(samp, uv).x;
	float2 screenUV = float2((input.position.x / 2.f + 1.f) / meta.screenWidth * 2.f, (input.position.y / 2.f + 1.f) / meta.screenHeight * 2.f);

	float3 cameraVec = normalize(cameraPosition - worldPosition);
	float3 reflectVec = reflect(-cameraVec, normal);
	
	// TMP POINTLIGHT
	float3 power = 5.f;
	float3 lPosition = float3(5, 5, -5);
	float3 lColor = 1.f;
	float3 lightVec = lPosition - worldPosition;
	float len = length(lightVec);
	lightVec = normalize(lightVec);

	float3 lightColor;
	float3 specular;
	float specularFactor;
	{
		float colorFactor = saturate(dot(lightVec, normal));
		float k = len + 1;
		float distanceFactor = 1;//power / (k * k);
		lightColor = colorFactor * distanceFactor * color;
		float r = saturate(dot(reflectVec, lightVec));
		r = pow(r, 5.f + 5.f * gloss);
		specular = (1.f - colorFactor) * lColor * r;
		specularFactor = r;
	}

	float3 reflectColor;
	{
		float3 skybox = txSkybox.Sample(samp, reflectVec).rgb;
		float r = saturate(dot(cameraVec, normal));
		float f = 0.22f;
		float fresnel = f + (1.f - f) * pow(1.f - r, 5);
        reflectColor = skybox * fresnel * (1.f - specularFactor);
	}

	float3 refractColor;
    float2 refractUV;
	{
        float refractDepth = dot(cameraVec, normal) * 0.35f;
        float3 refractVec = refract(-cameraVec, normal, 0.14f) * refractDepth;
		float2 offsetUV = mul(float4(refractVec, 1.f), meta.vpMatrix).xy;
        refractUV = screenUV + offsetUV;
        refractUV = saturate(refractUV);
        refractColor = txScreen.Sample(samp, refractUV).rgb;

        //http://answers.playcanvas.com/questions/679/how-do-i-convert-world-space-coordinates-into-screen-space
        //float refractDepth = txScreenDepth.Sample(samp, refractUV).x;
        //float pxDepth = input.position.z / input.position.w;
        //if (pxDepth > refractDepth)
        //    refractColor = txScreen.Sample(samp, refractUV).rgb;
        //else
        //    refractColor = txSkybox.Sample(samp, -cameraVec + refractVec).rgb;
	}




    float3 finalColor = specular + refractColor + reflectColor;

	// Tone mapping
	//finalColor = finalColor / (finalColor + 1);

    Output output;
    output.color = float4(finalColor, 1.f);
    output.depth = input.position.z / input.position.w;
    return output;
}
