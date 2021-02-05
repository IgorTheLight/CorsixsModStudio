/*
Rainman Library
Copyright (C) 2006 Corsix <corsix@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _RAINMAN_GNUC_DEFINES_H_
#define _RAINMAN_GNUC_DEFINES_H_
#ifdef __GNUC__

#define RAINMAN_GNUC

#define DWORD unsigned long
#define MAX_PATH 260
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define _open open
#define _O_BINARY 0
#define _O_RDONLY O_RDONLY
#define _stat stat
#define _fstat fstat
#define _close close
#define _mkdir(a) mkdir(a, 755)
#define _snprintf snprintf
#define _snwprintf swprintf
#define _vsnprintf vsnprintf

wchar_t* _ltow(long iVal, wchar_t* sStr, int iRadix);
wchar_t* _ultow(unsigned long iVal, wchar_t* sStr, int iRadix);
char* _ultoa(unsigned long iVal, char* sStr, int iRadix);

#else

#define RAINMAN_MSVC
#pragma warning(disable: 4251)

#endif

#define UNUSED(s) ( (s) )

#endif

