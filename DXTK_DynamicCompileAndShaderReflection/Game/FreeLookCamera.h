/**********************************************
 *
 *  FreeLookCamera.h
 *  �t���[���b�N�J����
 *
 *  ����ҁF���ە��m
 *
 **********************************************/
#pragma once
#include "Camera.h"
#include "Keyboard.h"
#include "Mouse.h"

namespace AirL
{
	class FreeLookCamera : public Camera
	{
	public:		// Setter
		// �}�E�X���x
		void SetMouseSensitivity(const float& sensitivity) { m_mouseSensitivity = sensitivity; }
		// �L�[�{�[�h���x
		void SetKeyboardSpeed(const float& speed) { m_keyboardSensitivity = speed; }

		// ���W�̐ݒ�
		void SetPosition(const DirectX::SimpleMath::Vector3& position) override
		{
			// ������ݒ�
			DirectX::SimpleMath::Vector3 toTarget = m_target - position;
			m_distance = toTarget.Length();

			// ��]�ʂ��v�Z
			DirectX::SimpleMath::Quaternion rot =
				DirectX::SimpleMath::Quaternion::FromToRotation(
					DirectX::SimpleMath::Vector3::Forward,
					toTarget
				);
			// �I�C���[�p�ŕێ�
			auto euler = rot.ToEuler();
			m_angle.x = euler.y;
			m_angle.y = euler.x;

			ApplyAngle();
		}

		// �^�[�Q�b�g���W
		void SetTarget(const DirectX::SimpleMath::Vector3& target) { m_target = target; ApplyAngle(); }



	public:
		// �^�[�Q�b�g���W
		const DirectX::SimpleMath::Vector3& GetTarget()const { return m_target; }
		// �}�E�X���x
		float GetMouseSensitivity() const { return m_mouseSensitivity; }
		// �L�[�{�[�h���x
		float GetKeyboardSensitivity() const { return m_keyboardSensitivity; }



	public:
		FreeLookCamera(const int& screenWidth, const int& screenHeight);
		~FreeLookCamera() = default;

		// �}�E�X�ɂ��X�V
		void Update(const DirectX::Mouse::State& mouseState, const float& deltaTime);



	private:
		// ��]��K��
		void ApplyAngle();



	private:
		// �f�t�H���g�̋���
		static const float DEFAULT_DISTANCE;


	private:
		// �����_
		DirectX::SimpleMath::Vector3 m_target;
		// ��]
		DirectX::SimpleMath::Vector2 m_angle;
		// ����
		float m_distance;

		// �}�E�X���x
		float m_mouseSensitivity;
		// �}�E�X���W�̑O��l�i��Βl���[�h�j
		DirectX::SimpleMath::Vector2 m_oldMousePos;
		// �N���b�N�̑O��l
		bool m_oldClick;

		// �L�[�{�[�h���x
		float m_keyboardSensitivity;
	};
}