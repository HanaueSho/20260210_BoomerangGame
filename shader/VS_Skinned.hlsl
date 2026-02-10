/*
    VS_Skinned.hlsl
    20251216  hanaue sho
    スキニング専用頂点シェーダ
*/
#include "common.hlsl"

void main(in VS_IN_SKINNED IN, out VS_OUT OUT)
{	
    // ローカル頂点
    float4 p = float4(IN.Position, 1.0f);
    float4 n = float4(IN.Normal, 0.0f);
    
    // スキニング結果
    float4 skinnedPos    = 0.0f;
    float4 skinnedNormal = 0.0f;
    
    // 重みの合計を後でチェック
    float weightSum = 0.0f;
    
    // ４本のボーンを合成
    [unroll]
    for (int i = 0; i < 4; i++)
    {
        uint idx = IN.BoneIndex[i];
        float w  = IN.BoneWeight[i];

        if (w <= 0.0f || idx >= (uint) BoneCount)
            continue;
        
        float4x4 B = BoneMatrixes[idx];
        
        skinnedPos    += mul(p, B) * w;
        skinnedNormal += mul(n, B) * w;
        weightSum     += w;
    }
    // BoneWeight が全部０だったらフォールバック
    if (weightSum == 0.0f)
    {
        skinnedPos = p;
        skinnedNormal = n;
    }
    
    // 一応ｗ成分を制御
    skinnedPos.w = 1.0f;
    skinnedNormal.w = 0.0f;
    
    float4 wp = mul(skinnedPos, World);
    //float4 wp = mul(p, World);
    OUT.SVPos         = mul(wp, ViewProj);
    OUT.WorldPosition = wp.xyz;
    OUT.WorldNormal = TransformNormalWorld(skinnedNormal.xyz);
    //OUT.WorldNormal   = TransformNormalWorld(IN.Normal.xyz);
    OUT.TexCoord      = IN.TexCoord;
    OUT.Color         = float4(1, 1, 1, 1);    
}