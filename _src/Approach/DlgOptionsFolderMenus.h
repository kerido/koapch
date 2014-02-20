#pragma once

#include "Preferences.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class DlgFolderMenus :
	public CDialogImpl<DlgFolderMenus>,
	public CWinDataExchange<DlgFolderMenus>
{
private:
	ApplicationSettings * mySt;

public:
	enum { IDD = IDD_MAIN_FEATURES_FOLDERMENUS };


public:
	DlgFolderMenus(ApplicationSettings * theSettings) : mySt(theSettings)
	{ }

	BEGIN_MSG_MAP(DlgFolderMenus)
		MESSAGE_HANDLER(WM_INITDIALOG, MsgHandler_InitDialog)
		COMMAND_ID_HANDLER(IDOK,       CmdHandler_Ok)
		COMMAND_ID_HANDLER(IDCANCEL,   CmdHandler_Cancel)
	END_MSG_MAP()


private:
	LRESULT MsgHandler_InitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return TRUE;
	}

	LRESULT CmdHandler_Ok(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(IDOK);
		return 0L;
	}

	LRESULT CmdHandler_Cancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(IDCANCEL);
		return 0L;
	}
};

