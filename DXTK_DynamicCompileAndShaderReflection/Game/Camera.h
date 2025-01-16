/**********************************************
 *
 *  Camera.h
 *  カメラ
 *
 *  製作者：牛丸文仁
 *
 **********************************************/
#pragma once
#include "SimpleMath.h"

namespace AirL
{
	class Camera
	{
	public:		//	Getter
		// 座標の取得
		const DirectX::SimpleMath::Vector3& GetPosition()		const { return m_pos; }
		// 回転の取得
		const DirectX::SimpleMath::Quaternion& GetRotation()	const { return m_rot; }
		// 視野角の取得
		const float& GetFieldOfViewDegree()						const { return m_fov; }


		// ビュー行列の取得
		const DirectX::SimpleMath::Matrix& GetView()			const { return m_matView; }
		// 射影行列の取得
		const DirectX::SimpleMath::Matrix& GetProjection()		const { return m_matProj; }


		// 正面ベクトルの取得
		DirectX::SimpleMath::Vector3 GetForward()	const { return DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::Forward, m_rot); }
		// 右向ベクトルの取得
		DirectX::SimpleMath::Vector3 GetRight()		const { return DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::Right, m_rot); }
		// 上向きベクトルの取得
		DirectX::SimpleMath::Vector3 GetUp()		const { return DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::Up, m_rot); }



	public:		//	Setter
		// 座標の設定
		virtual void SetPosition(const DirectX::SimpleMath::Vector3& position)
		{
			m_pos = position;
			UpdateViewMatrix();
		}
		// 回転の設定
		virtual void SetRotation(const DirectX::SimpleMath::Quaternion& rotation)
		{
			m_rot = rotation;
			UpdateViewMatrix();
		}
		//	描画範囲の設定
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
		// 視野角の設定
		void SetFieldOfView(const float& fovDeg)
		{
			m_fov = fovDeg;
			UpdateProjectionMatrix();
		}
		// 画面解像度の設定
		void SetScreenResolution(const int& width, const int& height)
		{
			m_screenWidth = width;
			m_screenHeight = height;
			UpdateProjectionMatrix();
		}

		// ビュー行列の設定
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

		// 射影行列の設定
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
		// デフォルト値
		static const DirectX::SimpleMath::Vector3	CAM_DEFAULT_POSITION;
		static const DirectX::SimpleMath::Vector3	CAM_DEFAULT_TARGET;
		static const float							CAM_DEFAULT_NEAR_PLANE;
		static const float							CAM_DEFAULT_FAR_PLANE;
		static const float							CAM_DEFAULT_FOV;


	public:
		// コンストラクタ
		Camera(const int& screenWidth, const int& screenHeight);
		// デストラクタ
		~Camera() = default;

		// 行列の更新
		virtual void UpdateViewMatrix();
		virtual void UpdateProjectionMatrix();


	private:
		// 座標
		DirectX::SimpleMath::Vector3 m_pos;
		// 回転
		DirectX::SimpleMath::Quaternion m_rot;

		// 描画範囲（近）
		float m_nearPlane;
		// 描画範囲（遠）
		float m_farPlane;
		// 視野角（度）
		float m_fov;
		// 画面解像度
		int m_screenWidth;
		int m_screenHeight;


		// ビュー行列
		DirectX::SimpleMath::Matrix m_matView;
		// 射影行列
		DirectX::SimpleMath::Matrix m_matProj;
	};
}