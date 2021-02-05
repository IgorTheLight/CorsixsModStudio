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
#include "gnuc_defines.h"

#ifdef RAINMAN_GNUC
#include <cmath>
#include <algorithm>

wchar_t* _ltow(long iVal, wchar_t* sStr, int iRadix)
{
	if (iRadix < 2 || iRadix > 16) { *sStr = 0; return sStr; }
	wchar_t* out = sStr;
	long quotient = iVal;
	long base = (long)iRadix;
	do {
		*out = "0123456789abcdef"[ std::abs( quotient % base ) ];
		++out;
		quotient /= base;
	
	} while ( quotient );
	if ( iVal < 0 && base == 10) *out++ = '-';
	
	std::reverse( sStr, out );
	
	*out = 0;
	
	return sStr;
}

wchar_t* _ultow(unsigned long iVal, wchar_t* sStr, int iRadix)
{
	if (iRadix < 2 || iRadix > 16) { *sStr = 0; return sStr; }
	wchar_t* out = sStr;
	unsigned long quotient = iVal;
	unsigned long base = (unsigned long)iRadix;
	do {
		*out = "0123456789abcdef"[ std::abs( (long)(quotient % base) ) ];
		++out;
		quotient /= base;
	
	} while ( quotient );
	if ( iVal < 0 && base == 10) *out++ = '-';
	
	std::reverse( sStr, out );
	
	*out = 0;
	
	return sStr;
}

char* _ultoa(unsigned long iVal, char* sStr, int iRadix)
{
	if (iRadix < 2 || iRadix > 16) { *sStr = 0; return sStr; }
	char* out = sStr;
	unsigned long quotient = iVal;
	unsigned long base = (unsigned long)iRadix;
	do {
		*out = "0123456789abcdef"[ std::abs( (long)(quotient % base) ) ];
		++out;
		quotient /= base;
	
	} while ( quotient );
	if ( iVal < 0 && base == 10) *out++ = '-';
	
	std::reverse( sStr, out );
	
	*out = 0;
	
	return sStr;
}

#endif

