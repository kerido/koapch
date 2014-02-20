#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////

class AppModule : public CAppModule
{
public:
	int Run(LPTSTR lpstrCmdLine = NULL, int nCmdShow = SW_SHOWDEFAULT);

	HRESULT Init(ATL::_ATL_OBJMAP_ENTRY* theObjMap, HINSTANCE theInstance, const GUID* theLibID = NULL);

	void Term();


private:
	class Application *        myApplication;	//!< Pimpl pattern
};