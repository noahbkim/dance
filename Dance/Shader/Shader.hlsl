struct VS_Input
{
    float3 pos : POS;
    float4 color : COL;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float4 color : COL;
};

VS_Output VS(VS_Input input)
{
    VS_Output output;
    output.position = float4(input.pos, 1.0f);
    output.color = input.color;

    return output;
}

float4 PS(VS_Output input) : SV_TARGET
{
    return input.color;
}
