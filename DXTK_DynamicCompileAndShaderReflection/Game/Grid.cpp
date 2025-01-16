/**********************************************
 *
 *  Grid.cpp
 *  �O���b�h
 *
 *  ����ҁF���ە��m
 *
 **********************************************/
#include "pch.h"
#include "Grid.h"
#include "DebugDraw.h"
#include "DirectXHelpers.h"

using namespace DirectX;


/// <summary>
/// �R���X�g���N�^
/// </summary>
AirL::Grid::Grid(ID3D11Device* device, ID3D11DeviceContext* context, const int& cellX, const int& cellY, const SimpleMath::Vector2& cellSize):
	m_cellSize(cellSize),
	m_cellX(cellX),
	m_cellY(cellY)
{
	//	�x�[�V�b�N�G�t�F�N�g�̍쐬
	m_basicEffect = std::make_unique<BasicEffect>(device);
	m_basicEffect->SetVertexColorEnabled(true);
	// �C���v�b�g���C�A�E�g�̍쐬
	DX::ThrowIfFailed(
		CreateInputLayoutFromEffect<VertexType>(device, m_basicEffect.get(), m_inputLayout.ReleaseAndGetAddressOf())
	);

	// �v���~�e�B�u�o�b�`�̍쐬
	m_primitiveBatch = std::make_unique<PrimitiveBatch<VertexType>>(context);
}





/// <summary>
/// �`�揈��
/// </summary>
/// <param name="view">�r���[�s��</param>
/// <param name="proj">�ˉe�s��</param>
void AirL::Grid::Render(
	ID3D11DeviceContext* context,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj
)
{
	// �X�P�[�����O�p�s��
	SimpleMath::Matrix world =
		SimpleMath::Matrix::CreateScale({ 0.5f * m_cellSize.x, 1.0f, 0.5f * m_cellSize.y }) *
		SimpleMath::Matrix::CreateScale({(float)m_cellX, 0.0f, (float)m_cellY})
		;

	// �C���v�b�g���C�A�E�g�̐ݒ�
	context->IASetInputLayout(m_inputLayout.Get());
	
	// �G�t�F�N�g�̐ݒ�
	m_basicEffect->SetMatrices(world, view, proj);
	m_basicEffect->Apply(context);
	
	// �O���b�h�̕`��
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
