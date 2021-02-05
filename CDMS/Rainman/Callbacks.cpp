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

#include "Callbacks.h"
#include <stdio.h>
#include <stdarg.h>
#include "Exception.h"

RAINMAN_API void CallCallback(CALLBACK_ARG, const char* sFormat, ...)
{
	/*
		Uses _vsnprintf to create the formatted string
		Doubles the buffer size and tries again if the buffer allocated was too small (begins with a 256 byte buffer)
		This way most messages make do with 256, and bigger ones will quickly get up to the buffer size needed and also always be using at least half of the buffer
	*/

	// Check if a callback function was passed (if one wasn't then it's not an error - the application may not want callbacks)
	if(fnStatusCallback)
	{
		size_t iSize = 256;
		va_list marker;
		char* sBuffer = 0;
		do
		{
			if(iSize < 256) // If iSize is less than 256 then it must have exceeded its maximum value and looped back round again -> error
			{
				va_end(marker);
				delete[] sBuffer;
				throw new CRainmanException(__FILE__, __LINE__, "Message too large");
			}
			if(iSize != 256)
			{
				va_end(marker);
				delete[] sBuffer;
			}
			sBuffer = CHECK_MEM(new char[iSize <<= 1]);
			va_start(marker, sFormat);
		} while(_vsnprintf(sBuffer, iSize - 1, sFormat, marker) == -1); // -1 is the return value for "bigger buffer needed"
		va_end(marker);
		fnStatusCallback(sBuffer, fnStatusCallbackTag);
		delete[] sBuffer;
	}
}

