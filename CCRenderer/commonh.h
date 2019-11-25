#pragma once

#include <xnamath.h>

#define MAX_CASCADED 8

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
};

struct VertexTBN
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT3 Tangent;
	XMFLOAT3 BiNormal;
	XMFLOAT2 Tex;
};

struct Vertex_Pos
{
	XMFLOAT3 Pos;
};

struct Vertex_PosUV
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};

struct ConstantShapeBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mWvp;	
};

struct ConstantCameraBuffer
{
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 vEye;
};

struct ConstantDepthBuffer
{
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 vTextureSize;
};

struct ConstantShadowMapBuffer
{
	XMMATRIX mView;
	XMMATRIX mProjection[MAX_CASCADED];
	XMFLOAT4 vfCascadeFrustumsEyeSpaceDepths[2];
	XMFLOAT4 vTextureSize;
};
