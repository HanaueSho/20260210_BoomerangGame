/*
	TransformComponent.h
	20250813  hanaue sho
	Transformを持っているコンポーネント
*/
#ifndef TRANSFORMCOMPONENT_H_
#define TRANSFORMCOMPONENT_H_
#include <vector>
#include <algorithm>
#include <cmath>
#include "Component.h"
#include "Transform.h"
#include "Matrix4x4.h"

#include <type_traits>
class TransformComponent : public Component
{
protected:
    // ==================================================
    // ----- 要素 -----
    // ==================================================
    // --------------------------------------------------
    // データ本体
    // --------------------------------------------------
	Transform m_Value{}; // 既存の Transform をそのまま内包
    // --------------------------------------------------
    // 親子関係（非所有）
    // --------------------------------------------------
    TransformComponent* m_pParent = nullptr;
    std::vector<TransformComponent*> m_Children;
    // --------------------------------------------------
    // キャッシュ
    // --------------------------------------------------
    mutable Matrix4x4 m_Local{};
    mutable Matrix4x4 m_World{};
    mutable Matrix4x4 m_WorldInv{}; // 逆ワールド行列のキャッシュ
    mutable bool m_LocalDirty = true; // 何か変更があったか（CB更新や階層伝搬の最適化に使える）
    mutable bool m_WorldDirty = true; // 何か変更があったか（CB更新や階層伝搬の最適化に使える）

public:
    // ==================================================
    // ----- コンストラクタ -----
    // ==================================================
    TransformComponent() = default;
    ~TransformComponent() = default;

