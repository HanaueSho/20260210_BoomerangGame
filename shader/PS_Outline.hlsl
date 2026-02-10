/*
    PS_Outline.hlsl
    20251027  hanaue sho
*/
#include "common.hlsl"

void main(in VS_OUT IN, out float4 outDiffuse : SV_Target)
{
    // ’PF
    outDiffuse = float4(OutlineColor, 1.0f);
}
