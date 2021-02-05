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

// This file is included in every .cpp file to allow easier memory leak tracing in debug mode

#ifndef _MEM_DEBUG_H_
#define _MEM_DEBUG_H_

#include "gnuc_defines.h"

#ifndef RAINMAN_GNUC
#ifdef _DEBUG
	#include <crtdbg.h>
	#include <memory.h>
	#include <string.h>
	#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#define strdup(s) strdup_memdebug(s, __FILE__, __LINE__)
	static char* strdup_memdebug(const char* s, const char* sFile, unsigned long iLine)
	{
		size_t iL = strlen(s);
		char* sR = new(_NORMAL_BLOCK, sFile, iLine) char[iL + 1];
		memcpy((void*)sR, (const void*)s, iL);
		sR[iL] = 0;
		return sR;
	}
#else
	#define DEBUG_NEW new
#endif

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif
#endif
#endif

