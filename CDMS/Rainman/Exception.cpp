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

#include "Exception.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include "memdebug.h"

CRainmanException::CRainmanException() {}

CRainmanException::CRainmanException(const char* sFile, unsigned long iLine, const char* sMessage, CRainmanException* pPrecursor)
: m_sFile(sFile), m_iLine(iLine), m_pPrecursor(pPrecursor)
{
	m_sMessage = strdup(sMessage);
}

CRainmanException::CRainmanException(CRainmanException* pPrecursor, const char* sFile, unsigned long iLine, const char* sFormat, ...)
: m_sFile(sFile), m_iLine(iLine), m_pPrecursor(pPrecursor)
{
	size_t iL = 128;
	va_list marker;
	while(1)
	{
		char* sBuf = (char*)malloc(iL);
		va_start(marker, sFormat);
		if(_vsnprintf(sBuf, iL - 1, sFormat, marker) == -1)
		{
			va_end(marker);
			free(sBuf);
			iL <<= 1;
		}
		else
		{
			va_end(marker);
			m_sMessage = sBuf;
			return;
		}
	}
}

CRainmanException::~CRainmanException() {}

void CRainmanException::destroy()
{
	free(m_sMessage);
	if(m_pPrecursor) m_pPrecursor->destroy();
	delete this;
}
