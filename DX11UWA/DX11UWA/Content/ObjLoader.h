#pragma once
#include <vector>
#include "Content\ShaderStructures.h"

#define EPSILON 0.00001f

float Clamp(float _val, float _max, float _min);

DirectX::XMFLOAT3 Lerp(DirectX::XMFLOAT3 _c1, DirectX::XMFLOAT3 _c2, float ratio);

DirectX::XMFLOAT3 Vector_Subtraction(DirectX::XMFLOAT3 &_a, DirectX::XMFLOAT3 &_b);

bool IsZero(float a);

float Vector_LengthSq(DirectX::XMFLOAT3 v);

float Vector_Length(DirectX::XMFLOAT3 v);

DirectX::XMFLOAT3 Vector_Normalize(DirectX::XMFLOAT3 v);

float Vector_Dot(DirectX::XMFLOAT3 v, DirectX::XMFLOAT3 w);

DirectX::XMFLOAT3 Vector_Scalar_Multiply(DirectX::XMFLOAT3 v, float s);

bool loadOBJ(const char * path, std::vector<DX11UWA::VertexPositionUVNormal> &out_vertices, std::vector<unsigned int> &out_indices, std::vector<DirectX::XMFLOAT3> &out_normals, std::vector<DirectX::XMFLOAT2> &out_uvs);