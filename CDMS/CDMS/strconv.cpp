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

wchar_t* AsciiToUnicode(const char* sAscii)
{
	size_t iLen = strlen(sAscii) + 1;
	wchar_t *pUnicode = new wchar_t[iLen];
	if(!pUnicode) return 0;
	for(size_t i = 0; i < iLen; ++i)
	{
		pUnicode[i] = (wchar_t)sAscii[i];
	}
	return pUnicode;
}

wchar_t* AsciiToUnicodeDel(char* sAscii)
{
	size_t iLen = strlen(sAscii) + 1;
	wchar_t *pUnicode = new wchar_t[iLen];
	if(!pUnicode)
	{
		delete[] sAscii;
		return 0;
	}
	for(size_t i = 0; i < iLen; ++i)
	{
		pUnicode[i] = (wchar_t)sAscii[i];
	}
	delete[] sAscii;
	return pUnicode;
}

wchar_t* AsciiToUnicodeFree(char* sAscii)
{
	size_t iLen = strlen(sAscii) + 1;
	wchar_t *pUnicode = new wchar_t[iLen];
	if(!pUnicode)
	{
		free(sAscii);
		return 0;
	}
	for(size_t i = 0; i < iLen; ++i)
	{
		pUnicode[i] = (wchar_t)sAscii[i];
	}
	free(sAscii);
	return pUnicode;
}

char* UnicodeToAscii(const wchar_t* pUnicode)
{
	if(!pUnicode) return 0;
	// Rainman and the file APIs it uses take ANSI strings, so convert through
	// the system ANSI code page. A plain truncation to one byte per character
	// used to turn everything above Latin-1 into '?', which made the program
	// unusable on localized Windows installations (Cyrillic, Greek, CJK, ...).
	int iNeeded = WideCharToMultiByte(CP_ACP, 0, pUnicode, -1, 0, 0, 0, 0);
	if(iNeeded <= 0) return 0;
	char* sAscii = new char[iNeeded];
	if(!sAscii) return 0;
	if(WideCharToMultiByte(CP_ACP, 0, pUnicode, -1, sAscii, iNeeded, 0, 0) <= 0)
	{
		delete[] sAscii;
		return 0;
	}
	return sAscii;
}

bool IsAnsiRepresentable(const wchar_t* pUnicode)
{
	if(!pUnicode) return false;
	// With the Windows "Use Unicode UTF-8 for worldwide language support"
	// option the ANSI code page is UTF-8, which can represent every string (and
	// rejects the WC_NO_BEST_FIT_CHARS flag). Treating it as unrepresentable
	// would refuse every path on such installations.
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
	/*
	wchar_t* pUnicode = AsciiToUnicode(sAscii);
	if(!pUnicode) return wxString();
	wxString r(pUnicode);
	delete[] pUnicode;
	return r;
	*/
	return wxString(sAscii, wxConvUTF8);
}

char* wxStringToAscii(const wxString& oStr)
{
	return UnicodeToAscii(oStr.c_str());
}