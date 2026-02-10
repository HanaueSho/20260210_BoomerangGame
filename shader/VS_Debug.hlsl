/*
    VS_Debug.hlsl
    20251130  hanaue sho
*/
#include "common.hlsl"

void main(in VS_IN_DEBUG IN, out VS_OUT_DEBUG OUT)
{
    float4 wp = float4(IN.Position, 1.0f);
    
    OUT.Position = mul(wp, ViewProj);
    OUT.Color = IN.Color;
}