    // ==================================================
    // ----- セッター・ゲッター -----
    // ==================================================
    // --------------------------------------------------
	// アクセス（値そのものへの参照）
    // --------------------------------------------------
	Transform&       Value()        noexcept { return m_Value; }
	const Transform& Value() const  noexcept { return m_Value; }
    void             SetValue(const Transform& t) noexcept { m_Value = t; MarkLocalDirty();}
    // --------------------------------------------------
    // ワールド座標
    // --------------------------------------------------
    void SetPosition(const Vector3& pos)
    {
        if (m_pParent)
        {
            Matrix4x4 invParent = m_pParent->WorldMatrix().Inverse(); // 逆行列の取得
            Vector3 localPos = invParent.TransformPoint(pos);
            SetLocalPosition(localPos);
        }
        else
            SetLocalPosition(pos); // 親がいないのでローカル＝ワールド
    }
    Vector3 Position() const
    {
        const Matrix4x4 w = WorldMatrix();
        return Vector3(w.m[3][0], w.m[3][1], w.m[3][2]);
    }
    // --------------------------------------------------
    // ワールド回転（Quaternion）
    // --------------------------------------------------
    void SetRotation(const Quaternion& q)
    { 
        Quaternion parentQ = m_pParent ? m_pParent->Rotation() : Quaternion::Identity();
        m_Value.rotation = (q * parentQ.Inverse()).normalized(); // 行ベクトル×右手×能動回転
        MarkLocalDirty();
    }
    Quaternion Rotation() const   noexcept { return Transform::FromMatrix(WorldMatrix()).rotation; }
    // --------------------------------------------------
    // ワールド回転（Euler）
    // --------------------------------------------------
    void    SetEulerAngles(const Vector3& e) noexcept 
    { 
        Quaternion qW = Quaternion::FromEulerAngles(e);
        SetRotation(qW);
    }
    Vector3 EulerAngles() const noexcept { return Rotation().ToEulerAngles(); }
    // --------------------------------------------------
    // ワールドスケール
    // 親が回転+非一様スケールの場合は近似
    // 正確にやるなら極分解（polar decomposition）で線形部を R*S に分解してから処理
    // --------------------------------------------------
    void SetScale(const Vector3& worldS) noexcept 
    { 
        if (m_pParent)
        {
            Vector3 ps = m_pParent->LossyScale();
            auto safeDiv = [](float a, float b) {return (fabs(b) > 1e-8f) ? (a / b) : a; };
            Vector3 localScale(
                safeDiv(worldS.x, ps.x),
                safeDiv(worldS.y, ps.y),
                safeDiv(worldS.z, ps.z)
            );
            SetLocalScale(localScale);
        }
        else
            SetLocalScale(worldS);
    }
    Vector3  LossyScale() const noexcept 
    {
        const Matrix4x4& W = WorldMatrix();
        Vector3 r0(W.m[0][0], W.m[0][1], W.m[0][2]);
        Vector3 r1(W.m[1][0], W.m[1][1], W.m[1][2]);
        Vector3 r2(W.m[2][0], W.m[2][1], W.m[2][2]);
        float sx = r0.length(); if (sx < 1e-8f) sx = 0.0f;
        float sy = r1.length(); if (sy < 1e-8f) sy = 0.0f;
        float sz = r2.length(); if (sz < 1e-8f) sz = 0.0f;
        
        Vector3 u0 = (sx > 0) ? (r0 * (1.0f / sx)) : Vector3(1, 0, 0);
        Vector3 u1 = (sy > 0) ? (r1 * (1.0f / sy)) : Vector3(0, 1, 0);
        Vector3 u2 = (sz > 0) ? (r2 * (1.0f / sz)) : Vector3(0, 0, 1);
        
        float det = Vector3::Dot(Vector3::Cross(u0, u1), u2);
        if (det < 0.0f) sz = -sz;
        return {sx, sy, sz};
    }
    // --------------------------------------------------
    // ワールドベクトル
    // --------------------------------------------------
    Vector3  WorldRight()    const   noexcept { return WorldMatrix().TransformNormal({ 1, 0, 0 }).normalized(); }
    Vector3  WorldUp()       const   noexcept { return WorldMatrix().TransformNormal({ 0, 1, 0 }).normalized(); }
    Vector3  WorldForward()  const   noexcept { return WorldMatrix().TransformNormal({ 0, 0, 1 }).normalized(); }
    // --------------------------------------------------
    // ローカル回転（Quaternion）
    // --------------------------------------------------
    void       SetLocalRotation(const Quaternion& q)       noexcept { m_Value.rotation = q.normalized(); MarkLocalDirty(); }
    Quaternion LocalRotation()                   const   noexcept { return m_Value.rotation; }
    // --------------------------------------------------
    // ローカル回転（Euler）
    // --------------------------------------------------
    void     SetLocalEulerAngles(const Vector3& e)       noexcept { m_Value.rotation = Quaternion::FromEulerAngles(e); MarkLocalDirty(); }
    Vector3  LocalEulerAngles()                  const   noexcept { return m_Value.rotation.ToEulerAngles(); }
    // --------------------------------------------------
    // ローカルスケール
    // --------------------------------------------------
    void     SetLocalScale(const Vector3& s)             noexcept { m_Value.scale = s; MarkLocalDirty(); }
    Vector3  LocalScale()                        const   noexcept { return m_Value.scale; }
    // --------------------------------------------------
    // ローカルベクトル
    // --------------------------------------------------
    Vector3  Right()    const   noexcept { return m_Value.GetRight(); }
    Vector3  Up()       const   noexcept { return m_Value.GetUp(); }
    Vector3  Forward()  const   noexcept { return m_Value.GetForward(); }
    // --------------------------------------------------
    // ローカル座標
    // --------------------------------------------------
    void SetLocalPosition(const Vector3& p)noexcept { m_Value.position = p; MarkLocalDirty(); }
    Vector3 LocalPosition() const noexcept { return m_Value.position; }

