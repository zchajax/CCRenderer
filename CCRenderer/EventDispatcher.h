#pragma once

#include <windows.h>

class EventDispatcher
{

public:

	~EventDispatcher();

	static EventDispatcher* getInstance();

	static void destroyInstance();

	void OnEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void Update(float delta);

private:

	EventDispatcher();

	static EventDispatcher* m_Instance;

	//Is mouse left button down
	bool m_IsLDraging;

	//Is mouse right button down
	bool m_IsRDraging;

	short m_MouseX;

	short m_MouseY;
};