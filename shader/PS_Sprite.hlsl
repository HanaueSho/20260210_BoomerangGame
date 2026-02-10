/*
    PS_Sprite.hlsl
    20260209  hanaue sho
    スプライト用
*/
#include "common.hlsl"

void main(in VS_OUT IN, out float4 outDiffuse : SV_Target)
{
    // とりあえずテクスチャ×色
    outDiffuse = (MatTextureEnable != 0) ? MainTex.Sample(MainSamp, IN.TexCoord) : float4(1, 1, 1, 1);
    outDiffuse *= IN.Color;
    outDiffuse.rgb *= outDiffuse.a; // これ要る？（ないと縁が汚くなるみたい）
}
