#include "pch.h"
#include "ObjLoader.h"

float Clamp(float _val, float _max, float _min) {
	if (_min >= _val)
		return _min;
	else if (_max <= _val)
		return _max;
	else
		return _val;

}

DirectX::XMFLOAT3 Lerp(DirectX::XMFLOAT3 _c1, DirectX::XMFLOAT3 _c2, float ratio) {
	DirectX::XMFLOAT3 temp;

	temp.x = (_c2.x - _c1.x) * ratio + _c1.x;
	temp.y = (_c2.y - _c1.y) * ratio + _c1.y;
	temp.z = (_c2.z - _c1.z) * ratio + _c1.z;

	return temp;
}

DirectX::XMFLOAT3 Vector_Subtraction(DirectX::XMFLOAT3 &_a, DirectX::XMFLOAT3 &_b) {
	DirectX::XMFLOAT3 temp;

	temp.x = _a.x - _b.x;
	temp.y = _a.y - _b.y;
	temp.z = _a.z + _b.z;

	return temp;
}

bool IsZero(float a)
{
	return (fabs(a))<EPSILON;
}

float Vector_LengthSq(DirectX::XMFLOAT3 v)
{
	float length;
	float x2, y2, z2;

	x2 = v.x * v.x;
	y2 = v.y * v.y;
	z2 = v.z * v.z;

	length = x2 + y2 + z2;

	return length;
}

float Vector_Length(DirectX::XMFLOAT3 v)
{
	return sqrtf(Vector_LengthSq(v));
}

DirectX::XMFLOAT3 Vector_Normalize(DirectX::XMFLOAT3 v)
{
	DirectX::XMFLOAT3 x;

	float length;

	length = Vector_Length(v);

	if (IsZero(length))
	{
		x.x = x.y = x.z = 0;
		return x;
	}
	else
	{
		x.x = v.x / length;
		x.y = v.y / length;
		x.z = v.z / length;
	}

	return x;
}

float Vector_Dot(DirectX::XMFLOAT3 v, DirectX::XMFLOAT3 w)
{
	return  v.x * w.x + v.y * w.y + v.z * w.z;
}

DirectX::XMFLOAT3 Vector_Scalar_Multiply(DirectX::XMFLOAT3 v, float s)
{
	DirectX::XMFLOAT3 x = v;

	x.x = v.x * s;
	x.y = v.y * s;
	x.z = v.z * s;

	return x;
}

bool loadOBJ(const char * path, std::vector<DX11UWA::VertexPositionUVNormal> &out_vertices, std::vector<unsigned int> &out_indices, std::vector<DirectX::XMFLOAT3> &out_normals, std::vector<DirectX::XMFLOAT2> &out_uvs)
{
	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<DirectX::XMFLOAT3> temp_vertices;
	std::vector<DirectX::XMFLOAT2> temp_uvs;
	std::vector<DirectX::XMFLOAT3> temp_normals;

	std::vector<DX11UWA::VertexPositionUVNormal> vertices;
	std::vector<DirectX::XMFLOAT3> normals;
	std::vector<DirectX::XMFLOAT2> uvs;
	std::vector<unsigned int> indices;

	FILE * file = fopen(path, "r");

	if (file == NULL)
	{
		printf("Impossible to open the file !\n");
		return false;
	}

	while (true) {
		char lineHeader[256];

		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);

		if (res == EOF)
			break;		// End of file. Quit the loop.

						// else : parse lineHeader
		if (strcmp(lineHeader, "v") == 0)
		{
			DirectX::XMFLOAT3 vertex;

			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0)
		{
			DirectX::XMFLOAT2 uv;

			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0)
		{
			DirectX::XMFLOAT3 normal;

			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0)
		{
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0],
				&vertexIndex[1], &uvIndex[1], &normalIndex[1],
				&vertexIndex[2], &uvIndex[2], &normalIndex[2]);

			if (matches != 9)
			{
				printf("File can't be read by our simple parser: (Try exporting with other options)\n");
			}

			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++)
	{
		DX11UWA::VertexPositionUVNormal temp;

		unsigned int vertexIndex = vertexIndices[i];
		DirectX::XMFLOAT3 vertex = temp_vertices[vertexIndex - 1];
		temp.pos.x = vertex.x;
		temp.pos.y = vertex.y;
		temp.pos.z = vertex.z;

		unsigned int uvIndex = uvIndices[i];
		DirectX::XMFLOAT2 uv = temp_uvs[uvIndex - 1];
		uvs.push_back(uv);

		unsigned int normalIndex = normalIndices[i];
		DirectX::XMFLOAT3 normal = temp_normals[normalIndex - 1];
		normals.push_back(normal);

		// Setup the Vertex Color
		temp.uv.x = uv.x;
		temp.uv.y = 1.0f - uv.y;

		// Setup the Vertex Normals
		temp.normal = normal;

		vertices.push_back(temp);
		indices.push_back(i);
	}

	out_vertices = vertices;
	out_indices = indices;
	out_normals = normals;
	out_uvs = uvs;

	return true;
}

