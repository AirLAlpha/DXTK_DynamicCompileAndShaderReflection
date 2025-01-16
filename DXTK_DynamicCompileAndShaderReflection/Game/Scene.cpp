/**********************************************
 *
 *  Scene.cpp
 *  シーン
 *
 *  製作者：牛丸文仁
 *
 **********************************************/
#include "pch.h"
#include "d3dcompiler.h"
#include "Scene.h"
#include "DeviceResources.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"
#include "ReadData.h"
#include <array>

using namespace DirectX;

// モデルタイプ：(0:ジオメトリックプリミティブ、1:CMO、2:SDKMESH）
#define MODELTYPE (0)
// インプットレイアウトのマクロをCMOにする
#define ILTYPE_CMO (false)

// テクスチャを使う
#define USE_ALBEDO_TEXTURE (true)
// 法線マップを使う
#define USE_NORMAL_TEXTURE (true)

// メタリック
#define METALLIC    (1.0f)
// 滑らかさ
#define SMOOTHNESS  (0.5f) 




/// <summary>
/// コンストラクタ
/// </summary>
/// <param name="pDeviceResources">デバイスリソースのポインタ</param>
Scene::Scene(DX::DeviceResources* pDeviceResources):
	m_pDeviceResources(pDeviceResources)
{
}



