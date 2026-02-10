/*
    unlitTexturePS.hlsl
    20251008  hanaue sho
*/
#include "common.hlsl"

void main(in VS_OUT IN, out float4 outDiffuse : SV_Target)
{
    if (MatTextureEnable)
    {
        outDiffuse = MainTex.Sample(MainSamp, IN.TexCoord);
        outDiffuse *= IN.Color;
    }
    else
    {
        outDiffuse = IN.Color;
    }
	
    // マテリアルの色を反映
    outDiffuse *= MatDiffuse;
    
	
    // デバッグ用
    //outDiffuse = MainTex.Sample(MainSamp, IN.TexCoord);
    // outDiffuse = In.Diffuse;
}
