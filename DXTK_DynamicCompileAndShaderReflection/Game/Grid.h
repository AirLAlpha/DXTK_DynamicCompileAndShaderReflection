/**********************************************
 *
 *  Grid.h
 *  グリッド
 *
 *  製作者：牛丸文仁
 *
 **********************************************/
#pragma once
#include "PrimitiveBatch.h"
#include "Effects.h"
#include "VertexTypes.h"
#include "SimpleMath.h"

namespace AirL
{
	class Grid
	{
	public:
		// コンストラクタ
		Grid(ID3D11Device* device, ID3D11DeviceContext* context, const int& cellX = 5, const int& cellY = 5, const DirectX::SimpleMath::Vector2& cellSize = DirectX::SimpleMath::Vector2::One);
		// デストラクタ
		~Grid() = default;

		// 描画処理
		void Render(
			ID3D11DeviceContext* context,
			const DirectX::SimpleMath::Matrix& view,
			const DirectX::SimpleMath::Matrix& proj
		);



	private:
		//	頂点タイプ
		using VertexType = DirectX::VertexPositionColor;

	private:
		// セルの幅
		DirectX::SimpleMath::Vector2 m_cellSize;
		// セルの個数
		int m_cellX;
		int m_cellY;

		// プリミティブバッチ
		std::unique_ptr<DirectX::PrimitiveBatch<VertexType>> m_primitiveBatch;
		// ベーシックエフェクト
		std::unique_ptr<DirectX::BasicEffect> m_basicEffect;
		// インプットレイアウト
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	};
}