/// <summary>
/// 初期化処理
/// </summary>
void Scene::Initialize()
{
    // デバイス
    ID3D11Device* device = m_pDeviceResources->GetD3DDevice();
    ID3D11DeviceContext* context = m_pDeviceResources->GetD3DDeviceContext();

    // マウスの作成
    m_mouse = std::make_unique<Mouse>();
    m_mouseButtonStateTracker = std::make_unique<Mouse::ButtonStateTracker>();

    // カメラの作成
    RECT windowRect = m_pDeviceResources->GetOutputSize();
    m_camera = std::make_unique<AirL::FreeLookCamera>((int)windowRect.right, (int)windowRect.bottom);

    // コモンステートの作成
    m_states = std::make_unique<CommonStates>(device);
    // グリッドの作成
    m_grid = std::make_unique<AirL::Grid>(device, context);

    // モデルの読み込み
    std::unique_ptr<EffectFactory> fx = std::make_unique<EffectFactory>(device);

#if MODELTYPE == 0
    // ジオメトリックプリミティブの作成
    m_geometricPrimitive = GeometricPrimitive::CreateTeapot(context);
#elif MODELTYPE == 1
    // CMOモデル
    m_model = Model::CreateFromCMO(device, L"Resources/CMO/torus.cmo", *fx);
#elif MODELTYPE == 2
    // SDKMESHモデル
    m_model = Model::CreateFromSDKMESH(device, L"Resources/SDKMESH/bunny.sdkmesh", *fx);
#endif


    // スカイボックス用モデルの作成
    m_skyboxModel = GeometricPrimitive::CreateGeoSphere(context, 2.0f, 3, false);


    Microsoft::WRL::ComPtr<ID3DBlob> blob, errorMsg;

    //////////////////////////
    // 頂点シェーダーの作成 //
    //////////////////////////

    // マクロ定義
    const D3D_SHADER_MACRO macros[] = {
#if ILTYPE_CMO
        {"MODELTYPE_CMO", ""},
#endif
        {NULL, NULL},
    };

    // 頂点シェーダーのコンパイル
    D3DCompileFromFile(
        L"Resources/HLSL/PBRLit_VS.hlsl",
        macros,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        blob.ReleaseAndGetAddressOf(),
        errorMsg.ReleaseAndGetAddressOf()
    );

    if (errorMsg)
        OutputDebugStringA((char*)errorMsg->GetBufferPointer());

    // 頂点シェーダーオブジェクトの作成
    DX::ThrowIfFailed(
        device->CreateVertexShader(
            blob->GetBufferPointer(),
            blob->GetBufferSize(),
            nullptr,
            m_vertexShader.ReleaseAndGetAddressOf()
        )
    );

    // 頂点シェーダーのシェーダーリフレクション
    Microsoft::WRL::ComPtr<ID3D11ShaderReflection> vsRef;
    D3DReflect(
        blob->GetBufferPointer(),
        blob->GetBufferSize(),
        IID_ID3D11ShaderReflection,
        (void**)vsRef.ReleaseAndGetAddressOf()
    );

    // リフレクションの情報を取得
    D3D11_SHADER_DESC vsShaderDesc{};
    vsRef->GetDesc(&vsShaderDesc);

    // インプットレイアウトの情報を取得
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements{ vsShaderDesc.InputParameters };
    for (int i = 0; i < (int)vsShaderDesc.InputParameters; i++)
    {
        // 要素ひとつ分の情報を取得
        D3D11_SIGNATURE_PARAMETER_DESC sigDesc;
        vsRef->GetInputParameterDesc(i, &sigDesc);
        // フォーマットを取得（Maskは float3:0b0111, float4:0b1111 のように値の数によって数値が変化する）
        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
        if ((sigDesc.Mask & 0x0F) == 0x0F) format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        else if ((sigDesc.Mask & 0x07) == 0x07) format = DXGI_FORMAT_R32G32B32_FLOAT;
        else if ((sigDesc.Mask & 0x07) == 0x03) format = DXGI_FORMAT_R32G32_FLOAT;
        else if ((sigDesc.Mask & 0x07) == 0x01) format = DXGI_FORMAT_R32_FLOAT;

        // ひとつ分の要素を作成
        D3D11_INPUT_ELEMENT_DESC elemDesc =
        {
            sigDesc.SemanticName,
            sigDesc.SemanticIndex,
            format,
            0,									// 決め打ち
            D3D11_APPEND_ALIGNED_ELEMENT,		// 決め打ち
            D3D11_INPUT_PER_VERTEX_DATA,		// 決め打ち
            0									// 決め打ち
        };

        // 配列に追加
        inputElements[i] = elemDesc;
    }

    // インプットレイアウトの作成
    DX::ThrowIfFailed(
        device->CreateInputLayout(
            &inputElements[0],
            (UINT)inputElements.size(),
            blob->GetBufferPointer(),
            blob->GetBufferSize(),
            m_inputLayout.ReleaseAndGetAddressOf()
        )
    );

    // 頂点シェーダーが利用する定数バッファの作成
    CreateConstantBufferFromReflect(vsRef.Get());



    //////////////////////////////
    // ピクセルシェーダーの作成 //
    //////////////////////////////

    // ピクセルシェーダーのコンパイル
    DX::ThrowIfFailed(
        D3DCompileFromFile(
            L"Resources/HLSL/PBRLit_PS.hlsl",
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "main",
            "ps_5_0",
            D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
            0,
            blob.ReleaseAndGetAddressOf(),
            errorMsg.ReleaseAndGetAddressOf()
        )
    );

    // ピクセルシェーダーオブジェクトの作成
    DX::ThrowIfFailed(
        device->CreatePixelShader(
            blob->GetBufferPointer(),
            blob->GetBufferSize(),
            nullptr,
            m_pixelShader.ReleaseAndGetAddressOf()
        )
    );

    // シェーダーリフレクション
    Microsoft::WRL::ComPtr<ID3D11ShaderReflection> psRef;
    DX::ThrowIfFailed(
        D3DReflect(
            blob->GetBufferPointer(),
            blob->GetBufferSize(),
            IID_ID3D11ShaderReflection,
            (void**)psRef.ReleaseAndGetAddressOf()
        )
    );

    // 定数バッファの作成
    CreateConstantBufferFromReflect(psRef.Get());



    //////////////////////////
    // テクスチャの読み込み //
    //////////////////////////

    // キューブマップの読み込み
    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(
            device,
            L"Resources/DDS/cubemap.dds",
            nullptr,
            m_cubemap.ReleaseAndGetAddressOf()
        )
    );

    // アルベドテクスチャの読み込み
    DX::ThrowIfFailed(
        CreateWICTextureFromFile(
            device,
            L"Resources/Texture/wood_floor_worn_diff_2k.png",
            nullptr,
            m_albedo.ReleaseAndGetAddressOf()
        )
    );

    // ノーマルマップの読み込み
    DX::ThrowIfFailed(
        CreateWICTextureFromFile(
            device,
            L"Resources/Texture/wood_floor_worn_nor_dx_2k.png",
            nullptr,
            m_normalMap.ReleaseAndGetAddressOf()
        )
    );
    
    //////////////////////////////////////////////
    // スカイボックスのシェーダー（.cso）を作成 //
    //////////////////////////////////////////////
    
    auto skyVSData = DX::ReadData(L"Resources/CSO/Skybox_VS.cso");
    auto skyPSData = DX::ReadData(L"Resources/CSO/Skybox_PS.cso");

    // インプットレイアウト
    DX::ThrowIfFailed(
        device->CreateInputLayout(
            VertexPosition::InputElements,
            VertexPosition::InputElementCount,
            skyVSData.data(),
            skyVSData.size(),
            m_skyInputLayout.ReleaseAndGetAddressOf()
        )
    );

    // 頂点シェーダー
    DX::ThrowIfFailed(
        device->CreateVertexShader(
            skyVSData.data(),
            skyVSData.size(),
            nullptr,
            m_skyVertexShader.ReleaseAndGetAddressOf()
        )
    );
    // ピクセルシェーダー
    DX::ThrowIfFailed(
        device->CreatePixelShader(
            skyPSData.data(),
            skyPSData.size(),
            nullptr,
            m_skyPixelShader.ReleaseAndGetAddressOf()
        )
    );
}



