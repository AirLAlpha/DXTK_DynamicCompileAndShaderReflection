/**********************************************
 *
 *  Scene.cpp
 *  �V�[��
 *
 *  ����ҁF���ە��m
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

// ���f���^�C�v�F(0:�W�I���g���b�N�v���~�e�B�u�A1:CMO�A2:SDKMESH�j
#define MODELTYPE (0)
// �C���v�b�g���C�A�E�g�̃}�N����CMO�ɂ���
#define ILTYPE_CMO (false)

// �e�N�X�`�����g��
#define USE_ALBEDO_TEXTURE (true)
// �@���}�b�v���g��
#define USE_NORMAL_TEXTURE (true)

// ���^���b�N
#define METALLIC    (1.0f)
// ���炩��
#define SMOOTHNESS  (0.5f) 




/// <summary>
/// �R���X�g���N�^
/// </summary>
/// <param name="pDeviceResources">�f�o�C�X���\�[�X�̃|�C���^</param>
Scene::Scene(DX::DeviceResources* pDeviceResources):
	m_pDeviceResources(pDeviceResources)
{
}



/// <summary>
/// ����������
/// </summary>
void Scene::Initialize()
{
    // �f�o�C�X
    ID3D11Device* device = m_pDeviceResources->GetD3DDevice();
    ID3D11DeviceContext* context = m_pDeviceResources->GetD3DDeviceContext();

    // �}�E�X�̍쐬
    m_mouse = std::make_unique<Mouse>();
    m_mouseButtonStateTracker = std::make_unique<Mouse::ButtonStateTracker>();

    // �J�����̍쐬
    RECT windowRect = m_pDeviceResources->GetOutputSize();
    m_camera = std::make_unique<AirL::FreeLookCamera>((int)windowRect.right, (int)windowRect.bottom);

    // �R�����X�e�[�g�̍쐬
    m_states = std::make_unique<CommonStates>(device);
    // �O���b�h�̍쐬
    m_grid = std::make_unique<AirL::Grid>(device, context);

    // ���f���̓ǂݍ���
    std::unique_ptr<EffectFactory> fx = std::make_unique<EffectFactory>(device);

#if MODELTYPE == 0
    // �W�I���g���b�N�v���~�e�B�u�̍쐬
    m_geometricPrimitive = GeometricPrimitive::CreateTeapot(context);
#elif MODELTYPE == 1
    // CMO���f��
    m_model = Model::CreateFromCMO(device, L"Resources/CMO/torus.cmo", *fx);
#elif MODELTYPE == 2
    // SDKMESH���f��
    m_model = Model::CreateFromSDKMESH(device, L"Resources/SDKMESH/bunny.sdkmesh", *fx);
#endif


    // �X�J�C�{�b�N�X�p���f���̍쐬
    m_skyboxModel = GeometricPrimitive::CreateGeoSphere(context, 2.0f, 3, false);


    Microsoft::WRL::ComPtr<ID3DBlob> blob, errorMsg;

    //////////////////////////
    // ���_�V�F�[�_�[�̍쐬 //
    //////////////////////////

    // �}�N����`
    const D3D_SHADER_MACRO macros[] = {
#if ILTYPE_CMO
        {"MODELTYPE_CMO", ""},
#endif
        {NULL, NULL},
    };

    // ���_�V�F�[�_�[�̃R���p�C��
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

    // ���_�V�F�[�_�[�I�u�W�F�N�g�̍쐬
    DX::ThrowIfFailed(
        device->CreateVertexShader(
            blob->GetBufferPointer(),
            blob->GetBufferSize(),
            nullptr,
            m_vertexShader.ReleaseAndGetAddressOf()
        )
    );

    // ���_�V�F�[�_�[�̃V�F�[�_�[���t���N�V����
    Microsoft::WRL::ComPtr<ID3D11ShaderReflection> vsRef;
    D3DReflect(
        blob->GetBufferPointer(),
        blob->GetBufferSize(),
        IID_ID3D11ShaderReflection,
        (void**)vsRef.ReleaseAndGetAddressOf()
    );

    // ���t���N�V�����̏����擾
    D3D11_SHADER_DESC vsShaderDesc{};
    vsRef->GetDesc(&vsShaderDesc);

    // �C���v�b�g���C�A�E�g�̏����擾
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements{ vsShaderDesc.InputParameters };
    for (int i = 0; i < (int)vsShaderDesc.InputParameters; i++)
    {
        // �v�f�ЂƂ��̏����擾
        D3D11_SIGNATURE_PARAMETER_DESC sigDesc;
        vsRef->GetInputParameterDesc(i, &sigDesc);
        // �t�H�[�}�b�g���擾�iMask�� float3:0b0111, float4:0b1111 �̂悤�ɒl�̐��ɂ���Đ��l���ω�����j
        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
        if ((sigDesc.Mask & 0x0F) == 0x0F) format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        else if ((sigDesc.Mask & 0x07) == 0x07) format = DXGI_FORMAT_R32G32B32_FLOAT;
        else if ((sigDesc.Mask & 0x07) == 0x03) format = DXGI_FORMAT_R32G32_FLOAT;
        else if ((sigDesc.Mask & 0x07) == 0x01) format = DXGI_FORMAT_R32_FLOAT;

        // �ЂƂ��̗v�f���쐬
        D3D11_INPUT_ELEMENT_DESC elemDesc =
        {
            sigDesc.SemanticName,
            sigDesc.SemanticIndex,
            format,
            0,									// ���ߑł�
            D3D11_APPEND_ALIGNED_ELEMENT,		// ���ߑł�
            D3D11_INPUT_PER_VERTEX_DATA,		// ���ߑł�
            0									// ���ߑł�
        };

        // �z��ɒǉ�
        inputElements[i] = elemDesc;
    }

    // �C���v�b�g���C�A�E�g�̍쐬
    DX::ThrowIfFailed(
        device->CreateInputLayout(
            &inputElements[0],
            (UINT)inputElements.size(),
            blob->GetBufferPointer(),
            blob->GetBufferSize(),
            m_inputLayout.ReleaseAndGetAddressOf()
        )
    );

    // ���_�V�F�[�_�[�����p����萔�o�b�t�@�̍쐬
    CreateConstantBufferFromReflect(vsRef.Get());



    //////////////////////////////
    // �s�N�Z���V�F�[�_�[�̍쐬 //
    //////////////////////////////

    // �s�N�Z���V�F�[�_�[�̃R���p�C��
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

    // �s�N�Z���V�F�[�_�[�I�u�W�F�N�g�̍쐬
    DX::ThrowIfFailed(
        device->CreatePixelShader(
            blob->GetBufferPointer(),
            blob->GetBufferSize(),
            nullptr,
            m_pixelShader.ReleaseAndGetAddressOf()
        )
    );

    // �V�F�[�_�[���t���N�V����
    Microsoft::WRL::ComPtr<ID3D11ShaderReflection> psRef;
    DX::ThrowIfFailed(
        D3DReflect(
            blob->GetBufferPointer(),
            blob->GetBufferSize(),
            IID_ID3D11ShaderReflection,
            (void**)psRef.ReleaseAndGetAddressOf()
        )
    );

    // �萔�o�b�t�@�̍쐬
    CreateConstantBufferFromReflect(psRef.Get());



    //////////////////////////
    // �e�N�X�`���̓ǂݍ��� //
    //////////////////////////

    // �L���[�u�}�b�v�̓ǂݍ���
    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(
            device,
            L"Resources/DDS/cubemap.dds",
            nullptr,
            m_cubemap.ReleaseAndGetAddressOf()
        )
    );

    // �A���x�h�e�N�X�`���̓ǂݍ���
    DX::ThrowIfFailed(
        CreateWICTextureFromFile(
            device,
            L"Resources/Texture/wood_floor_worn_diff_2k.png",
            nullptr,
            m_albedo.ReleaseAndGetAddressOf()
        )
    );

    // �m�[�}���}�b�v�̓ǂݍ���
    DX::ThrowIfFailed(
        CreateWICTextureFromFile(
            device,
            L"Resources/Texture/wood_floor_worn_nor_dx_2k.png",
            nullptr,
            m_normalMap.ReleaseAndGetAddressOf()
        )
    );
    
    //////////////////////////////////////////////
    // �X�J�C�{�b�N�X�̃V�F�[�_�[�i.cso�j���쐬 //
    //////////////////////////////////////////////
    
    auto skyVSData = DX::ReadData(L"Resources/CSO/Skybox_VS.cso");
    auto skyPSData = DX::ReadData(L"Resources/CSO/Skybox_PS.cso");

    // �C���v�b�g���C�A�E�g
    DX::ThrowIfFailed(
        device->CreateInputLayout(
            VertexPosition::InputElements,
            VertexPosition::InputElementCount,
            skyVSData.data(),
            skyVSData.size(),
            m_skyInputLayout.ReleaseAndGetAddressOf()
        )
    );

    // ���_�V�F�[�_�[
    DX::ThrowIfFailed(
        device->CreateVertexShader(
            skyVSData.data(),
            skyVSData.size(),
            nullptr,
            m_skyVertexShader.ReleaseAndGetAddressOf()
        )
    );
    // �s�N�Z���V�F�[�_�[
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
/// �X�V����
/// </summary>
/// <param name="deltaTime"></param>
void Scene::Update(const float& deltaTime)
{
    // �}�E�X�X�e�[�g�g���b�J�[�̍X�V
    m_mouseButtonStateTracker->Update(m_mouse->GetState());

    // �J�����ɉ�ʃT�C�Y��ݒ�
    RECT windowRect = m_pDeviceResources->GetOutputSize();
    m_camera->SetScreenResolution((int)windowRect.right, (int)windowRect.bottom);
    // �J�����̍X�V
    m_camera->Update(m_mouseButtonStateTracker->GetLastState(), deltaTime);

    // �}�E�X�̃X�N���[���ʂ����Z�b�g
    m_mouse->ResetScrollWheelValue();
}



/// <summary>
/// �`�揈��
/// </summary>
void Scene::Render()
{
    // �R���e�L�X�g
    ID3D11DeviceContext* context = m_pDeviceResources->GetD3DDeviceContext();

    // ���[���h�s��̍쐬
    SimpleMath::Matrix world =
        SimpleMath::Matrix::CreateRotationY(XM_PI) *
        SimpleMath::Matrix::Identity;

    // �r���[�s��̎擾
    SimpleMath::Matrix view = m_camera->GetView();
    // �ˉe�s��̎擾
    SimpleMath::Matrix proj = m_camera->GetProjection();


    ////////////////////////
    // �萔�o�b�t�@�̍X�V //
    ////////////////////////

    // ���ʒ萔�o�b�t�@(CommonConstantBuffer)�̍X�V
    {
        ConstantBuffer* commonCB = m_constantBuffers[0].get();

        // �e�s��
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

        // �J�������W
        XMVECTOR camPos = m_camera->GetPosition();
        for (int i = 0; i < 3; i++)
            commonCB->variableData["f_cameraPositionWS"].pArrayStart[i] = camPos.m128_f32[i];
    }
    // ���C�g�萔�o�b�t�@�̍X�V
    {
        ConstantBuffer* lightCB = m_constantBuffers.size() <= 1 || m_constantBuffers[1] == nullptr ? nullptr : m_constantBuffers[1].get();

        if (lightCB != nullptr)
        {
            XMVECTOR buff;
            // ���C�����C�g�̌���
            buff = SimpleMath::Vector3::Transform(SimpleMath::Vector3::Down, SimpleMath::Matrix::CreateFromYawPitchRoll({ 45.0f, 45.0f, 0.0f }));
            for (int i = 0; i < 3; i++)
                lightCB->variableData["f_mainLightDirection"].pArrayStart[i] = buff.m128_f32[i];
            // ���C�����C�g�̐F
            for (int i = 0; i < 4; i++)
                lightCB->variableData["c_mainLightColor"].pArrayStart[i] = Colors::White.f[i];
            // �����̐F
            for (int i = 0; i < 4; i++)
                lightCB->variableData["c_ambientLightColor"].pArrayStart[i] = Colors::White.f[i];
            // �����̋���
            lightCB->variableData["f_ambientLightIntensity"].pArrayStart[0] = 0.3f;
        }
    }
    // PBR�V�F�[�_�[�p�̒萔�o�b�t�@���X�V
    {
        ConstantBuffer* pbrCB = m_constantBuffers.size() <= 2 || m_constantBuffers[2] == nullptr ? nullptr : m_constantBuffers[2].get();

        if (pbrCB != nullptr)
        {
            // �x�[�X�J���[
            for (int i = 0; i < 4; i++)
                pbrCB->variableData["c_baseColor"].pArrayStart[i] = Colors::White.f[i];
            // �e�N�X�`�����g��
            pbrCB->variableData["t_useAlbedoTexture"].pArrayStart[0] = true;
            // �@���}�b�v���g��
            pbrCB->variableData["t_useNormalMap"].pArrayStart[0] = true;
            // ����
            pbrCB->variableData["f_metallic"].pArrayStart[0] = METALLIC;
            // �Ȃ߂炩��
            pbrCB->variableData["f_smoothness"].pArrayStart[0] = SMOOTHNESS;
        }
    }


    // ���ׂẴo�b�t�@���X�V
    for(int i = 0; i < (int)m_constantBuffers.size(); i++)
        context->UpdateSubresource(m_constantBuffers[i]->pBuffer.Get(), 0, NULL, m_constantBuffers[i]->pValues, 0, 0);


    
    //////////////////
    //     �`��     //
    //////////////////


    // �O���b�h�̕`��
    m_grid->Render(context, view, proj);



    // ���f���̕`��
#if MODELTYPE == 0
    m_geometricPrimitive->Draw({}, {}, {}, Colors::White, nullptr, false,
#else
    m_model->Draw(context, *m_states, {}, {}, {}, false, 
#endif
        [&]()
        {
            // �C���v�b�g���C�A�E�g�̐ݒ�
            context->IASetInputLayout(m_inputLayout.Get());

            // �V�F�[�_�[�̐ݒ�
            context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
            context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

            // �萔�o�b�t�@�̐ݒ�
            std::vector<ID3D11Buffer*> cb{ m_constantBuffers.size() };
            for (int i = 0; i < (int)m_constantBuffers.size(); i++)
                cb[i] = m_constantBuffers[i]->pBuffer.Get();
            context->VSSetConstantBuffers(0, (UINT)cb.size(), cb.data());
            context->PSSetConstantBuffers(0, (UINT)cb.size(), cb.data());

            // �e�N�X�`���̐ݒ�
            std::vector<ID3D11ShaderResourceView*> tex = {
                m_cubemap.Get(),
                m_albedo.Get(),
                m_normalMap.Get(),
            };
            context->VSSetShaderResources(0, (UINT)tex.size(), tex.data());
            context->PSSetShaderResources(0, (UINT)tex.size(), tex.data());
            // �T���v���[�̐ݒ�
            auto sampler = m_states->LinearClamp();
            context->VSSetSamplers(0, 1, &sampler);
            context->PSSetSamplers(0, 1, &sampler);
        });


    // �X�J�C�{�b�N�X�̕`��
    m_skyboxModel->Draw({}, {}, {}, Colors::White, nullptr, false, [&]()
        {
            // �C���v�b�g���C�A�E�g�̐ݒ�
            context->IASetInputLayout(m_skyInputLayout.Get());

            // �V�F�[�_�[�̐ݒ�
            context->VSSetShader(m_skyVertexShader.Get(), nullptr, 0);
            context->PSSetShader(m_skyPixelShader.Get(), nullptr, 0);

            // �萔�o�b�t�@�̐ݒ�i���ʒ萔�o�b�t�@�̂݁j
            context->VSSetConstantBuffers(0, 1, m_constantBuffers[0]->pBuffer.GetAddressOf());
            context->PSSetConstantBuffers(0, 1, m_constantBuffers[0]->pBuffer.GetAddressOf());

            // �e�N�X�`���̐ݒ�
            context->VSSetShaderResources(0, 1, m_cubemap.GetAddressOf());
            context->PSSetShaderResources(0, 1, m_cubemap.GetAddressOf());
            // �T���v���[�̐ݒ�
            auto sampler = m_states->LinearClamp();
            context->VSSetSamplers(0, 1, &sampler);
            context->PSSetSamplers(0, 1, &sampler);
        });
}



/// <summary>
/// �I������
/// </summary>
void Scene::Finalize()
{
}






/// <summary>
/// �V�F�[�_�[���t���N�V��������萔�o�b�t�@�̍쐬
/// </summary>
/// <param name="shaderRef">�V�F�[�_�[���t���N�V����</param>
void Scene::CreateConstantBufferFromReflect(ID3D11ShaderReflection* shaderRef)
{
    // �V�F�[�_�[���̎擾
    D3D11_SHADER_DESC shaderDesc = {};
    shaderRef->GetDesc(&shaderDesc);

    D3D11_SHADER_INPUT_BIND_DESC bindDesc{};
    for (int i = 0; i < (int)shaderDesc.ConstantBuffers; i++)
    {
        std::unique_ptr<ConstantBuffer> cbData = std::make_unique<ConstantBuffer>();

        // �萔�o�b�t�@�̃��t���N�V�������擾
        ID3D11ShaderReflectionConstantBuffer* cbRef = shaderRef->GetConstantBufferByIndex(i);
        // �萔�o�b�t�@�̏����擾
        D3D11_SHADER_BUFFER_DESC cbDesc{};
        cbRef->GetDesc(&cbDesc);

        // ���O�̎擾
        if (cbDesc.Name != 0)
        {
            cbData->bufferName = cbDesc.Name;
        }

        // �T�C�Y�ɉ������z����쐬
        int arrayNum = cbDesc.Size / 4;
        cbData->pValues = new float[arrayNum];

        // �l�̎擾
        int arrayOffset = 0;
        for (int j = 0; j < (int)cbDesc.Variables; j++)
        {
            ConstantBufferVariable variableData = {};

            // �ϐ��̏����擾
            ID3D11ShaderReflectionVariable* valRef = cbRef->GetVariableByIndex(j);
            D3D11_SHADER_VARIABLE_DESC valDesc{};
            valRef->GetDesc(&valDesc);

            // ���O�ƕϐ��^�C�v���擾
            variableData.variableName = valDesc.Name;

            // �X�^�[�g�ʒu�̃|�C���^���擾
            variableData.pArrayStart = &cbData->pValues[arrayOffset];
            // ������
            for (int k = 0; k < (int)valDesc.Size / 4; k++)
                variableData.pArrayStart[k] = 0;

            // �ϐ��̃T�C�Y���擾
            variableData.byteSize = valDesc.Size;


            // �A�z�z��ɓo�^
            cbData->variableData[variableData.variableName] = variableData;
            // �z��̃I�t�Z�b�g�����炷
            arrayOffset += valDesc.Size / 4;
        }


        // �f�o�C�X
        ID3D11Device* device = m_pDeviceResources->GetD3DDevice();
        // �萔�o�b�t�@�̍쐬
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


        // �o�C���h�����擾���A�z��C���f�b�N�X���擾
        shaderRef->GetResourceBindingDescByName(cbData->bufferName.c_str(), &bindDesc);
        assert(bindDesc.Type == D3D_SIT_CBUFFER);
        UINT arrayIndex = bindDesc.BindPoint;

        // �z��̃T�C�Y������Ȃ��Ƃ��́A�ǉ�����
        if (arrayIndex >= (UINT)m_constantBuffers.size())
            m_constantBuffers.resize((size_t)arrayIndex + 1u);
        m_constantBuffers[arrayIndex] = std::move(cbData);
    }
}
