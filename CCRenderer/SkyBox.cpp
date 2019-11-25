#include "SkyBox.h"
#include "GameApp.h"
#include "RenderContext.h"
#include "commonh.h"

static XMFLOAT3 ups[6] = 
{
	XMFLOAT3(0.0f, 1.0f, 0.0f),
	XMFLOAT3(0.0f, 1.0f, 0.0f),
	XMFLOAT3(0.0f, 0.0f, -1.0f),
	XMFLOAT3(0.0f, 0.0f, 1.0f),
	XMFLOAT3(0.0f, 1.0f, 0.0f),
	XMFLOAT3(0.0f, 1.0f, 0.0f),
};

static XMFLOAT3 targets[6] = 
{
	XMFLOAT3(1.0f, 0.0f, 0.0f),
	XMFLOAT3(-1.0f, 0.0f, 0.0f),
	XMFLOAT3(0.0f, 1.0f, 0.0f),
	XMFLOAT3(0.0f, -1.0f, 0.0f),
	XMFLOAT3(0.0f, 0.0f, 1.0f),
	XMFLOAT3(0.0f, 0.0f, -1.0f),
};

SkyBox::SkyBox()
{
	m_pVertexShader = NULL;
	m_pPixelShader = NULL;
	m_pVertexLayout = NULL;
	m_pVertexBuffer = NULL;
	m_pIndexBuffer = NULL;
	m_pConstantBuffer = NULL;
	m_pTextureRV = NULL;
	m_pSamplerLinear = NULL;
	m_pDepthStencilStateDisable = NULL;
	m_pDepthStencilStateAble = NULL;
	m_pIrradianceMap = NULL;
	m_pFilterMipmap = NULL;
	m_pBrdfMap = NULL;
}

SkyBox::~SkyBox()
{
	if (m_pVertexBuffer) m_pVertexBuffer->Release();
	if (m_pIndexBuffer) m_pIndexBuffer->Release();
	if (m_pVertexLayout) m_pVertexLayout->Release();
	if (m_pVertexShader) m_pVertexShader->Release();
	if (m_pPixelShader) m_pPixelShader->Release();
	if (m_pTextureRV) m_pTextureRV->Release();
	if (m_pSamplerLinear) m_pSamplerLinear->Release();
	if (m_pDepthStencilStateDisable) m_pDepthStencilStateDisable->Release();
	if (m_pDepthStencilStateAble) m_pDepthStencilStateAble->Release();
	if (m_pIrradianceMap) m_pIrradianceMap->Release();
	if (m_pFilterMipmap) m_pFilterMipmap->Release();
	if (m_pBrdfMap) m_pBrdfMap->Release();
}