    // --------------------------------------------------
    // 行列取得（遅延再計算）
    // --------------------------------------------------
    const Matrix4x4& LocalMatrix()
    {
        if (m_LocalDirty)
        {
            m_Local = m_Value.GetLocalMatrix();

            m_LocalDirty = false;
            m_WorldDirty = true; // ローカルが変わったのでワールドも更新
        }
        return m_Local;
    }
    const Matrix4x4& WorldMatrix()
    {
        // ローカル更新
        LocalMatrix(); // ここで WorldDirty フラグが立つことがある

        if (m_WorldDirty)
        {
            if (m_pParent)
                m_World = m_Local * m_pParent->WorldMatrix(); // ローカル×親
            else
                m_World = m_Local; // 親がいないので World = Local
            m_WorldDirty = false;

            // m_WorldInvTranspose の再計算
            CulcWorldNormalMatrix();
        }
        return m_World;
    }
    const Matrix4x4& WorldMatrix() const // ※※※　コイツのせいで mutable を使っています　※※※
    {
        const_cast<TransformComponent*>(this)->WorldMatrix(); // ここで再計算している（？）
        return m_World;
    }
    const Matrix4x4 WorldMatrixInv() const
    {
        const_cast<TransformComponent*>(this)->WorldMatrix(); // ここで再計算している（？）
        return m_WorldInv;
    }
    
    // --------------------------------------------------
    // Transform の強制書き換え
    // --------------------------------------------------
    void SetWorldMatrixFromExternal(const Matrix4x4& desireWorld)
    {
        Matrix4x4 M = desireWorld;
        // 親マトリクスの確認
        if (m_pParent)
        {
            const Matrix4x4 invParent = m_pParent->WorldMatrix().Inverse();
            M = desireWorld * invParent; // 親のローカルへ落とす
        }

        // ----- Translation 抽出 -----
        const Vector3 t(M.m[3][0], M.m[3][1], M.m[3][2]);

        // ----- Rotation 抽出 -----
        Vector3 r0(M.m[0][0], M.m[0][1], M.m[0][2]);
        Vector3 r1(M.m[1][0], M.m[1][1], M.m[1][2]);
        Vector3 r2(M.m[2][0], M.m[2][1], M.m[2][2]);

        const float eps = 1e-8f;

        float sx = r0.length();
        float sy = r1.length();
        float sz = r2.length();

        // 軸がつぶれている場合は回転を更新しない
        if (sx < eps || sy < eps || sz < eps)
        {
            m_Value.position = t;
            MarkLocalDirty();
            return;
        }

        // スケール除去（単位化）
        Vector3 u0 = r0 * (1.0f / sx);
        Vector3 u1 = r1 * (1.0f / sy);
        Vector3 u2 = r2 * (1.0f / sz);

        // ----- 簡易直交化で安定化 -----
        u0 = u0.normalized();
        // u1 から　u0 成分を取り除く
        u1 = u1 - u0 * Vector3::Dot(u0, u1);
        float u1len = u1.length();
        if (u1len < eps) // 直交失敗⇒回転維持
        {
            m_Value.position = t;
            MarkLocalDirty();
            return;
        }
        u1 = u1 * (1.0f / u1len);

        // 外積で u2 を作り直す
        u2 = Vector3::Cross(u0, u1);
        float u2len = u2.length();
        if (u2len < eps) // 直交失敗⇒回転維持
        {
            m_Value.position = t;
            MarkLocalDirty();
            return;
        }
        u2 = u2 * (1.0f / u2len);

        // det < 0 対策
        Vector3 u2raw = r2 * (1.0f / sz);
        if (Vector3::Dot(u2, u2raw) < 0.0f)
        {
            u2 *= -1.0f;
        }

        // 回転行列を作る
        Matrix4x4 R;
        R.identity();
        R.m[0][0] = u0.x; R.m[0][1] = u0.y; R.m[0][2] = u0.z;
        R.m[1][0] = u1.x; R.m[1][1] = u1.y; R.m[1][2] = u1.z;
        R.m[2][0] = u2.x; R.m[2][1] = u2.y; R.m[2][2] = u2.z;
        Quaternion q = Quaternion::FromMatrix(R).normalized();

        // ----- Transform へ反映 -----
        m_Value.position = t;
        m_Value.rotation = q;
        // m_Value.scale は維持

        MarkLocalDirty();
    }

