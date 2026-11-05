/*
    This file is part of Corsix's Mod Studio.

    Corsix's Mod Studio is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Corsix's Mod Studio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Corsix's Mod Studio; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "strconv.h"
#include <malloc.h>
#include <windows.h>
#include "Common.h"

/*
	The narrow (char*) strings that cross the Rainman API are UTF-8. Keeping one
	encoding from end to end means file and folder names work in any language on
	any Windows installation, instead of depending on the system ANSI code page.
*/

namespace
{
	char* _WideToUtf8(const wchar_t* pUnicode)
	{
		if(pUnicode == 0) return 0;
		int iNeeded = WideCharToMultiByte(CP_UTF8, 0, pUnicode, -1, 0, 0, 0, 0);
		if(iNeeded <= 0) return 0;
		char* sUtf8 = new char[iNeeded];
		if(sUtf8 == 0) return 0;
		if(WideCharToMultiByte(CP_UTF8, 0, pUnicode, -1, sUtf8, iNeeded, 0, 0) <= 0)
		{
			delete[] sUtf8;
			return 0;
		}
		return sUtf8;
	}

	wchar_t* _Utf8ToWide(const char* sUtf8)
	{
		if(sUtf8 == 0) return 0;
		int iNeeded = MultiByteToWideChar(CP_UTF8, 0, sUtf8, -1, 0, 0);
		UINT iCodePage = CP_UTF8;
		if(iNeeded <= 0)
		{
			// Not valid UTF-8: treat it as text from an old ANSI file so legacy
			// data still loads instead of turning into an empty string.
			iCodePage = CP_ACP;
			iNeeded = MultiByteToWideChar(CP_ACP, 0, sUtf8, -1, 0, 0);
			if(iNeeded <= 0) return 0;
		}
		wchar_t* wsWide = new wchar_t[iNeeded];
		if(wsWide == 0) return 0;
		if(MultiByteToWideChar(iCodePage, 0, sUtf8, -1, wsWide, iNeeded) <= 0)
		{
			delete[] wsWide;
			return 0;
		}
		return wsWide;
	}
}

wchar_t* AsciiToUnicode(const char* sAscii)
{
	return _Utf8ToWide(sAscii);
}

wchar_t* AsciiToUnicodeDel(char* sAscii)
{
	wchar_t* wsWide = _Utf8ToWide(sAscii);
	delete[] sAscii;
	return wsWide;
}

wchar_t* AsciiToUnicodeFree(char* sAscii)
{
	wchar_t* wsWide = _Utf8ToWide(sAscii);
	free(sAscii);
	return wsWide;
}

char* UnicodeToAscii(const wchar_t* pUnicode)
{
	return _WideToUtf8(pUnicode);
}

bool IsAnsiRepresentable(const wchar_t* pUnicode)
{
	if(pUnicode == 0) return false;
	// Text that ends up inside game files still has to fit the ANSI code page,
	// so the module editor can still warn about it. Paths no longer care.
	if(GetACP() == CP_UTF8) return true;
	int iNeeded = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, pUnicode, -1, 0, 0, 0, 0);
	if(iNeeded <= 0) return false;
	char* sTmp = new char[iNeeded];
	if(!sTmp) return false;
	BOOL bUsedDefaultChar = FALSE;
	BOOL bOk = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, pUnicode, -1, sTmp, iNeeded, 0, &bUsedDefaultChar);
	delete[] sTmp;
	return bOk != FALSE && bUsedDefaultChar == FALSE;
}

wxString AsciiTowxString(const char* sAscii)
{
	if(sAscii == 0) return wxString();
	wxString sResult = wxString::FromUTF8(sAscii);
	if(sResult.IsEmpty() && sAscii[0] != 0)
	{
		// Legacy ANSI text (a .lua or .scar file written years ago, say) is not
		// UTF-8. Fall back to the system code page so it still displays.
		int iNeeded = MultiByteToWideChar(CP_ACP, 0, sAscii, -1, 0, 0);
		if(iNeeded <= 0) return wxString();
		wchar_t* wsWide = new wchar_t[iNeeded];
		MultiByteToWideChar(CP_ACP, 0, sAscii, -1, wsWide, iNeeded);
		sResult = wxString(wsWide);
		delete[] wsWide;
	}
	return sResult;
}

char* wxStringToAscii(const wxString& oStr)
{
	return _WideToUtf8(oStr.c_str());
}

FILE* FopenUtf8(const char* sUtf8Path, const char* sMode)
{
	if(sUtf8Path == 0) return 0;
	wchar_t wsMode[8];
	size_t i = 0;
	for(; sMode[i] && i + 1 < 8; ++i)
	{
		wsMode[i] = (wchar_t)(unsigned char)sMode[i];
	}
	wsMode[i] = 0;
	int iNeeded = MultiByteToWideChar(CP_UTF8, 0, sUtf8Path, -1, 0, 0);
	if(iNeeded <= 0) return 0;
	wchar_t* wsPath = new wchar_t[iNeeded];
	MultiByteToWideChar(CP_UTF8, 0, sUtf8Path, -1, wsPath, iNeeded);
	FILE* f = _wfopen(wsPath, wsMode);
	delete[] wsPath;
	return f;
}