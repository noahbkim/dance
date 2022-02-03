#include "Constants.hlsl"

struct VIn
{
    float3 position : POSITION0;
    float3 normal : NORMAL0;
    float4 color : COLOR0;
    float2 textureUV : TEXCOORD0;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 textureUV : TEXCOORD0;
    float4 normal : NORMAL0;
    float4 worldPosition : TEXCOORD1;
};

VOut VS(VIn vIn)
{
    VOut output;
    output.worldPosition = mul(float4(vIn.position, 1.0), c_modelToWorld);
    output.position = mul(output.worldPosition, c_viewProj);
    output.normal = mul(float4(vIn.normal, 1.0), c_modelToWorld);
    output.color = vIn.color;
    output.textureUV = vIn.textureUV;
    return output;
}

float4 PS(VOut pIn) : SV_TARGET
{
    /*
    float3 normal = pIn.normal;
    normal = normalize(normal);

    float3 color = (0, 0, 0);
    [unroll(MAX_POINT_LIGHTS)] for (int i = 0; i < MAX_POINT_LIGHTS; ++i)
    {
        if (c_pointLight[i].isEnabled)
        {
            float3 pointToLight = pIn.worldPosition - c_pointLight[i].position;
            float step = 1 - smoothstep(c_pointLight[i].innerRadius, c_pointLight[i].outerRadius, length(pointToLight));
            color += step * c_pointLight[i].lightColor * saturate(dot(normal, normalize(c_pointLight[i].position - c_cameraPosition)));

            float3 lightDirection = normalize(pointToLight);
            float3 cameraDirection = normalize(c_cameraPosition - pIn.worldPosition);
            color += step * c_pointLight[i].lightColor * pow(saturate(dot(reflect(lightDirection, normal), cameraDirection)), c_specularPower);
        }
    }

    float4 sampled = DiffuseTexture.Sample(DefaultSampler, pIn.textureUV);
    */
    return pIn.color; // *sampled* float4(color, 1.0f);
}