    // ==================================================
    // ----- Dirty 関係 -----
    // ==================================================
    // --------------------------------------------------
    // Dirty チェック
    // --------------------------------------------------
    bool IsWorldDirty() const noexcept { return m_WorldDirty; }
    // --------------------------------------------------
    // ローカル行列の変更
    // --------------------------------------------------
    void MarkLocalDirty()
    {
        m_LocalDirty = true;
        MarkWorldDirtyRecursive(); // ローカルが変わるのでワールドも変える
    }
    // --------------------------------------------------
    // ワールド行列変更命令
    // --------------------------------------------------
    void MarkWorldDirtyRecursive()
    {
        m_WorldDirty = true;
        for (auto* ch : m_Children) if (ch) ch->MarkWorldDirtyRecursive(); // 子供全員の行列を再計算
    }

    // ==================================================
    // ----- 親子関係管理 -----
    // ==================================================
    // --------------------------------------------------
    // node の親の中に maybeAncestor がいないかチェック
    // --------------------------------------------------
    static bool IsDescendantOf(TransformComponent* node, TransformComponent* maybeAncestor) 
    {
        for (auto* p = node->m_pParent; p; p = p->m_pParent)
            if (p == maybeAncestor) return true;
        return false;
    }
    // --------------------------------------------------
    // 親設定（ワールド非維持）
    // --------------------------------------------------
    void SetParent(TransformComponent* pParent)
    {
        if (m_pParent == pParent) return; // 既に親に設定している
        if (pParent && IsDescendantOf(pParent, this)) return; // 自分の子を親に設定しないためにチェック

        // 旧親から自分を外す
        if (m_pParent)
        {
            auto& v = m_pParent->m_Children;
            v.erase(std::remove(v.begin(), v.end(), this), v.end());
        }

        m_pParent = pParent;

        if (m_pParent) m_pParent->m_Children.push_back(this);

        // 親が変わったのでワールド基準が変わるので再計算
        MarkWorldDirtyRecursive();
    }
    // --------------------------------------------------
    // 親設定（ワールド維持）
    // --------------------------------------------------
    void SetParentKeepWorld(TransformComponent* newParent)
    {
        if (m_pParent == newParent) return; // 既に親に設定している
        if (newParent && IsDescendantOf(newParent, this)) return; // 自分の子を親に設定しないためにチェック

        // 現在のワールド座標を保持
        Matrix4x4 currentWorld = WorldMatrix();

        // まずは通常の SetParent と同じ処理
        if (m_pParent)
        {
            auto& v = m_pParent->m_Children;
            v.erase(std::remove(v.begin(), v.end(), this), v.end());
        }
        m_pParent = newParent;
        if (m_pParent) m_pParent->m_Children.push_back(this);
         
        // 新しいローカル座標を計算して適用
        if (m_pParent)
        {
            Matrix4x4 invParent = m_pParent->WorldMatrix().Inverse();
            Matrix4x4 newLocal = currentWorld * invParent; // Local = World * Parent^-1
            m_Value = Transform::FromMatrix(newLocal);
        }
        else
            m_Value = Transform::FromMatrix(currentWorld);

        MarkLocalDirty(); // ワールドも更新される
    }
    // --------------------------------------------------
    // セッターゲッター
    // --------------------------------------------------
    TransformComponent* Parent()                noexcept { return m_pParent; }
    const TransformComponent* Parent() const    noexcept { return m_pParent; }
    const std::vector<TransformComponent*>& Children() const noexcept { return m_Children; }

