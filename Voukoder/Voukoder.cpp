#include "Voukoder.h"
#include <wx/wx.h>
#include "../Core/wxVoukoderDialog.h"

class actctx_activator
{
protected:
	ULONG_PTR m_cookie; // Cookie for context deactivation

public:
	// Construct the activator and activates the given activation context
	actctx_activator(_In_ HANDLE hActCtx)
	{
		if (!ActivateActCtx(hActCtx, &m_cookie))
			m_cookie = 0;
	}

	// Deactivates activation context and destructs the activator
	virtual ~actctx_activator()
	{
		if (m_cookie)
			DeactivateActCtx(0, m_cookie);
	}
};

HINSTANCE g_instance = NULL;
HANDLE g_act_ctx = NULL;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		g_instance = hModule;
		GetCurrentActCtx(&g_act_ctx);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		if (g_act_ctx)
			ReleaseActCtx(g_act_ctx);
	}

	return TRUE;
}

void init()
{
	Voukoder::Config::Get();
}

bool open_config_dialog(std::string* settings, HWND hwnd)
{
	// Default window is the active window
	if (hwnd == NULL)
		hwnd = GetActiveWindow();

	// Restore plugin's activation context.
	actctx_activator actctx(g_act_ctx);

	// Initialize application.
	new wxApp();
	wxEntryStart(g_instance);

	// Create and launch configuration dialog.
	ExportInfo exportInfo;
	exportInfo.Deserialize(*settings);

	int result;
	
	{
		// Create wxWidget-approved parent window.
		wxWindow parent;
		parent.SetHWND((WXHWND)hwnd);
		parent.AdoptAttributesFromHWND();
		wxTopLevelWindows.Append(&parent);

		// Show the dialog
		wxVoukoderDialog dialog(&parent, exportInfo);
		result = dialog.ShowModal();

		wxTopLevelWindows.DeleteObject(&parent);
		parent.SetHWND((WXHWND)NULL);
	}

	wxEntryCleanup();

	if (result == (int)wxID_OK)
	{
		settings->assign(exportInfo.Serialize());
		return true;
	}

	return false;
}
