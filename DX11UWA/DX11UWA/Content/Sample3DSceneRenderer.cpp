#include "pch.h"
#include "Sample3DSceneRenderer.h"
#include "..\Common\DirectXHelper.h"

#include <DirectXColors.h>
using namespace DX11UWA;

using namespace DirectX;
using namespace Windows::Foundation;

static const XMVECTORF32 eye = { -15.0f, 30.0f, -0.0f, 0.0f };
static const XMVECTORF32 eye2 = { -15.0f, 45.0f, 0.0f, 0.0f };
static const XMVECTORF32 at = { 0.0f, 18.0f, 2.5f, 0.0f };
static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

ID3D11RasterizerState* WireFrame;
ID3D11RasterizerState* DefaultState;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	memset(m_kbuttons, 0, sizeof(m_kbuttons));
	m_currMousePos = nullptr;
	m_prevMousePos = nullptr;
	memset(&m_camera, 0, sizeof(XMFLOAT4X4));

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources(void)
{
	Size outputSize = m_deviceResources->GetOutputSize();
	aspectRatio = (outputSize.Width / 2.0f) / outputSize.Height;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, zNear, zFar);
			
	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(&m_constantBufferData.projection, perspectiveMatrix * orientationMatrix);
	XMStoreFloat4x4(&m_constantBufferData_master_chief.projection, perspectiveMatrix * orientationMatrix);
	XMStoreFloat4x4(&m_constantBufferData_elephant.projection, perspectiveMatrix * orientationMatrix);
	XMStoreFloat4x4(&m_constantBufferData_ghost.projection, perspectiveMatrix * orientationMatrix);

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	// When key press set the at to the object's position

	if (firstRun_camera)
	{
		XMStoreFloat4x4(&m_camera, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye, at, up)));
		XMStoreFloat4x4(&m_camera2, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye2, at, up)));

		firstRun_camera = false;
	}

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixLookAtLH(eye, at, up));
	XMStoreFloat4x4(&m_constantBufferData_master_chief.view, XMMatrixLookAtLH(eye, at, up));
	XMStoreFloat4x4(&m_constantBufferData_elephant.view, XMMatrixLookAtLH(eye, at, up));
	XMStoreFloat4x4(&m_constantBufferData_ghost.view, XMMatrixLookAtLH(eye, at, up));
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_tracking)
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

		Rotate(radians);
	}

	// Create groundWorl matrix
	XMMATRIX groundWorld = XMMatrixIdentity();

	// Define Terrain's world space matrix
	XMMATRIX Scale = XMMatrixScaling(10.0f, 10.0f, 10.0f);
	XMMATRIX Translation = XMMatrixTranslation(-100.0f, -100.0f, -100.0f);

	// Set terrain's world space using the transformations
	groundWorld = Scale * Translation;

	// Update or move camera here
	UpdateCamera(timer, 10.0f, 0.75f);

	// Update Lights
	static float y_inc_dir = -timer.GetElapsedSeconds();
	float directional_light_boundaries = 5.0f;

	if (elephant_directional_light.direction.y > directional_light_boundaries)
	{
		elephant_directional_light.direction.y = directional_light_boundaries;
		y_inc_dir *= -1.0f;
	}
	if (elephant_directional_light.direction.y < -directional_light_boundaries)
	{
		elephant_directional_light.direction.y = -directional_light_boundaries;
		y_inc_dir *= -1.0f;
	}

	elephant_directional_light.direction.y += y_inc_dir;

	static float x_inc_point = -timer.GetElapsedSeconds();
	float point_light_boundaries_positive = 20.0f;
	float point_light_boundaries_negative = -3.0f;

	if (elephant_point_light.position.y > point_light_boundaries_positive)
	{
		elephant_point_light.position.y = point_light_boundaries_positive;
		x_inc_point *= -1.0f;
	}
	if (elephant_point_light.position.y < point_light_boundaries_negative)
	{
		elephant_point_light.position.y = point_light_boundaries_negative;
		x_inc_point *= -1.0f;
	}

	elephant_point_light.position.y += x_inc_point;

	static float y_inc_spot_pos = timer.GetElapsedSeconds();
	static float y_inc_spot_dir = -timer.GetElapsedSeconds();
	float spot_light_boundaries_positive = 20.0f;
	float spot_light_boundaries_negatiave = -3.0f;

	if (elephant_spot_light.position.y > spot_light_boundaries_positive)
	{
		elephant_spot_light.position.y = spot_light_boundaries_positive;
		y_inc_spot_pos *= -1.0f;
	}
	if (elephant_spot_light.position.y < spot_light_boundaries_negatiave)
	{
		elephant_spot_light.position.y = spot_light_boundaries_negatiave;
		y_inc_spot_pos *= -1.0f;
	}

	if (elephant_spot_light.cone_direction.y > spot_light_boundaries_positive)
	{
		elephant_spot_light.cone_direction.y = spot_light_boundaries_positive;
		y_inc_spot_dir *= -1.0f;
	}
	if (elephant_spot_light.cone_direction.y < spot_light_boundaries_negatiave)
	{
		elephant_spot_light.cone_direction.y = spot_light_boundaries_negatiave;
		y_inc_spot_dir *= -1.0f;
	}

	elephant_spot_light.position.y += y_inc_spot_pos;
	elephant_spot_light.cone_direction.y += y_inc_spot_dir;

	// Call Update Lights Function
	UpdateLights();
}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Set the model of the ghost to make it orbit around the elephant
	XMStoreFloat4x4(&m_constantBufferData_ghost.model, (XMMatrixMultiply(XMMatrixTranslation(1.0f, 1.0f, 1.0f), XMMatrixRotationY(radians))));
}

