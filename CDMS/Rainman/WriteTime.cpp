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

#include "WriteTime.h"
#ifndef RAINMAN_GNUC
#include <io.h>
#else
#include <errno.h>
#include <unistd.h>
#endif
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Exception.h"

tLastWriteTime GetLastWriteTime(const char* sFile)
{
	/*
		Opens the file using the lowest level calls (for speed)
		Gets alot of information using a low level call
		Closes the file

		If a better way exists to just get the write time, then please, go ahead...
	*/

	// Open the file at a low level
	int iH = _open(sFile, _O_BINARY | _O_RDONLY);
	if(iH == -1) throw new CRainmanException(0, __FILE__, __LINE__, "_open gave \'%s\' opening \'%s\'", strerror(errno), sFile);
	struct _stat s;

	// Get the information
	if(_fstat(iH, &s) == -1)
	{
		_close(iH);
		throw new CRainmanException(0, __FILE__, __LINE__, "_fstat gave \'%s\' on \'%s\'", strerror(errno), sFile);
	}

	// Clean up and return
	_close(iH);
	return s.st_mtime;
}

