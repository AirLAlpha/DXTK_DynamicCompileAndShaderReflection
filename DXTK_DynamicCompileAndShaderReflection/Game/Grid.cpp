/**********************************************
 *
 *  Grid.cpp
 *  グリッド
 *
 *  製作者：牛丸文仁
 *
 **********************************************/
#include "pch.h"
#include "Grid.h"
#include "DebugDraw.h"
#include "DirectXHelpers.h"

using namespace DirectX;


/// <summary>
/// コンストラクタ
/// </summary>
AirL::Grid::Grid(ID3D11Device* device, ID3D11DeviceContext* context, const int& cellX, const int& cellY, const SimpleMath::Vector2& cellSize):
	m_cellSize(cellSize),
	m_cellX(cellX),
	m_cellY(cellY)
{
	//	ベーシックエフェクトの作成
	m_basicEffect = std::make_unique<BasicEffect>(device);
	m_basicEffect->SetVertexColorEnabled(true);
	// インプットレイアウトの作成
	DX::ThrowIfFailed(
		CreateInputLayoutFromEffect<VertexType>(device, m_basicEffect.get(), m_inputLayout.ReleaseAndGetAddressOf())
	);

	// プリミティブバッチの作成
	m_primitiveBatch = std::make_unique<PrimitiveBatch<VertexType>>(context);
}





/// <summary>
/// 描画処理
/// </summary>
/// <param name="view">ビュー行列</param>
/// <param name="proj">射影行列</param>
void AirL::Grid::Render(
	ID3D11DeviceContext* context,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj
)
{
	// スケーリング用行列
	SimpleMath::Matrix world =
		SimpleMath::Matrix::CreateScale({ 0.5f * m_cellSize.x, 1.0f, 0.5f * m_cellSize.y }) *
		SimpleMath::Matrix::CreateScale({(float)m_cellX, 0.0f, (float)m_cellY})
		;

	// インプットレイアウトの設定
	context->IASetInputLayout(m_inputLayout.Get());
	
	// エフェクトの設定
	m_basicEffect->SetMatrices(world, view, proj);
	m_basicEffect->Apply(context);
	
	// グリッドの描画
	m_primitiveBatch->Begin();
	DX::DrawGrid(
		m_primitiveBatch.get(),
		SimpleMath::Vector3::Right,
		SimpleMath::Vector3::Forward,
		SimpleMath::Vector3::Zero,
		m_cellX,
		m_cellY
	);
	m_primitiveBatch->End();
}
