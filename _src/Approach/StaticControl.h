#pragma once

#include "UxManager.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class CStaticWithLine : public CWindowImpl<CStaticWithLine>
{
public:
	CWindow & operator = (HWND hWnd) throw()
	{
		Subclass(hWnd);
		return *this;
	}

	BOOL AttachToDlgItem(HWND theParent, UINT theDlgID)
	{
		return Subclass( ::GetDlgItem(theParent, theDlgID) );
	}

protected:
	BEGIN_MSG_MAP(CStaticWithLine)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()


private:
	LRESULT OnCreate(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		static HFONT aFont = CreateFont();
		SetFont(aFont, FALSE);

		return 0L;
	}

	LRESULT OnPaint(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		PAINTSTRUCT aPS;
		BeginPaint(&aPS);

		SetBkMode(aPS.hdc, TRANSPARENT);

		CString aBuf;

		int aTextLength = GetWindowTextLength()+1;
		TCHAR * aBasicBuf = aBuf.GetBuffer(aTextLength);

		GetWindowText(aBasicBuf, aTextLength);

		aBuf.ReleaseBuffer();

		HFONT aFont = GetFont();

		HGDIOBJ aPrevObj = SelectObject(aPS.hdc, aFont);

		RECT aRct;
		GetClientRect(&aRct);

		if ( aBuf.GetLength() != 0)
		{
			SIZE aSz;
			GetTextExtentPoint32(aPS.hdc, aBasicBuf, aBuf.GetLength(), &aSz);

			COLORREF aClr = GetSysColor(COLOR_HIGHLIGHT);
			COLORREF aClrPrev = SetTextColor(aPS.hdc, aClr);

			DrawText(aPS.hdc, aBasicBuf, aBuf.GetLength(), &aRct, DT_LEFT);

			SetTextColor(aPS.hdc, aClrPrev);

			aRct.left += aSz.cx + 5;

			TEXTMETRIC aTM;
			GetTextMetrics(aPS.hdc, &aTM);

			int aAscent = aTM.tmAscent * 3 / 4;
			aRct.top += aAscent;
		}
		else
			aRct.top += (aRct.bottom - aRct.top) / 2;

		DrawEdge(aPS.hdc, &aRct, EDGE_ETCHED, BF_TOP);

		SelectObject(aPS.hdc, aPrevObj);

		EndPaint(&aPS);
		return 1;
	}


private:
	static HFONT CreateFont()
	{
		NONCLIENTMETRICS aNCM;
		SecureZeroMemory(&aNCM, sizeof NONCLIENTMETRICS);
		aNCM.cbSize = sizeof NONCLIENTMETRICS;

		BOOL aRes = SystemParametersInfo(SPI_GETNONCLIENTMETRICS, aNCM.cbSize, (void *) &aNCM, 0);

		if (aRes != 0)
		{
			aNCM.lfCaptionFont.lfHeight = aNCM.lfMessageFont.lfHeight * 120 / 100;
			return CreateFontIndirect(&aNCM.lfCaptionFont);
		}
		else
			return NULL;
	}

	BOOL Subclass(HWND theWnd)
	{
		BOOL aRet = SubclassWindow(theWnd);

		static HFONT aFont = CreateFont();
		SetFont(aFont, FALSE);

		return aRet;
	}
};