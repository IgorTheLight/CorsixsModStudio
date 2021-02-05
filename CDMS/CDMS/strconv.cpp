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
	size_t iLen = wcslen(pUnicode) + 1;
	char *sAscii = new char[iLen];
	if(!sAscii) return 0;
	for(size_t i = 0; i < iLen; ++i)
	{
		wchar_t iChar = pUnicode[i];
		if(iChar & ~0xFF)
			sAscii[i] = '?';
		else
			sAscii[i] = (char)iChar;
	}
	return sAscii;
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