void Sample3DSceneRenderer::UpdateCamera(DX::StepTimer const& timer, float const moveSpd, float const rotSpd)
{
	const float delta_time = (float)timer.GetElapsedSeconds();

	if (m_kbuttons['W'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['S'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, -moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['A'])
	{
		XMMATRIX translation = XMMatrixTranslation(-moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['D'])
	{
		XMMATRIX translation = XMMatrixTranslation(moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['X'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, -moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['R'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}

	if (m_currMousePos)
	{
		if (m_currMousePos->Properties->IsLeftButtonPressed && m_prevMousePos)
		{
			float dx = m_currMousePos->Position.X - m_prevMousePos->Position.X;
			float dy = m_currMousePos->Position.Y - m_prevMousePos->Position.Y;

			XMFLOAT4 pos = XMFLOAT4(m_camera._41, m_camera._42, m_camera._43, m_camera._44);

			m_camera._41 = 0;
			m_camera._42 = 0;
			m_camera._43 = 0;

			XMMATRIX rotX = XMMatrixRotationX(dy * rotSpd * delta_time);
			XMMATRIX rotY = XMMatrixRotationY(dx * rotSpd * delta_time);

			XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
			temp_camera = XMMatrixMultiply(rotX, temp_camera);
			temp_camera = XMMatrixMultiply(temp_camera, rotY);

			XMStoreFloat4x4(&m_camera, temp_camera);

			m_camera._41 = pos.x;
			m_camera._42 = pos.y;
			m_camera._43 = pos.z;
		}
		m_prevMousePos = m_currMousePos;
	}

	// Setup camera 2 buttons
	if (m_kbuttons['I'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera2);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera2, result);
	}
	if (m_kbuttons['K'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, -moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera2);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera2, result);
	}
	if (m_kbuttons['J'])
	{
		XMMATRIX translation = XMMatrixTranslation(-moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera2);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera2, result);
	}
	if (m_kbuttons['L'])
	{
		XMMATRIX translation = XMMatrixTranslation(moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera2);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera2, result);
	}
	if (m_kbuttons['M'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, -moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera2);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera2, result);
	}
	if (m_kbuttons['U'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera2);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera2, result);
	}

	if (m_currMousePos2)
	{
		if (m_currMousePos2->Properties->IsRightButtonPressed && m_prevMousePos2)
		{
			float dx = m_currMousePos2->Position.X - m_prevMousePos2->Position.X;
			float dy = m_currMousePos2->Position.Y - m_prevMousePos2->Position.Y;

			XMFLOAT4 pos = XMFLOAT4(m_camera2._41, m_camera2._42, m_camera2._43, m_camera2._44);

			m_camera2._41 = 0;
			m_camera2._42 = 0;
			m_camera2._43 = 0;

			XMMATRIX rotX = XMMatrixRotationX(dy * rotSpd * delta_time);
			XMMATRIX rotY = XMMatrixRotationY(dx * rotSpd * delta_time);

			XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera2);
			temp_camera = XMMatrixMultiply(rotX, temp_camera);
			temp_camera = XMMatrixMultiply(temp_camera, rotY);

			XMStoreFloat4x4(&m_camera2, temp_camera);

			m_camera2._41 = pos.x;
			m_camera2._42 = pos.y;
			m_camera2._43 = pos.z;
		}
		m_prevMousePos2 = m_currMousePos2;
	}

	// Use last row of the object.model to change the at
	if (m_kbuttons[VK_F2])
		camera2_auto_rotate != camera2_auto_rotate;

	if (camera2_auto_rotate)
	{
		// Create a XMVECTORF32 to change the at to
		XMVECTORF32 ghost_location = { m_constantBufferData_ghost.model._41, m_constantBufferData_ghost.model._42, m_constantBufferData_ghost.model._43, 0.0f };
	}

	// Buttons To Change Between WireFrame and DefaultState 
	if (m_kbuttons[VK_F3])
	{
		auto context = m_deviceResources->GetD3DDeviceContext();
		context->RSSetState(DefaultState);
	}

	if (m_kbuttons[VK_F4])
	{
		auto context = m_deviceResources->GetD3DDeviceContext();
		context->RSSetState(WireFrame);
	}

	// Setup key presses to adjust Far and Near plane clipping
	// Have [] control the far plane, and <> control the near plane
	// perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, zFar, zNear);
	if (m_kbuttons[VK_OEM_4])
	{
		zFar += zFar_increment * moveSpd;

		UpdatePlanes();
	}
	if (m_kbuttons[VK_OEM_6])
	{
		zFar -= zFar_increment * moveSpd;

		if (zFar <= zNear)
			zFar = zNear + 0.1f;

		UpdatePlanes();
	}
	if (m_kbuttons[VK_OEM_PERIOD])
	{
		zNear -= zNear_incremenet * moveSpd;

		if (zNear <= 0.01f)
			zNear = 0.01f;

		UpdatePlanes();
	}
	if (m_kbuttons[VK_OEM_COMMA])
	{
		zNear += zNear_incremenet * moveSpd;

		UpdatePlanes();
	}

	// Setup the Mouse wheel to do zooms (or arrow keys)
	// Positive (Mouse Wheel up)

	if (m_kbuttons[VK_UP] && fovAngleY != small_zoom_clamp)
	{
		fovAngleY -= fov_increment;

		if (fovAngleY <= 0.0f)
			fovAngleY = small_zoom_clamp;

		UpdatePlanes();
	}

	if (m_kbuttons[VK_DOWN] && fovAngleY != large_zoom_clamp)
	{
		fovAngleY += fov_increment;

		if (fovAngleY >= large_zoom_clamp)
			fovAngleY = large_zoom_clamp;

		UpdatePlanes();
	}

