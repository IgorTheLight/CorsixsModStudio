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

#ifndef _PATHUTIL_H_
#define _PATHUTIL_H_

#include <stdio.h>

/*
	Paths that cross the Rainman API are UTF-8. The Windows file APIs need
	UTF-16, so every file system call converts at this boundary. That makes
	file and folder names in any language (Greek, Cyrillic, CJK, ...) work
	regardless of the system ANSI code page.
*/

wchar_t* PathUtil_Utf8ToWide(const char* sUtf8);
char* PathUtil_WideToUtf8(const wchar_t* wsWide);

FILE* PathUtil_fopen(const char* sUtf8Path, const char* sMode);
FILE* PathUtil_fopenW(const wchar_t* wsPath, const char* sMode);

int PathUtil_mkdir(const char* sUtf8Path);
int PathUtil_remove(const char* sUtf8Path);
bool PathUtil_moveReplace(const char* sUtf8Source, const char* sUtf8Destination);

/*! Converts text captured from a game file (ANSI) to the UTF-8 form used
    inside the program. Returns a new[] string the caller owns. */
char* PathUtil_AnsiToUtf8(const char* sAnsi);

/*! Like PathUtil_AnsiToUtf8, but allocated with malloc so existing code that
    frees module strings with free() keeps working. */
char* PathUtil_AnsiToUtf8Malloc(const char* sAnsi);

/*! Converts program text (UTF-8) back to the ANSI encoding game files use.
    Characters the code page cannot represent become '?'. New[] string. */
char* PathUtil_Utf8ToAnsi(const char* sUtf8);

#endif
