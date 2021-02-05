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

#ifndef _C_LUA_FROM_RGD_H_
#define _C_LUA_FROM_RGD_H_

#include "gnuc_defines.h"
#include "IFileStore.h"
#include "IMetaTable.h"
#include "CRgdHashTable.h"
#include "CLuaFile.h"
#include "CRgdFile.h"
#include "CModuleFile.h"
#include "Api.h"

RAINMAN_API void MakeLuaFromRgdQuickly(CRgdFile* pRgd, IFileStore::IOutputStream* pOut);

//! Creates a LUA source file from an RGD
/*!
	\param[in] pRgdIn The RGD to create a LUA from (Shouldn't be NULL)
	\param[in] pNilIn The NIL file which the RGD inherits from (Can be NULL)
	\param[in] pStore A filestore to read the Reference()d LUAs/RGDs from (Shouldn't be NULL)
	\param[in] pLuaOut The output stream for the created LUA
	\param[in] pUcsResolver The module class to use to resolve UCS values
	\return Returns no value, but throws a CRainmanException on error
*/
RAINMAN_API void MakeLuaFromRgdAndNil(CRgdFile* pRgdIn, CLuaFile* pNilIn, IFileStore* pStore, IFileStore::IOutputStream* pLuaOut, CModuleFile* pUcsResolver);

#endif