#pragma region Color Changing

	// Setup key presses to change directional light color
	DirectX::XMFLOAT4 lightColor_Directional, lightColor_Point, lightColor_Spot;

	// Directional Light Values
	static float a = 0.0f;
	static float r = 255.0f;
	static float g = 197.0f;
	static float b = 143.0f;

	// Point Light Values
	static float a2 = 0.0f;
	static float r2 = 201.0f;
	static float g2 = 226.0f;
	static float b2 = 255.0f;

	// Spot Light Values
	static float a3 = 0.0f;
	static float r3 = 167.0f;
	static float g3 = 0.0f;
	static float b3 = 255.0f;

	// To change Directional Light
	if (m_kbuttons[VK_NUMPAD1])
	{
		// Switch the light to Candle Light
		a = 0.0f;
		r = 255.0f;
		g = 147.0f;
		b = 41.0f;
	}
	if (m_kbuttons[VK_NUMPAD2])
	{
		// Switch the light to Carbon Arc Light
		a = 0.0f;
		r = 255.0f;
		g = 250.0f;
		b = 244.0f;
	}
	if (m_kbuttons[VK_NUMPAD3])
	{
		// Switch the light to Mercury Vapor Light
		a = 0.0f;
		r = 216.0f;
		g = 247.0f;
		b = 255.0f;
	}

	// To change Point Light
	if (m_kbuttons[VK_NUMPAD4])
	{
		// Switch the light to Sodium Vapor Light
		a2 = 0.0f;
		r2 = 255.0f;
		g2 = 209.0f;
		b2 = 178.0f;
	}
	if (m_kbuttons[VK_NUMPAD5])
	{
		// Switch the light to Warm Fluorescent Light
		a2 = 0.0f;
		r2 = 255.0f;
		g2 = 244.0f;
		b2 = 229.0f;
	}
	if (m_kbuttons[VK_NUMPAD6])
	{
		// Switch the light to 100W Tungsten Light
		a2 = 0.0f;
		r2 = 64.0f;
		g2 = 156.0f;
		b2 = 255.0f;
	}

	// To change Spot Light
	if (m_kbuttons[VK_NUMPAD7])
	{
		// Switch the light to Clear Blue Sky Light
		a3 = 0.0f;
		r3 = 64.0f;
		g3 = 156.0f;
		b3 = 255.0f;
	}
	if (m_kbuttons[VK_NUMPAD8])
	{
		// Switch the light to Cool White Flurescent Light
		a3 = 0.0f;
		r3 = 212.0f;
		g3 = 235.0f;
		b3 = 255.0f;
	}
	if (m_kbuttons[VK_NUMPAD9])
	{
		// Switch the light to High Pressure Sodium Light
		a3 = 0.0f;
		r3 = 255.0f;
		g3 = 183.0f;
		b3 = 76.0f;
	}

	if (m_kbuttons[VK_F1])
	{
		a = 0.0f;
		r = 255.0f;
		g = 197.0f;
		b = 143.0f;

		a2 = 0.0f;
		r2 = 201.0f;
		g2 = 226.0f;
		b2 = 255.0f;

		a3 = 0.0f;
		r3 = 167.0f;
		g3 = 0.0f;
		b3 = 255.0f;
	}

	// Calculate New Colors
	lightColor_Directional = { (r / 255.0f), (g / 255.0f), (b / 255.0f), a };
	lightColor_Point = { (r2 / 255.0f), (g2 / 255.0f), (b2 / 255.0f), a2 };
	lightColor_Spot = { (r3 / 255.0f), (g3 / 255.0f), (b3 / 255.0f), a3 };

	// Set the new color to the lights
	elephant_directional_light.color = lightColor_Directional;
	elephant_point_light.color = lightColor_Point;
	elephant_spot_light.color = lightColor_Spot;

	// Update lights
	UpdateLights();

#pragma endregion


}

void Sample3DSceneRenderer::SetKeyboardButtons(const char* list)
{
	memcpy_s(m_kbuttons, sizeof(m_kbuttons), list, sizeof(m_kbuttons));
}

void Sample3DSceneRenderer::SetMousePosition(const Windows::UI::Input::PointerPoint^ pos)
{
	m_currMousePos = const_cast<Windows::UI::Input::PointerPoint^>(pos);
	m_currMousePos2 = const_cast<Windows::UI::Input::PointerPoint^>(pos);
}

void Sample3DSceneRenderer::SetInputDeviceData(const char* kb, const Windows::UI::Input::PointerPoint^ pos)
{
	SetKeyboardButtons(kb);
	SetMousePosition(pos);
}

void DX11UWA::Sample3DSceneRenderer::StartTracking(void)
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void Sample3DSceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}
}

void Sample3DSceneRenderer::StopTracking(void)
{
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void Sample3DSceneRenderer::Render(int _camera_number)
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	DirectX::XMFLOAT4X4 _camera_to_use;

	if (_camera_number == 1)
		_camera_to_use = m_camera;

	else if (_camera_number == 2)
		_camera_to_use = m_camera2;

#pragma region Skybox

	ID3D11ShaderResourceView** skyboxViews[] = { skybox_meshSRV.GetAddressOf() };
	context->PSSetShaderResources(0, 1, *skyboxViews);

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixInverse(nullptr, XMLoadFloat4x4(&_camera_to_use)));

	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_inputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	// Draw the objects.
	context->DrawIndexed(m_indexCount, 0, 0);

#pragma endregion

