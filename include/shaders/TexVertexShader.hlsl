struct VS_INPUT
{
    float3 inPos: POSITION;
    float2 inTexCoord: TEXTCOORD;
};

struct VS_OUTPUT
{
    float4 outPosition: SV_POSITION;
    float2 outTexCoord: TEXTCOORD;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.outPosition = float4(input.inPos, 1.0f);
    output.outTexCoord = input.inTexCoord;
    return output;
}