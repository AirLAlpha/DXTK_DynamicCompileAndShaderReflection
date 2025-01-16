/**********************************************
 *
 *  Common.hlsli
 *  �������ׂẴV�F�[�_�[��include����
 *
 *  ����ҁF���� ���m
 *
 **********************************************/

// ���ʒ萔�o�b�t�@
cbuffer CommonConstBuffer : register(b0)
{
    matrix m_matWorld;          // ���[���h�s��
    matrix m_matView;           // �r���[�s��
    matrix m_matProj;           // �ˉe�s��

    float3 f_cameraPositionWS;  // �J�����̃��[���h���W
}

// ���ʃ��C�g�萔�o�b�t�@
cbuffer LightConstBuffer : register(b1)
{
    float3 f_mainLightDirection;    // ���C�����C�g�i�f�B���N�V���i�����C�g�j�̌���
    float  f_mainLightBrightness;   // ���C�����C�g�̋���
    float4 c_mainLightColor;        // ���C�����C�g�̐F
    float4 c_ambientLightColor;     // �����̐F
    float f_ambientLightIntensity;  // �����̋���
}

// �X�J�C�{�b�N�X��CubeMap
TextureCube<float4> g_skybox : register(t0);
// �T���v���[
SamplerState g_sampler : register(s0);

// PI
static const float F_PI = 3.1415926f;