#pragma region Master Chief

	if (!master_chief_model._loadingComplete)
	{
		return;
	}

	ID3D11ShaderResourceView* texViews[] = { masterChief_meshSRV, masterChief_meshSRV2 };
	context->PSSetShaderResources(0, 2, texViews);

	XMStoreFloat4x4(&m_constantBufferData_master_chief.view, (XMMatrixInverse(nullptr, XMLoadFloat4x4(&_camera_to_use))));

	// Setup Vertex Buffer
	UINT masterChief_stride = sizeof(DX11UWA::VertexPositionUVNormal);
	UINT masterChief_offset = 0;
	context->IASetVertexBuffers(0, 1, master_chief_model._vertexBuffer.GetAddressOf(), &masterChief_stride, &masterChief_offset);

	// Set Index buffer
	context->IASetIndexBuffer(master_chief_model._indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetInputLayout(master_chief_model._inputLayout.Get());

	context->UpdateSubresource1(master_chief_model._constantBuffer.Get(), 0, NULL, &m_constantBufferData_master_chief, 0, 0, 0);

	// Attach our vertex shader.
	context->VSSetShader(master_chief_model._vertexShader.Get(), nullptr, 0);

	// Attach our pixel shader.
	context->PSSetShader(master_chief_model._pixelShader.Get(), nullptr, 0);

	context->DrawIndexed(master_chief_model._indexCount, 0, 0);

#pragma endregion

#pragma region Elephant

	if (!elephant_model._loadingComplete)
	{
		return;
	}

	XMStoreFloat4x4(&m_constantBufferData_elephant.view, (XMMatrixInverse(nullptr, XMLoadFloat4x4(&_camera_to_use))));

	// Setup Vertex Buffer
	UINT elephant_stride = sizeof(DX11UWA::VertexPositionUVNormal);
	UINT elephant_offset = 0;
	context->IASetVertexBuffers(0, 1, elephant_model._vertexBuffer.GetAddressOf(), &elephant_stride, &elephant_offset);

	// Set Index buffer
	context->IASetIndexBuffer(elephant_model._indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetInputLayout(elephant_model._inputLayout.Get());

	context->UpdateSubresource1(elephant_model._constantBuffer.Get(), 0, NULL, &m_constantBufferData_elephant, 0, 0, 0);

	// Update subresources for the lights
	context->UpdateSubresource1(m_constantBuffer_directionalLight.Get(), 0, NULL, &elephant_directional_light, 0, 0, 0);
	context->UpdateSubresource1(m_constantBuffer_pointLight.Get(), 0, NULL, &elephant_point_light, 0, 0, 0);
	context->UpdateSubresource1(m_constantBuffer_spotLight.Get(), 0, NULL, &elephant_spot_light, 0, 0, 0);

	//Set the light constant buffers to the floor
	context->PSSetConstantBuffers1(0, 1, m_constantBuffer_directionalLight.GetAddressOf(), nullptr, nullptr);
	context->PSSetConstantBuffers1(1, 1, m_constantBuffer_pointLight.GetAddressOf(), nullptr, nullptr);
	context->PSSetConstantBuffers1(2, 1, m_constantBuffer_spotLight.GetAddressOf(), nullptr, nullptr);

	// Attach our vertex shader.
	context->VSSetShader(elephant_model._vertexShader.Get(), nullptr, 0);

	// Attach our pixel shader.
	context->PSSetShader(elephant_model._pixelShader.Get(), nullptr, 0);

	context->DrawIndexed(elephant_model._indexCount, 0, 0);

#pragma endregion

#pragma region 343 Guilty Spark

	if (!ghost_model._loadingComplete)
	{
		return;
	}

	ID3D11ShaderResourceView* guiltySpark_texViews[] = { ghost_meshSRV };
	context->PSSetShaderResources(0, 1, guiltySpark_texViews);

	XMStoreFloat4x4(&m_constantBufferData_ghost.view, (XMMatrixInverse(nullptr, XMLoadFloat4x4(&_camera_to_use))));

	// Setup Vertex Buffer
	UINT guiltySpark_stride = sizeof(DX11UWA::VertexPositionUVNormal);
	UINT guiltySpark_offset = 0;
	context->IASetVertexBuffers(0, 1, ghost_model._vertexBuffer.GetAddressOf(), &guiltySpark_stride, &guiltySpark_offset);

	// Set Index buffer
	context->IASetIndexBuffer(ghost_model._indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetInputLayout(ghost_model._inputLayout.Get());

	context->UpdateSubresource1(ghost_model._constantBuffer.Get(), 0, NULL, &m_constantBufferData_ghost, 0, 0, 0);

	// Attach our vertex shader.
	context->VSSetShader(ghost_model._vertexShader.Get(), nullptr, 0);

	// Attach our pixel shader.
	context->PSSetShader(ghost_model._pixelShader.Get(), nullptr, 0);

	context->DrawIndexed(ghost_model._indexCount, 0, 0);

#pragma endregion

#pragma region Height Map

	context->DrawIndexed(NumFaces * 3, 0, 0);

#pragma endregion


}

void Sample3DSceneRenderer::CreateDeviceDependentResources(void)
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");
	auto loadVSTaskSkybox = DX::ReadDataAsync(L"SkyboxVertexShader.cso");
	auto loadPSTaskSkybox = DX::ReadDataAsync(L"SkyboxPixelShader.cso");
	auto loadVSTaskTexture = DX::ReadDataAsync(L"TextureVertexShader.cso");
	auto loadPSTaskTexture = DX::ReadDataAsync(L"TexturePixelShader.cso");
	auto loadVSTaskHeightmap = DX::ReadDataAsync(L"HeightMapVertexShader.cso");
	auto loadPSTaskHeightmap = DX::ReadDataAsync(L"HeightMapPixelShader.cso");
	auto loadVSTaskSingleTexture = DX::ReadDataAsync(L"SingleTextureVertexShader.cso");
	auto loadPSTaskSingleTexture = DX::ReadDataAsync(L"SingleTexturePixelShader.cso");

#pragma region Skybox

	auto context_skybox = m_deviceResources->GetD3DDeviceContext();
	ID3D11Device *device_skybox;
	context_skybox->GetDevice(&device_skybox);

	const char *skybox_path = "Assets/Textures/Halo_Reach_Skybox.dds";

	size_t skybox_pathSize = strlen(skybox_path) + 1;
	wchar_t *skybox_wc = new wchar_t[skybox_pathSize];
	mbstowcs(&skybox_wc[0], skybox_path, skybox_pathSize);

	HRESULT hr2;
	hr2 = CreateDDSTextureFromFile(device_skybox, skybox_wc, &skybox_texture, &skybox_meshSRV);

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTaskSkybox.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_inputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTaskSkybox.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_pixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer));
	});

	// Once both shaders are loaded, create the mesh.
	auto createCubeTask = (createPSTask && createVSTask).then([this]()
	{
		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionColor cubeVertices[] =
		{
			{ XMFLOAT3(-150.0f, -150.0f, -150.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-150.0f, -150.0f,  150.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-150.0f,  150.0f, -150.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-150.0f,  150.0f,  150.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(150.0f, -150.0f, -150.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(150.0f, -150.0f,  150.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(150.0f,  150.0f, -150.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(150.0f,  150.0f,  150.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer));

		// Load mesh indices. Each trio of indices represents
		// a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes
		// 0, 2 and 1 from the vertex buffer compose the 
		// first triangle of this mesh.
		static const unsigned short cubeIndices[] =
		{
			2,1,0, // -x
			2,3,1,

			5,6,4, // +x
			7,6,5,

			1,5,0, // -y
			5,4,0,

			6,7,2, // +y
			7,3,2,

			4,6,0, // -z
			6,2,0,

			3,7,1, // +z
			7,5,1,
		};

		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer));
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createCubeTask.then([this]()
	{
		m_loadingComplete = true;
	});

#pragma endregion

#pragma region Master Chief

	auto context = m_deviceResources->GetD3DDeviceContext();
	ID3D11Device *device;
	context->GetDevice(&device);

	const char *path = "Assets/Textures/masterchief2.dds";
	const char *path2 = "Assets/Textures/masterchief_color.dds";


	size_t pathSize = strlen(path) + 1;
	wchar_t *wc = new wchar_t[pathSize];
	mbstowcs(&wc[0], path, pathSize);

	size_t pathSize2 = strlen(path2) + 1;
	wchar_t *wc2 = new wchar_t[pathSize2];
	mbstowcs(&wc2[0], path2, pathSize2);
	
	HRESULT hr;
	hr = CreateDDSTextureFromFile(device, wc, &masterChief_texture, &masterChief_meshSRV);
	hr = CreateDDSTextureFromFile(device, wc2, &masterChief_texture2, &masterChief_meshSRV2);

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask_Master_Chief_Model = loadVSTaskTexture.then([this](const std::vector<byte>& masterChief_fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&masterChief_fileData[0], masterChief_fileData.size(), nullptr, &master_chief_model._vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC master_chief_vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORM", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(master_chief_vertexDesc, ARRAYSIZE(master_chief_vertexDesc), &masterChief_fileData[0], masterChief_fileData.size(), &master_chief_model._inputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask_Master_Chief_Model = loadPSTaskTexture.then([this](const std::vector<byte>& masterChief_fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&masterChief_fileData[0], masterChief_fileData.size(), nullptr, &master_chief_model._pixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &master_chief_model._constantBuffer));
	});

	// Once both shaders are loaded, create the mesh.
	auto createTask_Master_Chief = (createPSTask_Master_Chief_Model && createVSTask_Master_Chief_Model).then([this]()
	{
		std::vector<DX11UWA::VertexPositionUVNormal> masterChief_vertices;
		std::vector<DirectX::XMFLOAT3> masterChief_normals;
		std::vector<DirectX::XMFLOAT2> masterChief_uvs;
		std::vector<unsigned int> masterChief_indices;

		loadOBJ("Assets/Models/H2_MC.obj", masterChief_vertices, masterChief_indices, masterChief_normals, masterChief_uvs);

		// Scale down the model
		for (unsigned int i = 0; i < masterChief_vertices.size(); i++)
		{
			VertexPositionUVNormal temp = masterChief_vertices[i];
			temp.pos.x *= .15f;
			temp.pos.y *= .15f;
			temp.pos.z *= .15f;

			// Move the Master Chief Model ontop of the elephant
			temp.pos.z += 2.5f;
			temp.pos.y += 13.5f;
			masterChief_vertices[i] = temp;
		}

		D3D11_SUBRESOURCE_DATA masterChief_vertexBufferData = { 0 };
		masterChief_vertexBufferData.pSysMem = masterChief_vertices.data();
		masterChief_vertexBufferData.SysMemPitch = 0;
		masterChief_vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC masterChief_vertexBufferDesc(sizeof(DX11UWA::VertexPositionUVNormal) * masterChief_vertices.size(), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&masterChief_vertexBufferDesc, &masterChief_vertexBufferData, &master_chief_model._vertexBuffer));

		master_chief_model._indexCount = masterChief_indices.size();

		D3D11_SUBRESOURCE_DATA masterChief_indexBufferData = { 0 };
		masterChief_indexBufferData.pSysMem = masterChief_indices.data();
		masterChief_indexBufferData.SysMemPitch = 0;
		masterChief_indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC masterChief_indexBufferDesc(sizeof(unsigned int) * masterChief_indices.size(), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&masterChief_indexBufferDesc, &masterChief_indexBufferData, &master_chief_model._indexBuffer));
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createTask_Master_Chief.then([this]()
	{
		master_chief_model._loadingComplete = true;
	});

#pragma endregion

#pragma region Halo Elephant

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask_Elephant_Model = loadVSTask.then([this](const std::vector<byte>& elephant_fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&elephant_fileData[0], elephant_fileData.size(), nullptr, &elephant_model._vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC elephant_vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORM", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(elephant_vertexDesc, ARRAYSIZE(elephant_vertexDesc), &elephant_fileData[0], elephant_fileData.size(), &elephant_model._inputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask_Elephant_Model = loadPSTask.then([this](const std::vector<byte>& elephant_fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&elephant_fileData[0], elephant_fileData.size(), nullptr, &elephant_model._pixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &elephant_model._constantBuffer));
	
		// Create the constant buffers for the lights
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer_directionalLight));
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer_pointLight));
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer_spotLight));
	});

	// Once both shaders are loaded, create the mesh.
	auto createTask_Elephant = (createPSTask_Elephant_Model && createVSTask_Elephant_Model).then([this]()
	{
		std::vector<DX11UWA::VertexPositionUVNormal> elephant_vertices;
		std::vector<DirectX::XMFLOAT3> elephant_normals;
		std::vector<DirectX::XMFLOAT2> elephant_uvs;
		std::vector<unsigned int> elephant_indices;

		loadOBJ("Assets/Models/H3_Elephant.obj", elephant_vertices, elephant_indices, elephant_normals, elephant_uvs);

		// Scale down the model
		for (unsigned int i = 0; i < elephant_vertices.size(); i++)
		{
			VertexPositionUVNormal temp = elephant_vertices[i];

			// scale down the elephant model by 50%
			temp.pos.x *= .50f;
			temp.pos.y *= .50f;
			temp.pos.z *= .50f;
			
			// Set uv's to 0.0f so it displays black
			temp.uv.x = 1.0f;
			temp.uv.y = 1.0f;
			temp.uv.z = 1.0f;

			elephant_vertices[i] = temp;
		}

		D3D11_SUBRESOURCE_DATA elephant_vertexBufferData = { 0 };
		elephant_vertexBufferData.pSysMem = elephant_vertices.data();
		elephant_vertexBufferData.SysMemPitch = 0;
		elephant_vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC elephant_vertexBufferDesc(sizeof(DX11UWA::VertexPositionUVNormal) * elephant_vertices.size(), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&elephant_vertexBufferDesc, &elephant_vertexBufferData, &elephant_model._vertexBuffer));

		elephant_model._indexCount = elephant_indices.size();

		D3D11_SUBRESOURCE_DATA elephant_indexBufferData = { 0 };
		elephant_indexBufferData.pSysMem = elephant_indices.data();
		elephant_indexBufferData.SysMemPitch = 0;
		elephant_indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC elephant_indexBufferDesc(sizeof(unsigned int) * elephant_indices.size(), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&elephant_indexBufferDesc, &elephant_indexBufferData, &elephant_model._indexBuffer));
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createTask_Elephant.then([this]()
	{
		elephant_model._loadingComplete = true;
	});

