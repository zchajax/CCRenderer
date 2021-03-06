#include "Rendering_Pipeline.h"
#include "RenderContext.h"
#include "RenderTarget.h"
#include "Filter2D.h"
#include "Texture_DX11.h"
#include "RenderDefs.h"

RENDER_TARGET* RENDERING_PIPELINE::_pTargetColor = NULL;
RENDER_TARGET* RENDERING_PIPELINE::_pTargetDepth = NULL;
RENDER_TARGET* RENDERING_PIPELINE::_pTargetNormal = NULL;
RENDER_TARGET* RENDERING_PIPELINE::_pResolveTarget = NULL;
FILTER2D* RENDERING_PIPELINE::_pGammaCorrection = NULL;

void RENDERING_PIPELINE::Init()
{
    extern bool gEnableMSAA;
    UINT8 sampleCount = gEnableMSAA ? MSAA_COUNT : 1;

	_pTargetColor = new RENDER_TARGET(RENDER_CONTEXT::GetWidth(), RENDER_CONTEXT::GetHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, sampleCount, false);
	_pTargetDepth = new RENDER_TARGET(RENDER_CONTEXT::GetWidth(), RENDER_CONTEXT::GetHeight(), DXGI_FORMAT_R32_FLOAT, sampleCount, false);
	_pTargetNormal = new RENDER_TARGET(RENDER_CONTEXT::GetWidth(), RENDER_CONTEXT::GetHeight(), DXGI_FORMAT_B8G8R8A8_UNORM, sampleCount, false);

    _pResolveTarget = new RENDER_TARGET(RENDER_CONTEXT::GetWidth(), RENDER_CONTEXT::GetHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, 1, false);

	_pGammaCorrection = new FILTER2D(L"shaders/Filter2D_ps.cso");
}

void RENDERING_PIPELINE::Release()
{
	if (_pTargetColor)
	{
		delete _pTargetColor;

		_pTargetColor = NULL;
	}

	if (_pTargetDepth)
	{
		delete _pTargetDepth;

		_pTargetDepth = NULL;
	}

	if (_pTargetNormal)
	{
		delete _pTargetNormal;

		_pTargetNormal = NULL;
	}

    if (_pResolveTarget)
    {
        delete _pResolveTarget;

        _pResolveTarget = NULL;
    }
}

void RENDERING_PIPELINE::Resolve()
{
    TEXTURE_DX11* srcTexture = RENDER_CONTEXT::GetCurrentRenderTarget()->GetTexture();

    TEXTURE_DX11* resolveTexture = _pResolveTarget->GetTexture();

    RENDER_CONTEXT::GetImmediateContext()->ResolveSubresource(resolveTexture->GetD3DTexture(), 0, srcTexture->GetD3DTexture(), 0, srcTexture->GetFormat());

    RENDER_CONTEXT::SetCurrentRenderTarget(_pResolveTarget);
}

void RENDERING_PIPELINE::GammaCorrection()
{
    RENDER_CONTEXT::PushMarker(MARK("Gamma Correction"));

	_pGammaCorrection->Init(RENDER_CONTEXT::GetCurrentRenderTarget(RCBI_COLOR_BUFFER)->GetTexture(), RENDER_CONTEXT::GetFrontBuffer());
	_pGammaCorrection->Apply();

    RENDER_CONTEXT::PopMarker();
}

void RENDERING_PIPELINE::SetTargetOutputColor()
{
	RENDER_CONTEXT::SetCurrentRenderTarget(_pTargetColor, RCBI_COLOR_BUFFER);
}

void RENDERING_PIPELINE::SetTargetOutputDepth()
{
	RENDER_CONTEXT::SetCurrentRenderTarget(_pTargetDepth, RCBI_DEPTH_BUFFER);
}

void RENDERING_PIPELINE::SetTargetOutputNormal()
{
	RENDER_CONTEXT::SetCurrentRenderTarget(_pTargetNormal, RCBI_NORMAL_BUFFER);
}
