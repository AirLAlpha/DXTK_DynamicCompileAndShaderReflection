/**********************************************
 *
 *  FreeLookCamera.cpp
 *  �t���[���b�N�J����
 *
 *  ����ҁF���ە��m
 *
 **********************************************/
#include "pch.h"
#include "FreeLookCamera.h"

using namespace DirectX;

// �f�t�H���g�l
const float AirL::FreeLookCamera::DEFAULT_DISTANCE = 5.0f;



/// <summary>
/// �R���X�g���N�^
/// </summary>
/// <param name="screenWidth"></param>
/// <param name="screenHeight"></param>
AirL::FreeLookCamera::FreeLookCamera(const int& screenWidth, const int& screenHeight):
	Camera(screenWidth, screenHeight),
	m_mouseSensitivity(1.0f),
	m_distance(DEFAULT_DISTANCE),
	m_oldClick(false),
	m_keyboardSensitivity(1.0f),
	m_angle(0.0f, XM_PI * -0.25f)
{
}





/// <summary>
/// �}�E�X�ɂ��X�V
/// </summary>
/// <param name="mouseState">�}�E�X�X�e�[�g</param>
void AirL::FreeLookCamera::Update(const Mouse::State& mouseState, const float& deltaTime)
{
	int deltaWheeleValue = -mouseState.scrollWheelValue;
	m_distance += (float)deltaWheeleValue * 0.01f;
	m_distance = std::max(m_distance, 0.01f);

	bool click = mouseState.leftButton;
	if (click)
	{
		// ��Βl���[�h
		if (mouseState.positionMode == Mouse::MODE_ABSOLUTE)
		{
			// �J�n���W��ێ�
			if (!m_oldClick && click)
				m_oldMousePos = { (float)mouseState.x, (float)mouseState.y };

			// �ړ��ʂ��Z�o
			SimpleMath::Vector2 mousePos = { (float)mouseState.x, (float)mouseState.y };
			SimpleMath::Vector2 deltaPos = m_oldMousePos - mousePos;
			// ���x��K��
			deltaPos *= m_mouseSensitivity;

			m_angle += deltaPos * deltaTime;

			m_oldMousePos = mousePos;
		}
		// ���΃��[�h
		else
		{
			SimpleMath::Vector2 deltaPos = { (float)mouseState.x, (float)mouseState.y };
			// ���x��K��
			deltaPos *= m_mouseSensitivity;

			m_angle += -deltaPos * deltaTime;
		}
	}
	// ��]��K��
	ApplyAngle();

	// �N���b�N�̕ێ�
	m_oldClick = click;
}





/// <summary>
/// ��]�̓K��
/// </summary>
void AirL::FreeLookCamera::ApplyAngle()
{
	// ��]���N�����v
	const float OFFSET = 0.001f;
	m_angle.y = std::min(std::max(m_angle.y, -XM_PIDIV2 + OFFSET), XM_PIDIV2 - OFFSET);
	// ��]�����[�v
	m_angle.x = std::fmodf(m_angle.x, XM_2PI);

	// ��]���쐬
	SimpleMath::Quaternion rot =
		SimpleMath::Quaternion::CreateFromYawPitchRoll({m_angle.y, m_angle.x, 0.0f});
	Camera::SetRotation(rot);

	// ���W�̍쐬
	SimpleMath::Vector3 pos = 
		m_target +
		SimpleMath::Vector3::Transform(SimpleMath::Vector3::Backward, rot) * m_distance;
	// �K��
	Camera::SetPosition(pos);
}