#pragma endregion

#pragma region Light Initialization

	// Initialize the directional light data
	elephant_directional_light.direction = { 0.0f, 2.0f, 1.0f, 0.0f };
	elephant_directional_light.color = { (255.0f / 255.0f) , (197.0f / 255.0f), (143.0f / 255.0f), 0.0f };

	// Initialize the point light data
	elephant_point_light.position = { 3.0f, 15.0f, 0.0f, 0.0f };
	elephant_point_light.color = { (201.0f / 255.0f), (226.0f / 255.0f), (255.0f / 255.0f), 0.0f };
	elephant_point_light.radius.x = 20.0f;

	// Initialize the spot light data
	elephant_spot_light.position = { 3.0f, 19.0f, -10.0f, 0.0f };
	elephant_spot_light.color = { (167.0f / 255.0f), (0.0f / 255.0f), (255.0f / 255.0f), 0.0f };
	elephant_spot_light.cone_direction = { 0.0f, 13.5f, 0.0f, 0.0f };	// Subtract From masterchief postion
	elephant_spot_light.cone_ratio.x = 0.5f;
	elephant_spot_light.inner_cone_ratio.x = 0.96f;
	elephant_spot_light.outer_cone_ratio.x = 0.95f;

#pragma endregion

#pragma region Ghost

	{
		const char *path = "Assets/Textures/ghost.dds";

		size_t pathSize = strlen(path) + 1;
		wchar_t *wc = new wchar_t[pathSize];
		mbstowcs(&wc[0], path, pathSize);

		hr = CreateDDSTextureFromFile(device, wc, &ghost_texture, &ghost_meshSRV);
	}

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask_ghost = loadVSTaskSingleTexture.then([this](const std::vector<byte>& ghost_fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&ghost_fileData[0], ghost_fileData.size(), nullptr, &ghost_model._vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC ghost_vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORM", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(ghost_vertexDesc, ARRAYSIZE(ghost_vertexDesc), &ghost_fileData[0], ghost_fileData.size(), &ghost_model._inputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask_ghost = loadPSTaskSingleTexture.then([this](const std::vector<byte>& ghost_fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&ghost_fileData[0], ghost_fileData.size(), nullptr, &ghost_model._pixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &ghost_model._constantBuffer));
	});

	// Once both shaders are loaded, create the mesh.
	auto createTask_Ghost = (createPSTask_ghost && createVSTask_ghost).then([this]()
	{
		std::vector<DX11UWA::VertexPositionUVNormal> ghost_vertices;
		std::vector<DirectX::XMFLOAT3> ghost_normals;
		std::vector<DirectX::XMFLOAT2> ghost_uvs;
		std::vector<unsigned int> ghost_indices;

		loadOBJ("Assets/Models/HR_Ghost.obj", ghost_vertices, ghost_indices, ghost_normals, ghost_uvs);

		// Scale down the model
		for (unsigned int i = 0; i < ghost_vertices.size(); i++)
		{
			VertexPositionUVNormal temp = ghost_vertices[i];
			temp.pos.x *= .05f;
			temp.pos.y *= .05f;
			temp.pos.z *= .05f;

			// Move the Ghost Model Somewhere above the elephant for it to orbit
			temp.pos.z += 50.0f;
			temp.pos.y += 25.0f;
			ghost_vertices[i] = temp;
		}

		D3D11_SUBRESOURCE_DATA ghost_vertexBufferData = { 0 };
		ghost_vertexBufferData.pSysMem = ghost_vertices.data();
		ghost_vertexBufferData.SysMemPitch = 0;
		ghost_vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC ghost_vertexBufferDesc(sizeof(DX11UWA::VertexPositionUVNormal) * ghost_vertices.size(), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&ghost_vertexBufferDesc, &ghost_vertexBufferData, &ghost_model._vertexBuffer));

		ghost_model._indexCount = ghost_indices.size();

		D3D11_SUBRESOURCE_DATA ghost_indexBufferData = { 0 };
		ghost_indexBufferData.pSysMem = ghost_indices.data();
		ghost_indexBufferData.SysMemPitch = 0;
		ghost_indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC ghost_indexBufferDesc(sizeof(unsigned int) * ghost_indices.size(), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&ghost_indexBufferDesc, &ghost_indexBufferData, &ghost_model._indexBuffer));
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createTask_Ghost.then([this]()
	{
		ghost_model._loadingComplete = true;
	});

