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

#ifndef _STRCONV_H_
#define _STRCONV_H_

#include <string.h>
#include <wx/string.h>

/*
	\return Returns a string, or 0 on error
*/
wchar_t* AsciiToUnicode(const char* sAscii);

/*
	\return Returns a string, or 0 on error
*/
wchar_t* AsciiToUnicodeDel(char* sAscii);

/*
	\return Returns a string, or 0 on error
*/
wchar_t* AsciiToUnicodeFree(char* sAscii);

/*
	\return Returns a string, or 0 on error
*/
char* UnicodeToAscii(const wchar_t* pUnicode);

/*
	Tests whether the string can be converted to the system ANSI code page
	without losing characters. Paths that fail this test cannot be passed to
	the file APIs, so callers should refuse them with a clear message.
*/
bool IsAnsiRepresentable(const wchar_t* pUnicode);

/*
	\return Returns a string
*/
wxString AsciiTowxString(const char* sAscii);

/*
	\return Returns a string, or 0 on error
*/
char* wxStringToAscii(const wxString& oStr);

/*! Opens a file whose path is UTF-8 (the encoding used throughout the program). */
FILE* FopenUtf8(const char* sUtf8Path, const char* sMode);

#endif