    // ==================================================
    // 回転処理
    // ==================================================
    // --------------------------------------------------
    // 回転させる関数（ローカル軸回転）
    // --------------------------------------------------
    void Rotate(const Vector3& deltaEuler)
    {
        Quaternion delta = Quaternion::FromEulerAngles(deltaEuler);
        Quaternion qL = LocalRotation();
        SetLocalRotation((qL * delta).normalized());
    }
    // --------------------------------------------------
    // 任意の軸で回転
    // --------------------------------------------------
    void RotateAxis(const Vector3& worldAxis, float angle)
    {
        Quaternion delta = Quaternion::FromAxisAngle(worldAxis, angle);
        Quaternion qW = Rotation();
        SetRotation((delta * qW).normalized());
    }
    // --------------------------------------------------
    // ターゲットの方向を向ける
    // --------------------------------------------------
    void LookAt(const Vector3& target, const Vector3& up = Vector3(0, 1, 0))
    {
        Quaternion qW = Quaternion::LookAt(Position(), target, up);
        SetRotation(qW.normalized());
    }
    
    // ==================================================
    // ----- ライフサイクル -----
    // ==================================================
    void OnAdded()  override 
    {
        m_LocalDirty = true;
        m_WorldDirty = true;
    }
    void Init()     override {}
    void Uninit()   override 
    {
        // 子の親参照を外す（お掃除）-----
        for (auto* ch : m_Children) if (ch) ch->m_pParent = nullptr;
        m_Children.clear();
        SetParent(nullptr);
    }
    // Transform は通常 Update/Draw で処理不要。階層化するならここでワールド更新など
    void FixedUpdate(float dt)  override {}
    void Update(float dt)       override {}
    void Draw()                 override {}

private:
    // --------------------------------------------------
    // ３×３の線形部のみの逆行列
    // --------------------------------------------------
    void CulcWorldNormalMatrix()
    {        // A = world の 3x3 線形部（行ベクトル規約）
        float a00 = m_World.m[0][0], a01 = m_World.m[0][1], a02 = m_World.m[0][2];
        float a10 = m_World.m[1][0], a11 = m_World.m[1][1], a12 = m_World.m[1][2];
        float a20 = m_World.m[2][0], a21 = m_World.m[2][1], a22 = m_World.m[2][2];

        float det = a00 * (a11 * a22 - a12 * a21) - a01 * (a10 * a22 - a12 * a20) + a02 * (a10 * a21 - a11 * a20);

        Matrix4x4 N; N.identity();
        if (std::fabs(det) > 1e-8f) {
            float invDet = 1.0f / det;

            // まず余因子（cofactor）を計算
            float c00 = (a11 * a22 - a12 * a21);
            float c01 = (a02 * a21 - a01 * a22);
            float c02 = (a01 * a12 - a02 * a11);

            float c10 = (a12 * a20 - a10 * a22);
            float c11 = (a00 * a22 - a02 * a20);
            float c12 = (a02 * a10 - a00 * a12);

            float c20 = (a10 * a21 - a11 * a20);
            float c21 = (a01 * a20 - a00 * a21);
            float c22 = (a00 * a11 - a01 * a10);

            // 逆行列 A^{-1} = (1/det) * Cofactor^T
            N.m[0][0] = c00 * invDet; N.m[0][1] = c10 * invDet; N.m[0][2] = c20 * invDet;
            N.m[1][0] = c01 * invDet; N.m[1][1] = c11 * invDet; N.m[1][2] = c21 * invDet;
            N.m[2][0] = c02 * invDet; N.m[2][1] = c12 * invDet; N.m[2][2] = c22 * invDet;
        }
        m_WorldInv = N;   // ← 変数名も“Inv”に。CB へもこの3×3を入れる
    }

};
static_assert(!std::is_abstract<TransformComponent>::value, "TransformComponent is abstract");
static_assert(std::is_default_constructible<TransformComponent>::value, "TransformComponent is NOT default-constructible");

#endif