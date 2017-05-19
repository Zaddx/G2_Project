#pragma once
#include <vector>

struct Model
{
	// For rendering
	bool _loadingComplete;

	// Index Count
	uint32	_indexCount;

	// Direct 3D resources for the model
	// ComPtr are safe to share
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		_indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		_constantBuffer;

	// Shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11HullShader>	_hullShader;
	Microsoft::WRL::ComPtr<ID3D11DomainShader>	_domainShader;

	// Stuff that individuals models will have
	std::vector<DirectX::XMFLOAT3>				_vertices;
	std::vector<DirectX::XMFLOAT2>				_uvs;
	std::vector<DirectX::XMFLOAT3>				_normals;

	// Path to the texture if it exists
	const char 									*_texture_path;

	// The World Matrix
	DirectX::XMMATRIX							_world_matrix;
};

struct DirectionalLight {
	DirectX::XMFLOAT4 direction;
	DirectX::XMFLOAT4 color;
};

struct PointLight {
	DirectX::XMFLOAT4 position;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT4 radius;
};

struct SpotLight {
	DirectX::XMFLOAT4 position;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT4 cone_direction;
	DirectX::XMFLOAT4 cone_ratio;
	DirectX::XMFLOAT4 inner_cone_ratio;
	DirectX::XMFLOAT4 outer_cone_ratio;
};