#pragma endregion

#pragma region Initialize RasterizerStates

	// Setup For WireFrame
	{
		D3D11_RASTERIZER_DESC wfdesc;
		ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
		wfdesc.FillMode = D3D11_FILL_WIREFRAME;
		wfdesc.CullMode = D3D11_CULL_NONE;

		auto context = m_deviceResources->GetD3DDeviceContext();
		ID3D11Device* device;
		context->GetDevice(&device);
		hr = device->CreateRasterizerState(&wfdesc, &WireFrame);

	}

	// Setup For DefaultState
	{
		D3D11_RASTERIZER_DESC dsdesc;
		ZeroMemory(&dsdesc, sizeof(D3D11_RASTERIZER_DESC));
		dsdesc.FillMode = D3D11_FILL_SOLID;
		dsdesc.CullMode = D3D11_CULL_NONE;

		auto context = m_deviceResources->GetD3DDeviceContext();
		ID3D11Device* device;
		context->GetDevice(&device);
		hr = device->CreateRasterizerState(&dsdesc, &DefaultState);

	}

#pragma endregion

#pragma region Height Map Initialization

	//HeightMapLoad("Assets/Heightmaps/HMCSHeightmap.bmp", hmInfo);

	//int cols = hmInfo.terrainWidth;
	//int rows = hmInfo.terrainHeight;

	//// Create the grid
	//NumVertices = rows * cols;
	//NumFaces = (rows - 1) * (cols - 1) * 2;

	//vector<VertexPositionUVNormal> v(NumVertices);
	// 
	//for (DWORD i = 0; i < rows; i++)
	//{
	//	for (DWORD j = 0; j < cols; j++)
	//	{
	//		v[i*cols + j].pos = hmInfo.heightMap[i*cols + j];
	//		v[i*cols + j].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	//	}
	//}

	//vector<DWORD> indices(NumFaces * 3);
	//int k = 0;
	//int texUIndex = 0;
	//int texVIndex = 0;

	//for (DWORD i = 0; i < rows-1; i++)
	//{
	//	for (DWORD j = 0; j < cols-1; j++)
	//	{
	//		indices[k] = i * cols + j;				// Bottom Left of quad
	//		v[i*cols + j].uv = XMFLOAT3(texUIndex + 0.0f, texVIndex + 1.0f, 0.0f);

	//		indices[k + 1] = i * cols + j + 1;		// Bottom right of quad
	//		v[i * cols + j + 1].uv = XMFLOAT3(texUIndex + 1.0f, texVIndex + 1.0f, 0.0f);

	//		indices[k + 2] = (i + 1) * cols + j;	// Top left of quad
	//		v[(i + 1) * cols + j].uv = XMFLOAT3(texUIndex + 0.0f, texVIndex + 0.0f, 0.0f);


	//		indices[k + 3] = (i + 1) * cols + j;	// Top left of quad
	//		v[(i + 1) * cols + j].uv = XMFLOAT3(texUIndex + 0.0f, texVIndex + 0.0f, 0.0f);

	//		indices[k + 4] = i * cols + j + 1;		// Bottom right of quad
	//		v[i * cols + j + 1].uv = XMFLOAT3(texUIndex + 1.0f, texVIndex + 1.0f, 0.0f);

	//		k += 6;	// Next quad
	//	}

	//	texUIndex = 0;
	//	texVIndex++;
	//}

	////////////////////////Compute Normals///////////////////////////
	//// Now we will compute the normals for each vertex using normal averaging
	//vector<XMFLOAT3> tempNormal;

	//// Normalized and unnormalized normals
	//XMFLOAT3 unnormalized = XMFLOAT3(0.0f, 0.0f, 0.0f);

	//// Used to get vectors (sides) from the position of the verts
	//float vecX, vecY, vecZ;

	//// Two edges of our triangle
	//XMVECTOR edge1 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//XMVECTOR edge2 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	//// Compute face normals
	//for (int i = 0; i < NumFaces; ++i)
	//{
	//	// Get the vector describing one edge of our triangle (edge 0, 2)
	//	vecX = v[indices[(i * 3)]].pos.x - v[indices[(i * 3) + 2]].pos.x;
	//	vecY = v[indices[(i * 3)]].pos.y - v[indices[(i * 3) + 2]].pos.y;
	//	vecZ = v[indices[(i * 3)]].pos.z - v[indices[(i * 3) + 2]].pos.z;
	//	edge1 = XMVectorSet(vecX, vecY, vecZ, 0.0f);		// Create First Edge

	//	// Get the vector describing another edge of our triangle (edge 2, 1)
	//	vecX = v[indices[(i * 3) + 2]].pos.x - v[indices[(i * 3) + 1]].pos.x;
	//	vecY = v[indices[(i * 3) + 2]].pos.y - v[indices[(i * 3) + 1]].pos.y;
	//	vecZ = v[indices[(i * 3) + 2]].pos.z - v[indices[(i * 3) + 1]].pos.z;

	//	// Cross multiply the two edge vectors to get the un-normalized face normal
	//	XMStoreFloat3(&unnormalized, XMVector3Cross(edge1, edge2));
	//	tempNormal.push_back(unnormalized);		// Save unormalized normal (for normal averaging)
	//}

	//// Compute vertex normals (normal Averaging)
	//XMVECTOR normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//int facesUsing = 0;
	//float tX;
	//float tY;
	//float tZ;

	//// Go nthrough each vertex
	//for (int i = 0; i < NumVertices; ++i)
	//{
	//	// Check which triangles use this vertex
	//	for (int j = 0; j < NumFaces; j++)
	//	{
	//		if (indices[j*3] == i ||
	//			indices[(j*3)+1] == i ||
	//			indices[(j*3)+2] == i)
	//		{
	//			tX = XMVectorGetX(normalSum) + tempNormal[j].x;
	//			tY = XMVectorGetY(normalSum) + tempNormal[j].y;
	//			tZ = XMVectorGetZ(normalSum) + tempNormal[j].z;

	//			normalSum = XMVectorSet(tX, tY, tZ, 0.0f);		// IEF a face is using the vertex, add the unormalized face normal to the normalSum
	//			facesUsing++;
	//		}
	//	}

	//	// Get the actual normal by dividing the normalSum by the number of faces sharing the vertex
	//	normalSum = normalSum / facesUsing;

	//	// Normalize the normalSum vector
	//	normalSum = XMVector3Normalize(normalSum);

	//	// Store the normal in our current vertex
	//	v[i].normal.x = XMVectorGetX(normalSum);
	//	v[i].normal.y = XMVectorGetY(normalSum);
	//	v[i].normal.z = XMVectorGetZ(normalSum);

	//	// Clear normalSum and faceUsing for next vertex
	//	normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//	facesUsing = 0;
	//}

	//// Creating The Vertex and Index Buffers
	//D3D11_BUFFER_DESC hm_indexbufferDesc;
	//ZeroMemory(&hm_indexbufferDesc, sizeof(hm_indexbufferDesc));

	//hm_indexbufferDesc.Usage = D3D11_USAGE_DEFAULT;
	///************************************New Stuff****************************************************/
	//hm_indexbufferDesc.ByteWidth = sizeof(DWORD) * NumFaces * 3;
	///*************************************************************************************************/
	//hm_indexbufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	//hm_indexbufferDesc.CPUAccessFlags = 0;
	//hm_indexbufferDesc.MiscFlags = 0;

	//D3D11_SUBRESOURCE_DATA iinitData;
	///************************************New Stuff****************************************************/
	//iinitData.pSysMem = &indices[0];
	///*************************************************************************************************/
	//device->CreateBuffer(&hm_indexbufferDesc, &iinitData, &hm_constantBuffer);

	//D3D11_BUFFER_DESC vertexBufferDesc;
	//ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	//vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	///************************************New Stuff****************************************************/
	//vertexBufferDesc.ByteWidth = sizeof(VertexPositionUVNormal) * NumVertices;
	///*************************************************************************************************/
	//vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//vertexBufferDesc.CPUAccessFlags = 0;
	//vertexBufferDesc.MiscFlags = 0;

	//D3D11_SUBRESOURCE_DATA vertexBufferData;

	//ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	///************************************New Stuff****************************************************/
	//vertexBufferData.pSysMem = &v[0];
	///*************************************************************************************************/
	//hr = device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &hm_vertexBuffer);

