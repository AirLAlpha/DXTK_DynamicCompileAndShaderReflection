/**********************************************
 *
 *  Scene.h
 *  シーン
 *
 *  製作者：牛丸文仁
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


// 前方宣言
namespace DX
{
	class DeviceResources;
}
struct ID3D11ShaderReflection;


class Scene
{
public:
	// コンストラクタ
	Scene(DX::DeviceResources* pDeviceResources);
	// デストラクタ
	~Scene() = default;

	// 初期化
	void Initialize();
	// 更新
	void Update(const float& deltaTime);
	// 描画
	void Render();
	// 終了処理
	void Finalize();


private:
	// シェーダーリフレクションから定数バッファの作成
	void CreateConstantBufferFromReflect(ID3D11ShaderReflection* shaderRef);


private:
	// 定数バッファのパラメーター
	struct ConstantBufferVariable
	{
		std::string variableName;		// 変数名
		float* pArrayStart = nullptr;	// 値の開始ポインタ
		size_t byteSize = 0;			// 変数のサイズ
	};

	// 定数バッファの情報
	struct ConstantBuffer
	{
		// 定数バッファの名前
		std::string bufferName;
		// 変数の情報（変数名：定数バッファの情報）
		std::unordered_map<std::string, ConstantBufferVariable> variableData;
		// 値の配列
		float* pValues = nullptr;
		// バッファ
		Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer;


		// デストラクタ
		~ConstantBuffer()
		{
			// 値があれば削除
			if (pValues != nullptr)
			{
				delete[] pValues;
				pValues = nullptr;
			}
		}
	};


private:
	// デバイスリソース
	DX::DeviceResources* m_pDeviceResources;
	// マウス
	std::unique_ptr<DirectX::Mouse>					m_mouse;
	std::unique_ptr<DirectX::Mouse::ButtonStateTracker> m_mouseButtonStateTracker;

	// カメラ
	std::unique_ptr<AirL::FreeLookCamera>			m_camera;

	// CommonStates
	std::unique_ptr<DirectX::CommonStates>          m_states;
	// グリッド
	std::unique_ptr<AirL::Grid>                     m_grid;
	// モデルオブジェクト
	std::unique_ptr<DirectX::Model>                 m_model;
	// ジオメトリックプリミティブ
	std::unique_ptr<DirectX::GeometricPrimitive>	m_geometricPrimitive;


	// インプットレイアウト
	Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_inputLayout;
	// 頂点シェーダー
	Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_vertexShader;
	// ピクセルシェーダー
	Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_pixelShader;

	// 定数バッファ（定数バッファ名：定数バッファの情報）
	std::vector<std::unique_ptr<ConstantBuffer>>	m_constantBuffers;

	// 環境テクスチャ（キューブマップ）
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cubemap;
	// テクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_albedo;
	// ノーマルマップ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_normalMap;


	// スカイボックスモデル
	std::unique_ptr<DirectX::GeometricPrimitive> m_skyboxModel;
	// インプットレイアウト
	Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_skyInputLayout;
	// 頂点シェーダー
	Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_skyVertexShader;
	// ピクセルシェーダー
	Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_skyPixelShader;
};