#include "EventDispatcher.h"
#include "GameApp.h"

#define SPEED 5

EventDispatcher* EventDispatcher::m_Instance = NULL;

EventDispatcher::EventDispatcher()
{
	m_IsLDraging = false;

	m_IsRDraging = false;

	m_MouseX = 0;

	m_MouseY = 0;
}

EventDispatcher::~EventDispatcher()
{

}

EventDispatcher* EventDispatcher::getInstance()
{
	if (!m_Instance)
	{
		m_Instance = new EventDispatcher();
	}

	return m_Instance;
}

void EventDispatcher::destroyInstance()
{
	if (m_Instance)
	{
		delete m_Instance;

		m_Instance = NULL;
	}
}

void EventDispatcher::OnEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_LBUTTONDOWN:
		{
			m_IsLDraging = true;
			m_MouseX = (short)LOWORD(lParam);
			m_MouseY = (short)HIWORD(lParam);
		}
		break;

	case WM_RBUTTONDOWN:
		{
			m_IsRDraging = true;
			m_MouseX = (short)LOWORD(lParam);
			m_MouseY = (short)HIWORD(lParam);
		}
		break;

	case WM_MOUSEMOVE:
		{
			if (m_IsLDraging)
			{
				Camera& camera = GameApp::getInstance()->getMainCamera();

				float offsetX =  - ((short)LOWORD(lParam) - m_MouseX) * 0.01f;
				float offsetY = ((short)HIWORD(lParam) - m_MouseY) * 0.01f;

				auto& eye = camera.GetEye();
				auto& lookAt = camera.GetLookAt();

				auto&  rightVector = camera.GetRightVector();
				auto&  upVector = camera.GetUp();

				auto offsetRight = XMLoadFloat4(&rightVector) * offsetX;
				auto offsetUp = XMLoadFloat4(&upVector) * offsetY;

				auto newEyeVector = XMLoadFloat4(&eye) + offsetRight + offsetUp;
				auto newLookAtVector = XMLoadFloat4(&lookAt) + offsetRight + offsetUp;

				XMFLOAT4 newEye;
				XMStoreFloat4(&newEye, newEyeVector);

				XMFLOAT4 newLookAt;
				XMStoreFloat4(&newLookAt, newLookAtVector);

				camera.SetViewParams(newEye, newLookAt); 
			}
			else if (m_IsRDraging)
			{
				Camera& camera = GameApp::getInstance()->getMainCamera();

				float offsetX = -((short)LOWORD(lParam) - m_MouseX) * 0.005f;
				float offsetY = ((short)HIWORD(lParam) - m_MouseY) * 0.005f;

				auto& eye = camera.GetEye();
				auto& lookAt = camera.GetLookAt();
				auto& lookVector = camera.GetLookVector();
				auto& rightVector = camera.GetRightVector();
				auto& upVector = camera.GetUp();

				auto rotationRight = XMMatrixRotationAxis(XMLoadFloat4(&rightVector), offsetY);
				// auto rotationUp = XMMatrixRotationAxis(XMLoadFloat4(&upVector), -offsetX);

				auto rotationUp = XMMatrixRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), -offsetX);

				auto newlookVector = XMVector4Transform(XMLoadFloat4(&lookVector), rotationRight);
				newlookVector = XMVector4Transform(newlookVector, rotationUp);
				auto newUpVector = XMVector4Transform(XMLoadFloat4(&upVector), rotationRight);
				newUpVector = XMVector4Transform(newUpVector, rotationUp);
				auto newLookAtVector = XMLoadFloat4(&eye) + newlookVector;

				XMFLOAT4  newUp;
				XMStoreFloat4(&newUp, newUpVector);

				XMFLOAT4 newLookAt;
				XMStoreFloat4(&newLookAt, newLookAtVector);

				camera.SetViewParams(eye, newLookAt, newUp);
			}

			m_MouseX = (short)LOWORD(lParam);
			m_MouseY = (short)HIWORD(lParam);
		}
		break;

	case WM_LBUTTONUP:
		{
			m_IsLDraging = false;
		}
		break;

	case WM_RBUTTONUP:
		{
			m_IsRDraging = false;
		}

	case WM_MOUSEWHEEL:
		{
			Camera& camera = GameApp::getInstance()->getMainCamera();

			float delta = (short)HIWORD(wParam) * 0.01f;

			auto& eye = camera.GetEye();
			auto& lookAt = camera.GetLookAt();
			auto& lookVector = camera.GetLookVector();

			auto offsetLook = XMLoadFloat4(&lookVector) * delta;

			auto newEyeVector = XMLoadFloat4(&eye) + offsetLook;
			auto newLookAtVector = XMLoadFloat4(&lookAt) + offsetLook;

			XMFLOAT4 newEye;
			XMStoreFloat4(&newEye, newEyeVector);

			XMFLOAT4 newLookAt;
			XMStoreFloat4(&newLookAt, newLookAtVector);

			camera.SetViewParams(newEye, newLookAt);
		}
		break;

	default:
		break;
	}
}

void EventDispatcher::Update(float delta)
{
	// Judge the current window is activing 
	DWORD proccessId = 0;
	GetWindowThreadProcessId(GetActiveWindow(), &proccessId);
	if (GetCurrentProcessId() != proccessId)
	{
		return;
	}

	// Tap  'w' 's' 'a' 'd' to move camera 
	Camera& camera = GameApp::getInstance()->getMainCamera();

	float offsetX = 0.0f;
	float offsetY = 0.0f;
	float offsetZ = 0.0f;

	if (GetAsyncKeyState('W') & 0x8000)
	{
		offsetZ += SPEED * delta;
	}

	if (GetAsyncKeyState('S') & 0x8000)
	{
		offsetZ -= SPEED * delta;
	}

	if (GetAsyncKeyState('A') & 0x8000)
	{
		offsetX -= SPEED * delta;
	}

	if (GetAsyncKeyState('D') & 0x8000)
	{
		offsetX += SPEED * delta;
	}

	if (GetAsyncKeyState('Q') & 0x8000)
	{
		offsetY += SPEED * delta;
	}

	if (GetAsyncKeyState('E') & 0x8000)
	{
		offsetY -= SPEED * delta;
	}

	if (offsetX || offsetY || offsetZ)
	{
		auto& eye = camera.GetEye();
		auto& lookAt = camera.GetLookAt();

		auto rightVector = camera.GetRightVector();
		auto upVector = camera.GetUp();
		auto lookVector = camera.GetLookVector();

		auto offsetRight = XMLoadFloat4(&rightVector) * offsetX;
		auto offsetUp = XMLoadFloat4(&upVector) * offsetY;
		auto offsetLook = XMLoadFloat4(&lookVector) * offsetZ;

		auto newEyeVector = XMLoadFloat4(&eye) + offsetRight + offsetUp + offsetLook;
		auto newLookAtVector = XMLoadFloat4(&lookAt) + offsetRight + offsetUp + offsetLook;

		XMFLOAT4 newEye;
		XMStoreFloat4(&newEye, newEyeVector);

		XMFLOAT4 newLookAt;
		XMStoreFloat4(&newLookAt, newLookAtVector);

		camera.SetViewParams(newEye, newLookAt);
	}
}