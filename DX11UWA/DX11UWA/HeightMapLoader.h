#pragma once
#include <vector>

/////////////////////////////////////////////////////////////////////////////////////
// Followed: https://www.braynzarsoft.net/viewtutorial/q16390-30-heightmap-terrain //
/////////////////////////////////////////////////////////////////////////////////////

using namespace DirectX;
using namespace std;

// Will Declare these in the Sample3DSceneRenderer.h
// int NumFaces = 0;
// int NumVertices = 0;

typedef struct tagBITMAPFILEHEADER
{
	WORD bfType;
	DWORD bfSize;
	WORD bfReserved1;
	WORD bfReserved2;
	DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

struct HeightMapInfo
{
	int terrainWidth;        // Width of heightmap
	int terrainHeight;        // Height (Length) of heightmap
	XMFLOAT3 *heightMap;    // Array to store terrain's vertex positions
};

bool HeightMapLoad(char* filename, HeightMapInfo &hminfo); 