/**********************************************
 *
 *  FreeLookCamera.cpp
 *  フリールックカメラ
 *
 *  製作者：牛丸文仁
 *
 **********************************************/
#include "pch.h"
#include "FreeLookCamera.h"

using namespace DirectX;

// デフォルト値
const float AirL::FreeLookCamera::DEFAULT_DISTANCE = 5.0f;



/// <summary>
/// コンストラクタ
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
/// マウスによる更新
/// </summary>
/// <param name="mouseState">マウスステート</param>
void AirL::FreeLookCamera::Update(const Mouse::State& mouseState, const float& deltaTime)
{
	int deltaWheeleValue = -mouseState.scrollWheelValue;
	m_distance += (float)deltaWheeleValue * 0.01f;
	m_distance = std::max(m_distance, 0.01f);

	bool click = mouseState.leftButton;
	if (click)
	{
		// 絶対値モード
		if (mouseState.positionMode == Mouse::MODE_ABSOLUTE)
		{
			// 開始座標を保持
			if (!m_oldClick && click)
				m_oldMousePos = { (float)mouseState.x, (float)mouseState.y };

			// 移動量を算出
			SimpleMath::Vector2 mousePos = { (float)mouseState.x, (float)mouseState.y };
			SimpleMath::Vector2 deltaPos = m_oldMousePos - mousePos;
			// 感度を適応
			deltaPos *= m_mouseSensitivity;

			m_angle += deltaPos * deltaTime;

			m_oldMousePos = mousePos;
		}
		// 相対モード
		else
		{
			SimpleMath::Vector2 deltaPos = { (float)mouseState.x, (float)mouseState.y };
			// 感度を適応
			deltaPos *= m_mouseSensitivity;

			m_angle += -deltaPos * deltaTime;
		}
	}
	// 回転を適応
	ApplyAngle();

	// クリックの保持
	m_oldClick = click;
}





/// <summary>
/// 回転の適応
/// </summary>
void AirL::FreeLookCamera::ApplyAngle()
{
	// 回転をクランプ
	const float OFFSET = 0.001f;
	m_angle.y = std::min(std::max(m_angle.y, -XM_PIDIV2 + OFFSET), XM_PIDIV2 - OFFSET);
	// 回転をループ
	m_angle.x = std::fmodf(m_angle.x, XM_2PI);

	// 回転を作成
	SimpleMath::Quaternion rot =
		SimpleMath::Quaternion::CreateFromYawPitchRoll({m_angle.y, m_angle.x, 0.0f});
	Camera::SetRotation(rot);

	// 座標の作成
	SimpleMath::Vector3 pos = 
		m_target +
		SimpleMath::Vector3::Transform(SimpleMath::Vector3::Backward, rot) * m_distance;
	// 適応
	Camera::SetPosition(pos);
}
