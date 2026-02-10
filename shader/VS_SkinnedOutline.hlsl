/*
    VS_SkinnedOutline.hlsl
    20260120  hanaue sho
    スキニングアウトライン専用頂点シェーダ
*/
#include "common.hlsl"

void main(in VS_IN_SKINNED IN, out VS_OUT OUT)
{
    // ローカル頂点
    float4 p = float4(IN.Position, 1.0f);
    float4 n = float4(IN.Normal, 0.0f);
    
    // スキニング結果
    float4 skinnedPos = 0.0f;
    float4 skinnedNormal = 0.0f;
    
    // 重みの合計を後でチェック
    float weightSum = 0.0f;
    
    // ４本のボーンを合成
    [unroll]
    for (int i = 0; i < 4; i++)
    {
        uint idx = IN.BoneIndex[i];
        float w = IN.BoneWeight[i];

        if (w <= 0.0f || idx >= (uint) BoneCount)
            continue;
        
        float4x4 B = BoneMatrixes[idx];
        
        skinnedPos += mul(p, B) * w;
        skinnedNormal += mul(n, B) * w;
        weightSum += w;
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
    
    // ----- アウトライン処理 -----
    // ワールド　→　ビュー
    float4 posW = mul(skinnedPos, World);
    float4 posV = mul(posW, View);
    skinnedNormal.xyz = normalize(skinnedNormal.xyz); // 正規化
    skinnedNormal.w = 0.0f;
    float3 normalW = TransformNormalWorld(skinnedNormal.xyz);
    float3 normalV = normalize(mul(float4(normalW, 0.0f), View).xyz);
    
    // Gravity Daze 的距離補正（orthの時は scale = 1 にする）
    float dist = abs(posV.z); // ビュー空間距離
    float fovScale = 1.0 / tan(FOV * 0.5); // 投影倍率
    float scale = dist * fovScale * 0.035; // 調整係数（おおよそ 0.001 ~ 0.005 と言われたがこれに拘る必要はない）
    if (dist > 20) // 一定の距離を超えたら scale = 1 にする（こうしないとアウトラインに潰される）
        scale = 1; // しかし、さらに遠くなると depthBias の影響でアウトラインに潰される
    
    
    // 法線方向に膨張
    bool isWorld = false; // ビューの方がキレイ？今後消すかも
    if (isWorld) //（ワールド空間）
    {
        posW.xyz += normalW * OutlineWidth * scale;
        OUT.SVPos = mul(posW, ViewProj);
    }
    else //（ビュー空間）
    {
        posV.xyz += normalV * OutlineWidth * scale;
        OUT.SVPos = mul(posV, Proj);
    }
    
    OUT.WorldPosition = posW.xyz;
    OUT.WorldNormal = normalW;
    OUT.TexCoord = IN.TexCoord;
    OUT.Color = float4(1, 1, 1, 1);
}