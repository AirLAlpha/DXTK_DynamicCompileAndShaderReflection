/**********************************************
 *
 *  Scene.h
 *  �V�[��
 *
 *  ����ҁF���ە��m
 *
 **********************************************/
#pragma once
#include <string>
#include <unordered_map>
#include "SimpleMath.h"
#include "Grid.h"
#include "Model.h"
#include "CommonStates.h"
#include "GeometricPrimitive.h"
#include "FreeLookCamera.h"


// �O���錾
namespace DX
{
	class DeviceResources;
}
struct ID3D11ShaderReflection;


class Scene
{
public:
	// �R���X�g���N�^
	Scene(DX::DeviceResources* pDeviceResources);
	// �f�X�g���N�^
	~Scene() = default;

	// ������
	void Initialize();
	// �X�V
	void Update(const float& deltaTime);
	// �`��
	void Render();
	// �I������
	void Finalize();


private:
	// �V�F�[�_�[���t���N�V��������萔�o�b�t�@�̍쐬
	void CreateConstantBufferFromReflect(ID3D11ShaderReflection* shaderRef);


private:
	// �萔�o�b�t�@�̃p�����[�^�[
	struct ConstantBufferVariable
	{
		std::string variableName;		// �ϐ���
		float* pArrayStart = nullptr;	// �l�̊J�n�|�C���^
		size_t byteSize = 0;			// �ϐ��̃T�C�Y
	};

	// �萔�o�b�t�@�̏��
	struct ConstantBuffer
	{
		// �萔�o�b�t�@�̖��O
		std::string bufferName;
		// �ϐ��̏��i�ϐ����F�萔�o�b�t�@�̏��j
		std::unordered_map<std::string, ConstantBufferVariable> variableData;
		// �l�̔z��
		float* pValues = nullptr;
		// �o�b�t�@
		Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer;


		// �f�X�g���N�^
		~ConstantBuffer()
		{
			// �l������΍폜
			if (pValues != nullptr)
			{
				delete[] pValues;
				pValues = nullptr;
			}
		}
	};


private:
	// �f�o�C�X���\�[�X
	DX::DeviceResources* m_pDeviceResources;
	// �}�E�X
	std::unique_ptr<DirectX::Mouse>					m_mouse;
	std::unique_ptr<DirectX::Mouse::ButtonStateTracker> m_mouseButtonStateTracker;

	// �J����
	std::unique_ptr<AirL::FreeLookCamera>			m_camera;

	// CommonStates
	std::unique_ptr<DirectX::CommonStates>          m_states;
	// �O���b�h
	std::unique_ptr<AirL::Grid>                     m_grid;
	// ���f���I�u�W�F�N�g
	std::unique_ptr<DirectX::Model>                 m_model;
	// �W�I���g���b�N�v���~�e�B�u
	std::unique_ptr<DirectX::GeometricPrimitive>	m_geometricPrimitive;


	// �C���v�b�g���C�A�E�g
	Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_inputLayout;
	// ���_�V�F�[�_�[
	Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_vertexShader;
	// �s�N�Z���V�F�[�_�[
	Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_pixelShader;

	// �萔�o�b�t�@�i�萔�o�b�t�@���F�萔�o�b�t�@�̏��j
	std::vector<std::unique_ptr<ConstantBuffer>>	m_constantBuffers;

	// ���e�N�X�`���i�L���[�u�}�b�v�j
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cubemap;
	// �e�N�X�`��
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_albedo;
	// �m�[�}���}�b�v
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_normalMap;


	// �X�J�C�{�b�N�X���f��
	std::unique_ptr<DirectX::GeometricPrimitive> m_skyboxModel;
	// �C���v�b�g���C�A�E�g
	Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_skyInputLayout;
	// ���_�V�F�[�_�[
	Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_skyVertexShader;
	// �s�N�Z���V�F�[�_�[
	Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_skyPixelShader;
};