#pragma once

#include <d3d11_1.h>

class FILTER2D;
class RENDER_TARGET;
class RENDERING_PIPELINE
{
public:

	static void					Init();
	static void					Release();

	static void					GammaCorrection();

	static void					SetTargetOutputColor();
	static void					SetTargetOutputDepth();
	static void					SetTargetOutputNormal();

	static RENDER_TARGET*		GetTargetOutputColor()
	{
		return _pTargetColor;
	}

	static RENDER_TARGET*		GetTargetOutputDepth()
	{
		return _pTargetDepth;
	}

	static RENDER_TARGET*		GetTargetOutputNormal()
	{
		return _pTargetNormal;
	}

    static RENDER_TARGET* GetResolveTarget()
    {
        return _pResolveTarget;
    }

private:

	static RENDER_TARGET*		_pTargetColor;
	static RENDER_TARGET*		_pTargetDepth;
	static RENDER_TARGET*		_pTargetNormal;

    static RENDER_TARGET*       _pResolveTarget;

	static FILTER2D*			_pGammaCorrection;
};