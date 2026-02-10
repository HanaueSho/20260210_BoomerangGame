/*
    VS_Lit.hlsl
    20251017  hanaue sho
    オーソドックスな頂点シェーダ
*/
#include "common.hlsl"

void main(in VS_IN IN, out VS_OUT OUT)
{	
    float4 wp = mul(float4(IN.Position, 1.0f), World);
    OUT.SVPos         = mul(wp, ViewProj);
    OUT.WorldPosition = wp.xyz;
    OUT.WorldNormal   = TransformNormalWorld(IN.Normal);
    OUT.Color         = IN.Color;
    OUT.TexCoord      = IN.TexCoord;
}