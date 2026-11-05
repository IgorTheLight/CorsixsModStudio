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

#include "PathUtil.h"
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#ifdef _WIN32
#include <windows.h>
#endif

namespace
{
	int _WidenMode(const char* sMode, wchar_t* wsMode, size_t iCount)
	{
		size_t i = 0;
		for(; sMode[i] && i + 1 < iCount; ++i)
		{
			wsMode[i] = (wchar_t)(unsigned char)sMode[i];
		}
		wsMode[i] = 0;
		return (int)i;
	}
}

#ifdef _WIN32

wchar_t* PathUtil_Utf8ToWide(const char* sUtf8)
{
	if(sUtf8 == 0)
	{
		wchar_t* wsEmpty = new wchar_t[1];
		wsEmpty[0] = 0;
		return wsEmpty;
	}
	int iNeeded = MultiByteToWideChar(CP_UTF8, 0, sUtf8, -1, 0, 0);
	if(iNeeded <= 0)
	{
		// Not valid UTF-8; fall back to the system code page so old files still
		// open instead of silently turning into an empty path.
		iNeeded = MultiByteToWideChar(CP_ACP, 0, sUtf8, -1, 0, 0);
		if(iNeeded <= 0)
		{
			wchar_t* wsEmpty = new wchar_t[1];
			wsEmpty[0] = 0;
			return wsEmpty;
		}
		wchar_t* wsPath = new wchar_t[iNeeded];
		MultiByteToWideChar(CP_ACP, 0, sUtf8, -1, wsPath, iNeeded);
		return wsPath;
	}
	wchar_t* wsPath = new wchar_t[iNeeded];
	if(MultiByteToWideChar(CP_UTF8, 0, sUtf8, -1, wsPath, iNeeded) <= 0)
	{
		wsPath[0] = 0;
	}
	return wsPath;
}

char* PathUtil_WideToUtf8(const wchar_t* wsWide)
{
	if(wsWide == 0)
	{
		char* sEmpty = new char[1];
		sEmpty[0] = 0;
		return sEmpty;
	}
	int iNeeded = WideCharToMultiByte(CP_UTF8, 0, wsWide, -1, 0, 0, 0, 0);
	if(iNeeded <= 0)
	{
		char* sEmpty = new char[1];
		sEmpty[0] = 0;
		return sEmpty;
	}
	char* sUtf8 = new char[iNeeded];
	if(WideCharToMultiByte(CP_UTF8, 0, wsWide, -1, sUtf8, iNeeded, 0, 0) <= 0)
	{
		sUtf8[0] = 0;
	}
	return sUtf8;
}

FILE* PathUtil_fopen(const char* sUtf8Path, const char* sMode)
{
	wchar_t* wsPath = PathUtil_Utf8ToWide(sUtf8Path);
	wchar_t wsMode[8];
	_WidenMode(sMode, wsMode, 8);
	FILE* f = _wfopen(wsPath, wsMode);
	delete[] wsPath;
	return f;
}

FILE* PathUtil_fopenW(const wchar_t* wsPath, const char* sMode)
{
	wchar_t wsMode[8];
	_WidenMode(sMode, wsMode, 8);
	return _wfopen(wsPath, wsMode);
}

int PathUtil_mkdir(const char* sUtf8Path)
{
	wchar_t* wsPath = PathUtil_Utf8ToWide(sUtf8Path);
	BOOL bOk = CreateDirectoryW(wsPath, 0);
	DWORD dwError = bOk ? 0 : GetLastError();
	delete[] wsPath;
	if(bOk) return 0;
	return (dwError == ERROR_ALREADY_EXISTS) ? 0 : -1;
}

int PathUtil_remove(const char* sUtf8Path)
{
	wchar_t* wsPath = PathUtil_Utf8ToWide(sUtf8Path);
	BOOL bOk = DeleteFileW(wsPath);
	delete[] wsPath;
	return bOk ? 0 : -1;
}

bool PathUtil_moveReplace(const char* sUtf8Source, const char* sUtf8Destination)
{
	wchar_t* wsSource = PathUtil_Utf8ToWide(sUtf8Source);
	wchar_t* wsDestination = PathUtil_Utf8ToWide(sUtf8Destination);
	BOOL bOk = MoveFileExW(wsSource, wsDestination, MOVEFILE_REPLACE_EXISTING);
	delete[] wsSource;
	delete[] wsDestination;
	return bOk != FALSE;
}

char* PathUtil_AnsiToUtf8(const char* sAnsi)
{
	if(sAnsi == 0)
	{
		char* sEmpty = new char[1];
		sEmpty[0] = 0;
		return sEmpty;
	}
	int iWide = MultiByteToWideChar(CP_ACP, 0, sAnsi, -1, 0, 0);
	if(iWide <= 0)
	{
		char* sCopy = new char[strlen(sAnsi) + 1];
		strcpy(sCopy, sAnsi);
		return sCopy;
	}
	wchar_t* wsText = new wchar_t[iWide];
	MultiByteToWideChar(CP_ACP, 0, sAnsi, -1, wsText, iWide);
	char* sUtf8 = PathUtil_WideToUtf8(wsText);
	delete[] wsText;
	return sUtf8;
}

