#include "Rendering_Pipeline.h"
#include "RenderContext.h"
#include "RenderTarget.h"
#include "Filter2D.h"

RENDER_TARGET* RENDERING_PIPELINE::_pTargetColor = NULL;
RENDER_TARGET* RENDERING_PIPELINE::_pTargetDepth = NULL;
RENDER_TARGET* RENDERING_PIPELINE::_pTargetNormal = NULL;
RENDER_TARGET* RENDERING_PIPELINE::_pResolveTarget = NULL;
FILTER2D* RENDERING_PIPELINE::_pGammaCorrection = NULL;

void RENDERING_PIPELINE::Init()
{
	_pTargetColor = new RENDER_TARGET(RENDER_CONTEXT::GetWidth(), RENDER_CONTEXT::GetHeight(), DXGI_FORMAT_B8G8R8A8_UNORM, 4, false);
	/*_pTargetDepth = new RENDER_TARGET(RENDER_CONTEXT::GetWidth(), RENDER_CONTEXT::GetHeight(), DXGI_FORMAT_R32_FLOAT, false);
	_pTargetNormal = new RENDER_TARGET(RENDER_CONTEXT::GetWidth(), RENDER_CONTEXT::GetHeight(), DXGI_FORMAT_B8G8R8A8_UNORM, false);*/

    _pResolveTarget = new RENDER_TARGET(RENDER_CONTEXT::GetWidth(), RENDER_CONTEXT::GetHeight(), DXGI_FORMAT_B8G8R8A8_UNORM, 1, false);

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

void RENDERING_PIPELINE::GammaCorrection()
{
	_pGammaCorrection->Init(RENDER_CONTEXT::GetCurrentRenderTarget(RCBI_COLOR_BUFFER)->GetTexture(), RENDER_CONTEXT::GetFrontBuffer());
	_pGammaCorrection->Apply();
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
