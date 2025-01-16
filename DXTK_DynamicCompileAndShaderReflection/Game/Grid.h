/**********************************************
 *
 *  Grid.h
 *  �O���b�h
 *
 *  ����ҁF���ە��m
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
		// �R���X�g���N�^
		Grid(ID3D11Device* device, ID3D11DeviceContext* context, const int& cellX = 5, const int& cellY = 5, const DirectX::SimpleMath::Vector2& cellSize = DirectX::SimpleMath::Vector2::One);
		// �f�X�g���N�^
		~Grid() = default;

		// �`�揈��
		void Render(
			ID3D11DeviceContext* context,
			const DirectX::SimpleMath::Matrix& view,
			const DirectX::SimpleMath::Matrix& proj
		);



	private:
		//	���_�^�C�v
		using VertexType = DirectX::VertexPositionColor;

	private:
		// �Z���̕�
		DirectX::SimpleMath::Vector2 m_cellSize;
		// �Z���̌�
		int m_cellX;
		int m_cellY;

		// �v���~�e�B�u�o�b�`
		std::unique_ptr<DirectX::PrimitiveBatch<VertexType>> m_primitiveBatch;
		// �x�[�V�b�N�G�t�F�N�g
		std::unique_ptr<DirectX::BasicEffect> m_basicEffect;
		// �C���v�b�g���C�A�E�g
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	};
}