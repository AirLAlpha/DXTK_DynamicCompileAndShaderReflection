/**********************************************
 *
 *  Camera.cpp
 *  �J����
 *
 *  ����ҁF���ە��m
 *
 **********************************************/
#include "pch.h"
#include "Camera.h"

using namespace DirectX;

const SimpleMath::Vector3 AirL::Camera::CAM_DEFAULT_POSITION = SimpleMath::Vector3::Backward;
const SimpleMath::Vector3 AirL::Camera::CAM_DEFAULT_TARGET	 = SimpleMath::Vector3::Zero;
const float AirL::Camera::CAM_DEFAULT_NEAR_PLANE			 = 0.1f;
const float AirL::Camera::CAM_DEFAULT_FAR_PLANE				 = 100.0f;
const float AirL::Camera::CAM_DEFAULT_FOV					 = 45.0f;



/// <summary>
/// �R���X�g���N�^
/// </summary>
/// <param name="screenWidth">��ʃT�C�Y�iX�j</param>
/// <param name="screenHeight">��ʃT�C�Y�iY�j</param>
AirL::Camera::Camera(const int& screenWidth, const int& screenHeight):
	m_pos(CAM_DEFAULT_POSITION),
	m_rot(SimpleMath::Quaternion::Identity),
	m_nearPlane(CAM_DEFAULT_NEAR_PLANE),
	m_farPlane(CAM_DEFAULT_FAR_PLANE),
	m_fov(CAM_DEFAULT_FOV),
	m_screenWidth(screenWidth),
	m_screenHeight(screenHeight)
{
	UpdateViewMatrix();
	UpdateProjectionMatrix();
}



/// <summary>
/// �s��̍X�V
/// </summary>
void AirL::Camera::UpdateViewMatrix()
{
	SimpleMath::Vector3 target = SimpleMath::Vector3::Transform(SimpleMath::Vector3::Forward, m_rot) + m_pos;

	m_matView = SimpleMath::Matrix::CreateLookAt(
		m_pos,
		target,
		SimpleMath::Vector3::Up
	);
}

void AirL::Camera::UpdateProjectionMatrix()
{
	m_matProj = SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		XMConvertToRadians(m_fov),
		(float)m_screenWidth / (float)m_screenHeight,
		m_nearPlane,
		m_farPlane
	);
}