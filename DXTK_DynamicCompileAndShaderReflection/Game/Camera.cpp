/**********************************************
 *
 *  Camera.cpp
 *  カメラ
 *
 *  製作者：牛丸文仁
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
/// コンストラクタ
/// </summary>
/// <param name="screenWidth">画面サイズ（X）</param>
/// <param name="screenHeight">画面サイズ（Y）</param>
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
/// 行列の更新
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