HRESULT SkyBox::Init(ID3D11Device* device)
{
	m_pDevice = device;

	HRESULT hr = S_OK;
	GameApp* app = GameApp::getInstance();

	// Create the vertex shader
	std::vector<char> compiledShader;
	SIZE_T size = 0;
	RENDER_CONTEXT::LoadShader(L"shaders/skybox_vs.cso", compiledShader, size);
	RENDER_CONTEXT::CreateVertexShader(&compiledShader[0], size, &m_pVertexShader);

	// Create the pixel shader
	RENDER_CONTEXT::CreatePixelShader(L"shaders/skybox_ps.cso", &m_pPixelShader);


	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);
	hr = RENDER_CONTEXT::GetDevice()->CreateInputLayout(layout, numElements, &compiledShader[0],
		size, &m_pVertexLayout);
	if (FAILED(hr))
		return hr;

	// Create vertex buffer 
	Vertex_Pos vertices[] =
	{
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f)},
		{ XMFLOAT3(1.0f, 1.0f, -1.0f)},
		{ XMFLOAT3(1.0f, 1.0f, 1.0f)},
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f)},
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f)},
		{ XMFLOAT3(1.0f, -1.0f, -1.0f)},
		{ XMFLOAT3(1.0f, -1.0f, 1.0f)},
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f)},
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	hr = RENDER_CONTEXT::GetDevice()->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
	if (FAILED(hr))
		return hr;

	// Create indices buffer
	WORD indices[] =
	{
		0, 1, 3,
		3, 1, 2,

		4, 5, 0,
		0, 5, 1,

		7, 4, 3,
		3, 4, 0,

		5, 6, 1,
		1, 6, 2,

		6, 7, 2,
		2, 7, 3,

		5, 4, 6,
		6, 4, 7,
	};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(indices);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	hr = RENDER_CONTEXT::GetDevice()->CreateBuffer(&bd, &InitData, &m_pIndexBuffer);
	if (FAILED(hr))
		return hr;

	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantShapeBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = RENDER_CONTEXT::GetDevice()->CreateBuffer(&bd, NULL, &m_pConstantBuffer);
	if (FAILED(hr))
		return hr;

	// Load the Texture
	D3DX11_IMAGE_LOAD_INFO loadSMInfo;
	loadSMInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	loadSMInfo.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	hr = app->CreateShaderResourceViewFromFile(L"assets/skybox.dds", &loadSMInfo, NULL, &m_pTextureRV, NULL);
	if (FAILED(hr))
		return hr;

	hr = GameApp::getInstance()->CreateShaderResourceViewFromFile(L"assets/irradiance.dds", NULL, NULL, &m_pIrradianceMap, NULL);
	if (FAILED(hr))
		abort();

	hr = app->CreateShaderResourceViewFromFile(L"assets/mipmaps.dds", NULL, NULL, &m_pFilterMipmap, NULL);
	if (FAILED(hr))
		return hr;

	hr = app->CreateShaderResourceViewFromFile(L"assets/brdf.dds", NULL, NULL, &m_pBrdfMap, NULL);
	if (FAILED(hr))
		return hr;

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = RENDER_CONTEXT::GetDevice()->CreateSamplerState(&sampDesc, &m_pSamplerLinear);
	if (FAILED(hr))
		return hr;

	// Create disable depth stencil state
	D3D11_DEPTH_STENCIL_DESC disableDepthDesc;
	ZeroMemory(&disableDepthDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	disableDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	RENDER_CONTEXT::GetDevice()->CreateDepthStencilState(&disableDepthDesc, &m_pDepthStencilStateDisable);

	// Create able depth stencil state
	D3D11_DEPTH_STENCIL_DESC ableDepthDesc;
	//ZeroMemory(&ableDepthDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	ableDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	RENDER_CONTEXT::GetDevice()->CreateDepthStencilState(&ableDepthDesc, &m_pDepthStencilStateAble);

	XMFLOAT3 ups[6] =
	{
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
	};

	XMFLOAT3 targets[6] =
	{
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
	};

	for (int i = 0; i < 6; i++)
	{
		XMStoreFloat4x4(&m_mViewMatrix[i], XMMatrixLookAtLH(
			XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
			XMLoadFloat3(&targets[i]),
			XMLoadFloat3(&ups[i])));
	}

	XMStoreFloat4x4(&m_mProjMatrix, XMMatrixPerspectiveFovLH(XM_PI * 0.5f, 1.0f, 1.0f, 100.0f));

	return S_OK;
}

void SkyBox::Update(float delta)
{
	const Camera& camera = GameApp::getInstance()->getMainCamera();

	auto world = XMMatrixIdentity();
	auto& eye = camera.GetEye();
	XMMATRIX translate = XMMatrixTranslation(eye.x, eye.y, eye.z);
	world = world * translate;
	XMStoreFloat4x4(&m_World, world);

	auto view = XMLoadFloat4x4(&camera.GetViewMatrix());
	auto proj = XMLoadFloat4x4(&camera.GetProjMatrix());
	auto wvp = world * view * proj;
	XMStoreFloat4x4(&m_Wvp, wvp);
}

void SkyBox::Render()
{
	ID3D11DeviceContext* immediateContext = RENDER_CONTEXT::GetImmediateContext();

	// Update constant buffer
	ConstantShapeBuffer cb;
	cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&m_World));
	cb.mWvp = XMMatrixTranspose(XMLoadFloat4x4(&m_Wvp));
	immediateContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb, 0, 0);

	// Set the input layout
	immediateContext->IASetInputLayout(m_pVertexLayout);

	// Set vertex buffer
	UINT stride = sizeof(Vertex_Pos);
	UINT offset = 0;
	immediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// Set index buffer
	immediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set vertex shader
	immediateContext->VSSetShader(m_pVertexShader, NULL, 0);
	immediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	// Set pixel shader
	immediateContext->PSSetShader(m_pPixelShader, NULL, 0);
	RENDER_CONTEXT::SetPixelShaderResource(10, m_pTextureRV);
	RENDER_CONTEXT::SetPixelSampler(0, m_pSamplerLinear);

	// Disable depth write
	immediateContext->OMSetDepthStencilState(m_pDepthStencilStateDisable, NULL);

	// Render
	immediateContext->DrawIndexed(36, 0, 0);

	// Able depth write
	immediateContext->OMSetDepthStencilState(m_pDepthStencilStateAble, NULL);
}