/// <summary>
/// 更新処理
/// </summary>
/// <param name="deltaTime"></param>
void Scene::Update(const float& deltaTime)
{
    // マウスステートトラッカーの更新
    m_mouseButtonStateTracker->Update(m_mouse->GetState());

    // カメラに画面サイズを設定
    RECT windowRect = m_pDeviceResources->GetOutputSize();
    m_camera->SetScreenResolution((int)windowRect.right, (int)windowRect.bottom);
    // カメラの更新
    m_camera->Update(m_mouseButtonStateTracker->GetLastState(), deltaTime);

    // マウスのスクロール量をリセット
    m_mouse->ResetScrollWheelValue();
}



/// <summary>
/// 描画処理
/// </summary>
void Scene::Render()
{
    // コンテキスト
    ID3D11DeviceContext* context = m_pDeviceResources->GetD3DDeviceContext();

    // ワールド行列の作成
    SimpleMath::Matrix world =
        SimpleMath::Matrix::CreateRotationY(XM_PI) *
        SimpleMath::Matrix::Identity;

    // ビュー行列の取得
    SimpleMath::Matrix view = m_camera->GetView();
    // 射影行列の取得
    SimpleMath::Matrix proj = m_camera->GetProjection();


    ////////////////////////
    // 定数バッファの更新 //
    ////////////////////////

    // 共通定数バッファ(CommonConstantBuffer)の更新
    {
        ConstantBuffer* commonCB = m_constantBuffers[0].get();

        // 各行列
        XMMATRIX tWorld = world.Transpose();
        XMMATRIX tView = view.Transpose();
        XMMATRIX tProj = proj.Transpose();
        for (int r = 0; r < 4; r++)
        {
            for (int c = 0; c < 4; c++)
            {
                int i = r * 4 + c;
                commonCB->variableData["m_matWorld"].pArrayStart[i] = tWorld.r[r].m128_f32[c];
                commonCB->variableData["m_matView"].pArrayStart[i] = tView.r[r].m128_f32[c];
                commonCB->variableData["m_matProj"].pArrayStart[i] = tProj.r[r].m128_f32[c];
            }
        }

        // カメラ座標
        XMVECTOR camPos = m_camera->GetPosition();
        for (int i = 0; i < 3; i++)
            commonCB->variableData["f_cameraPositionWS"].pArrayStart[i] = camPos.m128_f32[i];
    }
    // ライト定数バッファの更新
    {
        ConstantBuffer* lightCB = m_constantBuffers.size() <= 1 || m_constantBuffers[1] == nullptr ? nullptr : m_constantBuffers[1].get();

        if (lightCB != nullptr)
        {
            XMVECTOR buff;
            // メインライトの向き
            buff = SimpleMath::Vector3::Transform(SimpleMath::Vector3::Down, SimpleMath::Matrix::CreateFromYawPitchRoll({ 45.0f, 45.0f, 0.0f }));
            for (int i = 0; i < 3; i++)
                lightCB->variableData["f_mainLightDirection"].pArrayStart[i] = buff.m128_f32[i];
            // メインライトの色
            for (int i = 0; i < 4; i++)
                lightCB->variableData["c_mainLightColor"].pArrayStart[i] = Colors::White.f[i];
            // 環境光の色
            for (int i = 0; i < 4; i++)
                lightCB->variableData["c_ambientLightColor"].pArrayStart[i] = Colors::White.f[i];
            // 環境光の強さ
            lightCB->variableData["f_ambientLightIntensity"].pArrayStart[0] = 0.3f;
        }
    }
    // PBRシェーダー用の定数バッファを更新
    {
        ConstantBuffer* pbrCB = m_constantBuffers.size() <= 2 || m_constantBuffers[2] == nullptr ? nullptr : m_constantBuffers[2].get();

        if (pbrCB != nullptr)
        {
            // ベースカラー
            for (int i = 0; i < 4; i++)
                pbrCB->variableData["c_baseColor"].pArrayStart[i] = Colors::White.f[i];
            // テクスチャを使う
            pbrCB->variableData["t_useAlbedoTexture"].pArrayStart[0] = true;
            // 法線マップを使う
            pbrCB->variableData["t_useNormalMap"].pArrayStart[0] = true;
            // 金属
            pbrCB->variableData["f_metallic"].pArrayStart[0] = METALLIC;
            // なめらかさ
            pbrCB->variableData["f_smoothness"].pArrayStart[0] = SMOOTHNESS;
        }
    }


    // すべてのバッファを更新
    for(int i = 0; i < (int)m_constantBuffers.size(); i++)
        context->UpdateSubresource(m_constantBuffers[i]->pBuffer.Get(), 0, NULL, m_constantBuffers[i]->pValues, 0, 0);


    
    //////////////////
    //     描画     //
    //////////////////


    // グリッドの描画
    m_grid->Render(context, view, proj);



    // モデルの描画
#if MODELTYPE == 0
    m_geometricPrimitive->Draw({}, {}, {}, Colors::White, nullptr, false,
#else
    m_model->Draw(context, *m_states, {}, {}, {}, false, 
#endif
        [&]()
        {
            // インプットレイアウトの設定
            context->IASetInputLayout(m_inputLayout.Get());

            // シェーダーの設定
            context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
            context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

            // 定数バッファの設定
            std::vector<ID3D11Buffer*> cb{ m_constantBuffers.size() };
            for (int i = 0; i < (int)m_constantBuffers.size(); i++)
                cb[i] = m_constantBuffers[i]->pBuffer.Get();
            context->VSSetConstantBuffers(0, (UINT)cb.size(), cb.data());
            context->PSSetConstantBuffers(0, (UINT)cb.size(), cb.data());

            // テクスチャの設定
            std::vector<ID3D11ShaderResourceView*> tex = {
                m_cubemap.Get(),
                m_albedo.Get(),
                m_normalMap.Get(),
            };
            context->VSSetShaderResources(0, (UINT)tex.size(), tex.data());
            context->PSSetShaderResources(0, (UINT)tex.size(), tex.data());
            // サンプラーの設定
            auto sampler = m_states->LinearClamp();
            context->VSSetSamplers(0, 1, &sampler);
            context->PSSetSamplers(0, 1, &sampler);
        });


    // スカイボックスの描画
    m_skyboxModel->Draw({}, {}, {}, Colors::White, nullptr, false, [&]()
        {
            // インプットレイアウトの設定
            context->IASetInputLayout(m_skyInputLayout.Get());

            // シェーダーの設定
            context->VSSetShader(m_skyVertexShader.Get(), nullptr, 0);
            context->PSSetShader(m_skyPixelShader.Get(), nullptr, 0);

            // 定数バッファの設定（共通定数バッファのみ）
            context->VSSetConstantBuffers(0, 1, m_constantBuffers[0]->pBuffer.GetAddressOf());
            context->PSSetConstantBuffers(0, 1, m_constantBuffers[0]->pBuffer.GetAddressOf());

            // テクスチャの設定
            context->VSSetShaderResources(0, 1, m_cubemap.GetAddressOf());
            context->PSSetShaderResources(0, 1, m_cubemap.GetAddressOf());
            // サンプラーの設定
            auto sampler = m_states->LinearClamp();
            context->VSSetSamplers(0, 1, &sampler);
            context->PSSetSamplers(0, 1, &sampler);
        });
}



/// <summary>
/// 終了処理
/// </summary>
void Scene::Finalize()
{
}






/// <summary>
/// シェーダーリフレクションから定数バッファの作成
/// </summary>
/// <param name="shaderRef">シェーダーリフレクション</param>
void Scene::CreateConstantBufferFromReflect(ID3D11ShaderReflection* shaderRef)
{
    // シェーダー情報の取得
    D3D11_SHADER_DESC shaderDesc = {};
    shaderRef->GetDesc(&shaderDesc);

    D3D11_SHADER_INPUT_BIND_DESC bindDesc{};
    for (int i = 0; i < (int)shaderDesc.ConstantBuffers; i++)
    {
        std::unique_ptr<ConstantBuffer> cbData = std::make_unique<ConstantBuffer>();

        // 定数バッファのリフレクションを取得
        ID3D11ShaderReflectionConstantBuffer* cbRef = shaderRef->GetConstantBufferByIndex(i);
        // 定数バッファの情報を取得
        D3D11_SHADER_BUFFER_DESC cbDesc{};
        cbRef->GetDesc(&cbDesc);

        // 名前の取得
        if (cbDesc.Name != 0)
        {
            cbData->bufferName = cbDesc.Name;
        }

        // サイズに応じた配列を作成
        int arrayNum = cbDesc.Size / 4;
        cbData->pValues = new float[arrayNum];

        // 値の取得
        int arrayOffset = 0;
        for (int j = 0; j < (int)cbDesc.Variables; j++)
        {
            ConstantBufferVariable variableData = {};

            // 変数の情報を取得
            ID3D11ShaderReflectionVariable* valRef = cbRef->GetVariableByIndex(j);
            D3D11_SHADER_VARIABLE_DESC valDesc{};
            valRef->GetDesc(&valDesc);

            // 名前と変数タイプを取得
            variableData.variableName = valDesc.Name;

            // スタート位置のポインタを取得
            variableData.pArrayStart = &cbData->pValues[arrayOffset];
            // 初期化
            for (int k = 0; k < (int)valDesc.Size / 4; k++)
                variableData.pArrayStart[k] = 0;

            // 変数のサイズを取得
            variableData.byteSize = valDesc.Size;


            // 連想配列に登録
            cbData->variableData[variableData.variableName] = variableData;
            // 配列のオフセットをずらす
            arrayOffset += valDesc.Size / 4;
        }


        // デバイス
        ID3D11Device* device = m_pDeviceResources->GetD3DDevice();
        // 定数バッファの作成
        CD3D11_BUFFER_DESC buffDesc
        {
            cbDesc.Size,
            D3D11_BIND_CONSTANT_BUFFER
        };
        DX::ThrowIfFailed(
            device->CreateBuffer(
                &buffDesc,
                nullptr,
                cbData->pBuffer.ReleaseAndGetAddressOf()
            )
        );


        // バインド情報を取得し、配列インデックスを取得
        shaderRef->GetResourceBindingDescByName(cbData->bufferName.c_str(), &bindDesc);
        assert(bindDesc.Type == D3D_SIT_CBUFFER);
        UINT arrayIndex = bindDesc.BindPoint;

        // 配列のサイズが足りないときは、追加する
        if (arrayIndex >= (UINT)m_constantBuffers.size())
            m_constantBuffers.resize((size_t)arrayIndex + 1u);
        m_constantBuffers[arrayIndex] = std::move(cbData);
    }
}