char* PathUtil_AnsiToUtf8Malloc(const char* sAnsi)
{
	char* sUtf8 = PathUtil_AnsiToUtf8(sAnsi);
	char* sCopy = (char*)malloc(strlen(sUtf8) + 1);
	if(sCopy)
	{
		strcpy(sCopy, sUtf8);
	}
	delete[] sUtf8;
	return sCopy;
}
char* PathUtil_Utf8ToAnsi(const char* sUtf8)
{
	if(sUtf8 == 0)
	{
		char* sEmpty = new char[1];
		sEmpty[0] = 0;
		return sEmpty;
	}
	int iWide = MultiByteToWideChar(CP_UTF8, 0, sUtf8, -1, 0, 0);
	if(iWide <= 0)
	{
		iWide = MultiByteToWideChar(CP_ACP, 0, sUtf8, -1, 0, 0);
		if(iWide <= 0)
		{
			char* sCopy = new char[strlen(sUtf8) + 1];
			strcpy(sCopy, sUtf8);
			return sCopy;
		}
		wchar_t* wsText = new wchar_t[iWide];
		MultiByteToWideChar(CP_ACP, 0, sUtf8, -1, wsText, iWide);
		char* sAnsi = new char[iWide * 2 + 1];
		int iAnsi = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, wsText, -1, sAnsi, iWide * 2 + 1, 0, 0);
		delete[] wsText;
		if(iAnsi <= 0)
		{
			delete[] sAnsi;
			sAnsi = new char[strlen(sUtf8) + 1];
			strcpy(sAnsi, sUtf8);
		}
		return sAnsi;
	}
	wchar_t* wsText = new wchar_t[iWide];
	MultiByteToWideChar(CP_UTF8, 0, sUtf8, -1, wsText, iWide);
	int iAnsi = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, wsText, -1, 0, 0, 0, 0);
	if(iAnsi <= 0)
	{
		delete[] wsText;
		char* sCopy = new char[strlen(sUtf8) + 1];
		strcpy(sCopy, sUtf8);
		return sCopy;
	}
	char* sAnsi = new char[iAnsi];
	WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, wsText, -1, sAnsi, iAnsi, 0, 0);
	delete[] wsText;
	return sAnsi;
}

#else // !_WIN32

wchar_t* PathUtil_Utf8ToWide(const char* sUtf8)
{
	size_t iLen = (sUtf8 ? strlen(sUtf8) : 0) + 1;
	wchar_t* wsPath = new wchar_t[iLen];
	if(sUtf8 == 0)
	{
		wsPath[0] = 0;
		return wsPath;
	}
	if(mbstowcs(wsPath, sUtf8, iLen) == (size_t)-1)
	{
		wsPath[0] = 0;
	}
	return wsPath;
}

char* PathUtil_WideToUtf8(const wchar_t* wsWide)
{
	size_t iLen = (wsWide ? wcslen(wsWide) : 0) * 4 + 1;
	char* sUtf8 = new char[iLen];
	if(wsWide == 0)
	{
		sUtf8[0] = 0;
		return sUtf8;
	}
	if(wcstombs(sUtf8, wsWide, iLen) == (size_t)-1)
	{
		sUtf8[0] = 0;
	}
	return sUtf8;
}

FILE* PathUtil_fopen(const char* sUtf8Path, const char* sMode) { return fopen(sUtf8Path, sMode); }
FILE* PathUtil_fopenW(const wchar_t* wsPath, const char* sMode)
{
	char* sPath = PathUtil_WideToUtf8(wsPath);
	FILE* f = fopen(sPath, sMode);
	delete[] sPath;
	return f;
}
int PathUtil_mkdir(const char* sUtf8Path) { return mkdir(sUtf8Path, 0755); }
int PathUtil_remove(const char* sUtf8Path) { return remove(sUtf8Path); }
bool PathUtil_moveReplace(const char* sSource, const char* sDestination) { return rename(sSource, sDestination) == 0; }
char* PathUtil_AnsiToUtf8(const char* sAnsi)
{
	char* sCopy = new char[(sAnsi ? strlen(sAnsi) : 0) + 1];
	strcpy(sCopy, sAnsi ? sAnsi : "");
	return sCopy;
}
char* PathUtil_AnsiToUtf8Malloc(const char* sAnsi)
{
	char* sUtf8 = PathUtil_AnsiToUtf8(sAnsi);
	char* sCopy = (char*)malloc(strlen(sUtf8) + 1);
	if(sCopy)
	{
		strcpy(sCopy, sUtf8);
	}
	delete[] sUtf8;
	return sCopy;
}
char* PathUtil_Utf8ToAnsi(const char* sUtf8)
{
	char* sCopy = new char[(sUtf8 ? strlen(sUtf8) : 0) + 1];
	strcpy(sCopy, sUtf8 ? sUtf8 : "");
	return sCopy;
}

#endif
