/**********************************************
 *
 *  Common.hlsli
 *  原則すべてのシェーダーがincludeする
 *
 *  製作者：牛丸 文仁
 *
 **********************************************/

// 共通定数バッファ
cbuffer CommonConstBuffer : register(b0)
{
    matrix m_matWorld;          // ワールド行列
    matrix m_matView;           // ビュー行列
    matrix m_matProj;           // 射影行列

    float3 f_cameraPositionWS;  // カメラのワールド座標
}

// 共通ライト定数バッファ
cbuffer LightConstBuffer : register(b1)
{
    float3 f_mainLightDirection;    // メインライト（ディレクショナルライト）の向き
    float  f_mainLightBrightness;   // メインライトの強さ
    float4 c_mainLightColor;        // メインライトの色
    float4 c_ambientLightColor;     // 環境光の色
    float f_ambientLightIntensity;  // 環境光の強さ
}

// スカイボックスのCubeMap
TextureCube<float4> g_skybox : register(t0);
// サンプラー
SamplerState g_sampler : register(s0);

// PI
static const float F_PI = 3.1415926f;
