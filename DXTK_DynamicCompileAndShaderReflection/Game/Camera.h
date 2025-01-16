/**********************************************
 *
 *  Camera.h
 *  �J����
 *
 *  ����ҁF���ە��m
 *
 **********************************************/
#pragma once
#include "SimpleMath.h"

namespace AirL
{
	class Camera
	{
	public:		//	Getter
		// ���W�̎擾
		const DirectX::SimpleMath::Vector3& GetPosition()		const { return m_pos; }
		// ��]�̎擾
		const DirectX::SimpleMath::Quaternion& GetRotation()	const { return m_rot; }
		// ����p�̎擾
		const float& GetFieldOfViewDegree()						const { return m_fov; }


		// �r���[�s��̎擾
		const DirectX::SimpleMath::Matrix& GetView()			const { return m_matView; }
		// �ˉe�s��̎擾
		const DirectX::SimpleMath::Matrix& GetProjection()		const { return m_matProj; }


		// ���ʃx�N�g���̎擾
		DirectX::SimpleMath::Vector3 GetForward()	const { return DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::Forward, m_rot); }
		// �E���x�N�g���̎擾
		DirectX::SimpleMath::Vector3 GetRight()		const { return DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::Right, m_rot); }
		// ������x�N�g���̎擾
		DirectX::SimpleMath::Vector3 GetUp()		const { return DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::Up, m_rot); }



	public:		//	Setter
		// ���W�̐ݒ�
		virtual void SetPosition(const DirectX::SimpleMath::Vector3& position)
		{
			m_pos = position;
			UpdateViewMatrix();
		}
		// ��]�̐ݒ�
		virtual void SetRotation(const DirectX::SimpleMath::Quaternion& rotation)
		{
			m_rot = rotation;
			UpdateViewMatrix();
		}
		//	�`��͈͂̐ݒ�
		void SetNearPlane(const float& nearPlane)
		{
			m_nearPlane = nearPlane;
			UpdateProjectionMatrix();
		}
		void SetFarPlane(const float& farPlane)
		{
			m_farPlane = farPlane;
			UpdateProjectionMatrix();
		}
		void SetNearFarPlane(const float& nearPlane, const float& farPlane)
		{
			m_nearPlane = nearPlane;
			m_farPlane = farPlane;
			UpdateProjectionMatrix();
		}
		// ����p�̐ݒ�
		void SetFieldOfView(const float& fovDeg)
		{
			m_fov = fovDeg;
			UpdateProjectionMatrix();
		}
		// ��ʉ𑜓x�̐ݒ�
		void SetScreenResolution(const int& width, const int& height)
		{
			m_screenWidth = width;
			m_screenHeight = height;
			UpdateProjectionMatrix();
		}

		// �r���[�s��̐ݒ�
		virtual void SetView(
			const DirectX::SimpleMath::Vector3& position,
			const DirectX::SimpleMath::Vector3& target
		)
		{
			m_pos = position;

			DirectX::SimpleMath::Vector3 toTarget = target - position;
			m_rot = DirectX::SimpleMath::Quaternion::LookRotation(toTarget, DirectX::SimpleMath::Vector3::Up);
	
			UpdateViewMatrix();
		}
		void SetView(const DirectX::SimpleMath::Matrix& viewMat) { m_matView = viewMat; }

		// �ˉe�s��̐ݒ�
		void SetProjection(
			const float& nearPlane,
			const float& farPlane,
			const float& fov,
			const int& screenWidth,
			const int& screenHeight
		)
		{
			m_nearPlane = nearPlane;
			m_farPlane = farPlane;
			m_fov = fov;
			m_screenWidth = screenWidth;
			m_screenHeight = screenHeight;
			UpdateProjectionMatrix();
		}



	public:
		// �f�t�H���g�l
		static const DirectX::SimpleMath::Vector3	CAM_DEFAULT_POSITION;
		static const DirectX::SimpleMath::Vector3	CAM_DEFAULT_TARGET;
		static const float							CAM_DEFAULT_NEAR_PLANE;
		static const float							CAM_DEFAULT_FAR_PLANE;
		static const float							CAM_DEFAULT_FOV;


	public:
		// �R���X�g���N�^
		Camera(const int& screenWidth, const int& screenHeight);
		// �f�X�g���N�^
		~Camera() = default;

		// �s��̍X�V
		virtual void UpdateViewMatrix();
		virtual void UpdateProjectionMatrix();


	private:
		// ���W
		DirectX::SimpleMath::Vector3 m_pos;
		// ��]
		DirectX::SimpleMath::Quaternion m_rot;

		// �`��͈́i�߁j
		float m_nearPlane;
		// �`��͈́i���j
		float m_farPlane;
		// ����p�i�x�j
		float m_fov;
		// ��ʉ𑜓x
		int m_screenWidth;
		int m_screenHeight;


		// �r���[�s��
		DirectX::SimpleMath::Matrix m_matView;
		// �ˉe�s��
		DirectX::SimpleMath::Matrix m_matProj;
	};
}