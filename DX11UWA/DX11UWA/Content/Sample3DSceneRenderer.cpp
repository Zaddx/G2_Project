#include "pch.h"
#include "Sample3DSceneRenderer.h"

#include "..\Common\DirectXHelper.h"

using namespace DX11UWA;

using namespace DirectX;
using namespace Windows::Foundation;

static const XMVECTORF32 eye = { -15.0f, 30.0f, -0.0f, 0.0f };
static const XMVECTORF32 eye2 = { -15.0f, 45.0f, 0.0f, 0.0f };
static const XMVECTORF32 at = { 0.0f, 18.0f, 2.5f, 0.0f };
static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

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
	fovAngleY = 70.0f * (XM_PI / 180.0f);
	zFar = 100.0f;
	zNear = 0.01f;

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

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.


	// When key press set the at to the object's position

	XMStoreFloat4x4(&m_camera, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_camera2, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye2, at, up)));

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixLookAtLH(eye, at, up));
	XMStoreFloat4x4(&m_constantBufferData_master_chief.view, XMMatrixLookAtLH(eye, at, up));
	XMStoreFloat4x4(&m_constantBufferData_elephant.view, XMMatrixLookAtLH(eye, at, up));
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


	// Update or move camera here
	UpdateCamera(timer, 10.0f, 0.75f);

	// Call Update Lights Function
	// UpdateLights();
}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMMATRIX identity = XMMatrixIdentity();
	XMStoreFloat4x4(&m_constantBufferData.model, identity);

	// Set The model of the master chief model to the identity matrix
	XMStoreFloat4x4(&m_constantBufferData_master_chief.model, identity);

	// Set The model of the asgard base model to the identity matrix
	XMStoreFloat4x4(&m_constantBufferData_elephant.model, identity);
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
		if (m_currMousePos->Properties->IsRightButtonPressed && m_prevMousePos)
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
		if (m_currMousePos2->Properties->IsLeftButtonPressed && m_prevMousePos2)
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

	// Setup key press for Camera 2 auto rotation
	// Once, key is pressed auto rotate != auto rotate
	// Then, set the camera to rotate, look at currMousePos for help
	//if (m_kbuttons['H'])
	//	camera2_auto_rotate != camera2_auto_rotate;

	//if (camera2_auto_rotate)
	//{
	//	// Setup the camera to auto rotate 
	//	float dx = m_currMousePos2->Position.X - m_prevMousePos2->Position.X;
	//	float dy = m_currMousePos2->Position.Y - m_prevMousePos2->Position.Y;

	//	XMFLOAT4 pos = XMFLOAT4(m_camera2._41, m_camera2._42, m_camera2._43, m_camera2._44);

	//	m_camera2._41 = 0;
	//	m_camera2._42 = 0;
	//	m_camera2._43 = 0;

	//	XMMATRIX rotX = XMMatrixRotationX(dy * rotSpd * delta_time);
	//	XMMATRIX rotY = XMMatrixRotationY(dx * rotSpd * delta_time);

	//	XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera2);
	//	temp_camera = XMMatrixMultiply(rotX, temp_camera);
	//	temp_camera = XMMatrixMultiply(temp_camera, rotY);

	//	XMStoreFloat4x4(&m_camera2, temp_camera);

	//	m_camera2._41 = pos.x;
	//	m_camera2._42 = pos.y;
	//	m_camera2._43 = pos.z;
	//}

	// Use last row of the object.model to change the at

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
	// Positive (Mouse Wheel

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

	ID3D11ShaderResourceView** texViews[] = { masterChief_meshSRV.GetAddressOf() };
	context->PSSetShaderResources(0, 1, *texViews);

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
	context->UpdateSubresource1(m_constantBuffer_directionalLight.Get(), 0, NULL, &floor_directional_light, 0, 0, 0);
	//context->UpdateSubresource1(m_constantBuffer_pointLight.Get(), 0, NULL, &floor_point_light, 0, 0, 0);
	//context->UpdateSubresource1(m_constantBuffer_spotLight.Get(), 0, NULL, &floor_spot_light, 0, 0, 0);

	//Set the light constant buffers to the floor
	context->PSSetConstantBuffers1(0, 1, m_constantBuffer_directionalLight.GetAddressOf(), nullptr, nullptr);
	//context->PSSetConstantBuffers1(1, 1, m_constantBuffer_pointLight.GetAddressOf(), nullptr, nullptr);
	//context->PSSetConstantBuffers1(2, 1, m_constantBuffer_spotLight.GetAddressOf(), nullptr, nullptr);

	// Attach our vertex shader.
	context->VSSetShader(elephant_model._vertexShader.Get(), nullptr, 0);

	// Attach our pixel shader.
	context->PSSetShader(elephant_model._pixelShader.Get(), nullptr, 0);

	context->DrawIndexed(elephant_model._indexCount, 0, 0);

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
			{ XMFLOAT3(-100.0f, -100.0f, -100.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-100.0f, -100.0f,  100.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-100.0f,  100.0f, -100.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-100.0f,  100.0f,  100.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(100.0f, -100.0f, -100.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(100.0f, -100.0f,  100.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(100.0f,  100.0f, -100.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(100.0f,  100.0f,  100.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
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

	size_t pathSize = strlen(path) + 1;
	wchar_t *wc = new wchar_t[pathSize];
	mbstowcs(&wc[0], path, pathSize);

	HRESULT hr;
	hr = CreateDDSTextureFromFile(device, wc, &masterChief_texture, &masterChief_meshSRV);

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
		//DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer_pointLight));
		//DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer_spotLight));
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
			/*temp.uv.x = 0.5f;
			temp.uv.y = 0.1f;
			temp.uv.z = 0.1f;*/

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
	floor_directional_light.direction = { 0.0f, 0.0f, 1.0f, 0.0f };
	floor_directional_light.color = { 0.250980f , 0.611764f, 1.0f, 0.0f };

	// Initialize the point light data
	floor_point_light.position = { 0.0f, 0.0f, 0.0f, 0.0f };
	floor_point_light.color = { 0.788f, 0.886f, 1.0f, 0.0f };
	floor_point_light.radius.x = 3.0f;

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
	context->UpdateSubresource1(m_constantBuffer_directionalLight.Get(), 0, NULL, &floor_directional_light, 0, 0, 0);
	//context->UpdateSubresource1(m_constantBuffer_pointLight.Get(), 0, NULL, &floor_point_light, 0, 0, 0);
	//context->UpdateSubresource1(m_constantBuffer_spotLight.Get(), 0, NULL, &floor_spot_light, 0, 0, 0);


	// Set the light constant buffers to the floor
	context->PSSetConstantBuffers1(0, 1, m_constantBuffer_directionalLight.GetAddressOf(), nullptr, nullptr);
	//context->PSSetConstantBuffers1(1, 1, m_constantBuffer_pointLight.GetAddressOf(), nullptr, nullptr);
	//context->PSSetConstantBuffers1(2, 1, m_constantBuffer_spotLight.GetAddressOf(), nullptr, nullptr);
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

	auto context = m_deviceResources->GetD3DDeviceContext();
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
	context->UpdateSubresource1(master_chief_model._constantBuffer.Get(), 0, NULL, &m_constantBufferData_master_chief, 0, 0, 0);
	context->UpdateSubresource1(elephant_model._constantBuffer.Get(), 0, NULL, &m_constantBufferData_elephant, 0, 0, 0);
}