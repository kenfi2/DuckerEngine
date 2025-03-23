cbuffer UBO : register(b0, space1)
{
    float4x4 ModelViewProj;
};

Texture2D<float4> Texture : register(t0, space2);
SamplerState Sampler : register(s0, space2);

struct VSInput
{
    float2 Position : TEXCOORD0;
    float2 TexCoord : TEXCOORD1;
};

struct VSOutput
{
    float2 TexCoord : TEXCOORD0;
    float4 Position : SV_Position;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    output.TexCoord = input.TexCoord;
    output.Position = mul(ModelViewProj, float4(input.Position, 1.0f, 1.0f));
    return output;
}

float4 PSMain(float2 TexCoord : TEXCOORD0) : SV_Target0
{
    return Texture.Sample(Sampler, TexCoord);
}
