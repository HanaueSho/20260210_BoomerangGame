/*
    VS_Outline.hlsl
    20251027  hanaue sho
    アウトライン用の頂点シェーダ（背面法）
*/
#include "common.hlsl"

void main(in VS_IN IN, out VS_OUT OUT)
{	
    float4 posW = mul(float4(IN.Position, 1.0f), World);
    float3 normalW = TransformNormalWorld(IN.Normal);
    
    // ワールド　→　ビュー
    float4 posV = mul(posW, View);
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
    // 値格納
    OUT.WorldPosition = posW.xyz;
    OUT.WorldNormal = normalW;
    OUT.Color = IN.Color;
    OUT.TexCoord = IN.TexCoord;
}