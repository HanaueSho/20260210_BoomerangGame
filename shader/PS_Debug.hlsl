/*
    PS_Debug.hlsl
    20251130  hanaue sho
*/
#include "common.hlsl"

void main(in VS_OUT_DEBUG IN, out float4 outDiffuse : SV_Target)
{
    outDiffuse.rgb = IN.Color;
    outDiffuse.a = IN.Color.a;
}
