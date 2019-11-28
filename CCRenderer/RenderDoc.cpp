#include "RenderDoc.h"
#include "renderdoc_app.h"
#include <windows.h>
#include <string> 

extern std::string wstr_to_str(const std::wstring& wstr, UINT CodePage = CP_OEMCP);

static RENDERDOC_API_1_1_2* s_renderdoc{ nullptr };
#define RENDERDOC_DIRECTORY L"./renderdoc"

void RENDER_DOC::Initialize(bool debug)
{
	// s_renderdoc memory is return from dll Library, no need to alloc.
	HMODULE renderdoc = LoadLibrary(TEXT("renderdoc.dll"));
	pRENDERDOC_GetAPI renderdocGetApiFunc = (pRENDERDOC_GetAPI)GetProcAddress(renderdoc, "RENDERDOC_GetAPI");
	renderdocGetApiFunc(eRENDERDOC_API_Version_1_1_2, reinterpret_cast<void**>(&s_renderdoc));

	WIN32_FIND_DATA wfd;
	HANDLE findResult = FindFirstFile(RENDERDOC_DIRECTORY, &wfd);
	if (findResult == INVALID_HANDLE_VALUE)
	{
		CreateDirectory(RENDERDOC_DIRECTORY, NULL);
	}
	FindClose(findResult);

	s_renderdoc->SetCaptureFilePathTemplate(wstr_to_str(RENDERDOC_DIRECTORY"/heavyrain_").c_str());
	if (debug)
	{
		s_renderdoc->SetCaptureOptionU32(eRENDERDOC_Option_DebugDeviceMode, 1);
		s_renderdoc->SetCaptureOptionU32(eRENDERDOC_Option_DebugOutputMute, 0);
		s_renderdoc->SetCaptureOptionU32(eRENDERDOC_Option_VerifyMapWrites, 1);
	}

	RENDERDOC_InputButton k = eRENDERDOC_Key_PrtScrn;
	s_renderdoc->SetCaptureKeys(&k, 1);
}



void RENDER_DOC::Finalize()
{
	// s_renderdoc memory is return from dll Library, no need to free.
	s_renderdoc = nullptr;
}