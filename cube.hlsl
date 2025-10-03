cbuffer UBO : register(b0, space1)
{
    float4x4 u_ProjectionTransformMatrix;
    float4 u_Color;
    float2 u_Resolution;
    float u_Size;
};

struct VSInput
{
    float2 Position : TEXCOORD0;
};

struct VSOutput
{
    float4 Color : TEXCOORD0;
    float4 Position : SV_Position;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    output.Color = u_Color;
    output.Position = mul(u_ProjectionTransformMatrix, float4(input.Position, 1.0f, 1.0f));
    return output;
}

float4 PSMain(VSOutput input) : SV_Target0
{
    return input.Color;
}
