#pragma once

#include "util_Localization.h"

#include "resource.h"
#include "auto_LocResource.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class KeyFileUtility
{
public:
	struct KeyFileDialogData
	{
		const static int MainSize = 1024;
		const static int TitleSize = 128;

		OPENFILENAME DialogData;
		TCHAR        Filter[MainSize];
		TCHAR        FileName[MainSize];
		TCHAR        Title[TitleSize];
		TCHAR      * Ext[1];

		KeyFileDialogData()
		{
			Filter[0] = FileName[0] = Title[0] = 0;
			Ext[0] = _T(".key");

			LocalizationManagerPtr aMan;

			//1. Localize Filter
			int aSize = MainSize-100;
			aMan->GetStringSafe( KEYOF(IDS_KEYDLG_FILTER), Filter, &aSize);
			CopyMemory( Filter + aSize, _T(" (*.key)\0*.key\0\0" ), 18 * sizeof TCHAR );


			//2. Localize Title
			aSize = TitleSize;
			aMan->GetStringSafe( KEYOF(IDS_KEYDLG_TITLE), Title, &aSize);


			SecureZeroMemory(&DialogData, sizeof OPENFILENAME);

			DialogData.lStructSize      = sizeof OPENFILENAME;
			DialogData.lpstrFilter      = Filter;
			DialogData.nFilterIndex     = 1;
			DialogData.lpstrFile        = FileName;
			DialogData.nMaxFile         = MainSize;
			DialogData.lpstrTitle       = Title;
			DialogData.Flags            = OFN_ENABLESIZING;
		}

		public:
			BOOL GetSaveKeyFileName(HWND theParent)
			{
				DialogData.hwndOwner = theParent;
				DialogData.Flags     |= OFN_OVERWRITEPROMPT;

				BOOL aRet = GetSaveFileName(&DialogData);

				if (aRet != FALSE)
					EnsureFileExtension();

				return aRet;
			}

			BOOL GetOpenKeyFileName(HWND theParent)
			{
				DialogData.hwndOwner = theParent;

				BOOL aRet = GetOpenFileName(&DialogData);

				if (aRet != FALSE)
					EnsureFileExtension();

				return aRet;
			}

			void EnsureFileExtension()
			{
				int aExtIndex = DialogData.nFilterIndex-1;
				const TCHAR * aExtension = Ext[aExtIndex];

				int aFileLength = lstrlen(FileName);
				int aExtLength  = lstrlen(aExtension);

				bool aNoExtension =
					( aFileLength - aExtLength < 0 ) ||
					( lstrcmpi(FileName + aFileLength - aExtLength, aExtension) != 0 );

				if (aNoExtension)
					lstrcat(FileName, aExtension);
			}
	};

public:
	static bool GetKeyFilePath(TCHAR * theOut)
	{
		const FilePathProvider * aPath = Application::InstanceC().GetFilePathProvider();

		int aLength = MAX_PATH;
		HRESULT aRes = aPath->GetPath(FilePathProvider::File_Key, theOut, &aLength);

		return SUCCEEDED(aRes);
	}
};