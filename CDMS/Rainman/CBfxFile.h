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

#ifndef _BFX_FILE_H_
#define _BFX_FILE_H_

#include "CRgdFile.h"
#include <lua.h>

class RAINMAN_API CBfxFile : public CRgdFile
{
public:
	CBfxFile();
	void SaveAsBfxLua(IFileStore::IOutputStream *pStream, lua_State *Lmap);

protected:
	void _SaveRaw(IFileStore::IOutputStream *pStream, _RgdEntry *pSource, lua_State *Lmap, bool bNumericID = false, size_t iIndentLvl = 0);
};

#endif