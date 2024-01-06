// Make fonts, brushes and pens once, to not need to do them over and over again
#include <afxwin.h>
#include <cstring>

#pragma once
class CFontHelper {
public:
	static CFont Euroscope14;
	static CFont EuroscopeBold;
	static CFont Euroscope16;
	static CFont Segoe12;
	static CFont Segoe14;

	static void CreateFonts(); 
	static void DeleteFonts();
};