#pragma endregion

#pragma region Grid Initialization

#pragma endregion

#pragma region World Matrix Initialization

	// Prepare to pass the updated model matrix to the shader
	XMMATRIX identity = XMMatrixIdentity();
	XMStoreFloat4x4(&m_constantBufferData.model, identity);

	// Set The model of the master chief model to the identity matrix
	XMStoreFloat4x4(&m_constantBufferData_master_chief.model, identity);

	// Set The model of the elephant model to the identity matrix
	XMStoreFloat4x4(&m_constantBufferData_elephant.model, identity);

	// Set the model of the ghost model to the identity matrix
	XMStoreFloat4x4(&m_constantBufferData_ghost.model, identity);

#pragma endregion

}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources(void)
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}

void Sample3DSceneRenderer::UpdateLights()
{
	// Get the context so I can update the lights
	auto context = m_deviceResources->GetD3DDeviceContext();

	// Update subresources for the lights
	context->UpdateSubresource1(m_constantBuffer_directionalLight.Get(), 0, NULL, &elephant_directional_light, 0, 0, 0);
	context->UpdateSubresource1(m_constantBuffer_pointLight.Get(), 0, NULL, &elephant_point_light, 0, 0, 0);
	context->UpdateSubresource1(m_constantBuffer_spotLight.Get(), 0, NULL, &elephant_spot_light, 0, 0, 0);


	// Set the light constant buffers to the floor
	context->PSSetConstantBuffers1(0, 1, m_constantBuffer_directionalLight.GetAddressOf(), nullptr, nullptr);
	context->PSSetConstantBuffers1(1, 1, m_constantBuffer_pointLight.GetAddressOf(), nullptr, nullptr);
	context->PSSetConstantBuffers1(2, 1, m_constantBuffer_spotLight.GetAddressOf(), nullptr, nullptr);
}

void Sample3DSceneRenderer::UpdatePlanes()
{
	// This sample makes use of a right-handed coordinate system using row-major matrices.
	perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, zNear, zFar);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(&m_constantBufferData.projection, perspectiveMatrix * orientationMatrix);
	XMStoreFloat4x4(&m_constantBufferData_master_chief.projection, perspectiveMatrix * orientationMatrix);
	XMStoreFloat4x4(&m_constantBufferData_elephant.projection, perspectiveMatrix * orientationMatrix);
	XMStoreFloat4x4(&m_constantBufferData_ghost.projection, perspectiveMatrix * orientationMatrix);


	auto context = m_deviceResources->GetD3DDeviceContext();
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
	context->UpdateSubresource1(master_chief_model._constantBuffer.Get(), 0, NULL, &m_constantBufferData_master_chief, 0, 0, 0);
	context->UpdateSubresource1(elephant_model._constantBuffer.Get(), 0, NULL, &m_constantBufferData_elephant, 0, 0, 0);
	context->UpdateSubresource1(elephant_model._constantBuffer.Get(), 0, NULL, &m_constantBufferData_ghost, 0, 0, 0);
}