/**********************************************
 *
 *  FreeLookCamera.h
 *  フリールックカメラ
 *
 *  製作者：牛丸文仁
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
		// マウス感度
		void SetMouseSensitivity(const float& sensitivity) { m_mouseSensitivity = sensitivity; }
		// キーボード速度
		void SetKeyboardSpeed(const float& speed) { m_keyboardSensitivity = speed; }

		// 座標の設定
		void SetPosition(const DirectX::SimpleMath::Vector3& position) override
		{
			// 距離を設定
			DirectX::SimpleMath::Vector3 toTarget = m_target - position;
			m_distance = toTarget.Length();

			// 回転量を計算
			DirectX::SimpleMath::Quaternion rot =
				DirectX::SimpleMath::Quaternion::FromToRotation(
					DirectX::SimpleMath::Vector3::Forward,
					toTarget
				);
			// オイラー角で保持
			auto euler = rot.ToEuler();
			m_angle.x = euler.y;
			m_angle.y = euler.x;

			ApplyAngle();
		}

		// ターゲット座標
		void SetTarget(const DirectX::SimpleMath::Vector3& target) { m_target = target; ApplyAngle(); }



	public:
		// ターゲット座標
		const DirectX::SimpleMath::Vector3& GetTarget()const { return m_target; }
		// マウス感度
		float GetMouseSensitivity() const { return m_mouseSensitivity; }
		// キーボード感度
		float GetKeyboardSensitivity() const { return m_keyboardSensitivity; }



	public:
		FreeLookCamera(const int& screenWidth, const int& screenHeight);
		~FreeLookCamera() = default;

		// マウスによる更新
		void Update(const DirectX::Mouse::State& mouseState, const float& deltaTime);



	private:
		// 回転を適応
		void ApplyAngle();



	private:
		// デフォルトの距離
		static const float DEFAULT_DISTANCE;


	private:
		// 注視点
		DirectX::SimpleMath::Vector3 m_target;
		// 回転
		DirectX::SimpleMath::Vector2 m_angle;
		// 距離
		float m_distance;

		// マウス速度
		float m_mouseSensitivity;
		// マウス座標の前回値（絶対値モード）
		DirectX::SimpleMath::Vector2 m_oldMousePos;
		// クリックの前回値
		bool m_oldClick;

		// キーボード速度
		float m_keyboardSensitivity;
	};
}