// VATCANSitu.h : main header file for the VATCANSitu DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CVATCANSituApp
// See VATCANSitu.cpp for the implementation of this class
//

class CVATCANSituApp : public CWinApp
{
public:
	CVATCANSituApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
