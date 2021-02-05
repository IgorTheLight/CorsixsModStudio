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

#ifndef _WRITETIME_H_
#define _WRITETIME_H_

#include "gnuc_defines.h"

//! A typemapping for write time
/*!
	This implementation of write time uses the number of seconds since the unix epoch.
	The application should not depend upon the format of tLastWriteTime and must use the functions in this file.
	If you change the format of tLastWriteTime, you should update CSgaFile
	\sa GetLastWriteTime() IsValidWriteTime() GetInvalidWriteTime() IsWriteTimeNewer()
*/
typedef unsigned long long tLastWriteTime;

//! Gets the last write time of a file on harddisk
/*!
	\param[in] sFile The name of a file on harddisk (eg. can be opened with _open() )
	\return Returns a valid write time or throws a CRainmanException
*/
tLastWriteTime GetLastWriteTime(const char* sFile);

//! Verifies whether a tLastWriteTime is valid or not
/*!
	\deprecated
		This function should no longer be needed as a CRainmanException should be thrown instead of returning an invalid time.
		As such, this function may be removed in future builds.
	\return Returns true if oTime is a valid write time
*/
inline bool IsValidWriteTime(tLastWriteTime oTime)
{
	return oTime != 0;
}

inline tLastWriteTime GetInvalidWriteTime()
{
	return 0;
}

/*!
	Returns true if A is newer than B
	Otherwise, returns false
*/
inline bool IsWriteTimeNewer(tLastWriteTime oA, tLastWriteTime oB)
{
	return (oA > oB);
}

#endif

