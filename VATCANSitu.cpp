// VATCANSitu.cpp : Defines the initialization routines for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "VATCANSitu.h"
#include "SituPlugin.h"
#include "EuroScopePlugIn.h"
#include <gdiplus.h>

using namespace Gdiplus;

// GDI+ initialization
ULONG_PTR m_gdiplusToken = 0;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CVATCANSituApp

SituPlugin* gpMyPlugIn = NULL;

void __declspec (dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
	
	// Initialize GDI+
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);
	
	*ppPlugInInstance = gpMyPlugIn = new SituPlugin();
}

void __declspec (dllexport) EuroScopePlugInExit(void)
{
	delete gpMyPlugIn;
}

BEGIN_MESSAGE_MAP(CVATCANSituApp, CWinApp)
END_MESSAGE_MAP()


// CVATCANSituApp construction

CVATCANSituApp::CVATCANSituApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CVATCANSituApp object

CVATCANSituApp theApp;


// CVATCANSituApp initialization

BOOL CVATCANSituApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}
