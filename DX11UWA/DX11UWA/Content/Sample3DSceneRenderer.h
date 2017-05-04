﻿#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"

// My Header Files
#include "ObjLoader.h"
#include "Structures.h"

// Texture header file
#include "DDSTextureLoader.h"

namespace DX11UWA
{
	// This sample renderer instantiates a basic rendering pipeline.
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources(void);
		void CreateWindowSizeDependentResources(void);
		void ReleaseDeviceDependentResources(void);
		void Update(DX::StepTimer const& timer);
		void Render(int _camera_number);
		void StartTracking(void);
		void TrackingUpdate(float positionX);
		void StopTracking(void);
		inline bool IsTracking(void) { return m_tracking; }

		// Helper functions for keyboard and mouse input
		void SetKeyboardButtons(const char* list);
		void SetMousePosition(const Windows::UI::Input::PointerPoint^ pos);
		void SetInputDeviceData(const char* kb, const Windows::UI::Input::PointerPoint^ pos);


	private:
		void Rotate(float radians);
		void UpdateCamera(DX::StepTimer const& timer, float const moveSpd, float const rotSpd);
		void UpdateLights();

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Direct3D resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;

		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer	m_constantBufferData;
		uint32	m_indexCount;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;

		// Data members for keyboard and mouse input
		char	m_kbuttons[256];
		Windows::UI::Input::PointerPoint^ m_currMousePos;
		Windows::UI::Input::PointerPoint^ m_prevMousePos;

		Windows::UI::Input::PointerPoint^ m_currMousePos2;
		Windows::UI::Input::PointerPoint^ m_prevMousePos2;

		// Matrix data member for the camera
		DirectX::XMFLOAT4X4 m_camera;
		DirectX::XMFLOAT4X4 m_camera2;

		// Boolean to make camera 2 rotate on a axis
		bool camera2_auto_rotate = false;

		// Anything for the window
		DirectX::XMMATRIX perspectiveMatrix;
		float aspectRatio;
		float fovAngleY;
		float zFar;
		float zNear;

		////////////////////////////////////////////////////////////////
		//                  BEGIN SKYBOX MODEL STUFF                  //
		////////////////////////////////////////////////////////////////
		// Texture Variables
		Microsoft::WRL::ComPtr<ID3D11Resource> skybox_texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skybox_meshSRV;
		////////////////////////////////////////////////////////////////
		//                   END SKYBOX MODEL STUFF                   //
		////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////
		//                    BEGIN MC MODEL STUFF                    //
		////////////////////////////////////////////////////////////////
		// Master Chief
		Model master_chief_model;
		ModelViewProjectionConstantBuffer m_constantBufferData_master_chief;

		// Texture Variables
		Microsoft::WRL::ComPtr<ID3D11Resource> masterChief_texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> masterChief_meshSRV;
		////////////////////////////////////////////////////////////////
		//                     END MC MODEL STUFF                     //
		////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////
		//                 BEGIN ELEPHANT MODEL STUFF                 //
		////////////////////////////////////////////////////////////////
		// Master Chief
		Model elephant_model;
		ModelViewProjectionConstantBuffer m_constantBufferData_elephant;

		// Lights
		DirectionalLight floor_directional_light;
		PointLight floor_point_light;
		SpotLight floor_spot_light;

		// Light Constant Buffers & Data
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer_pointLight;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer_directionalLight;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer_spotLight;

		// Light Movement Variables
		float y_inc_dir;
		float x_inc_point;
		float x_inc_spot_pos;
		float z_inc_spot_pos;
		float x_inc_spot_dir;
		////////////////////////////////////////////////////////////////
		//                  END ELEPHANT MODEL STUFF                  //
		////////////////////////////////////////////////////////////////
	};
}