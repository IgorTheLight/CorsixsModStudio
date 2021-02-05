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

#ifndef _C_LUAFILE_CACHE_H_
#define _C_LUAFILE_CACHE_H_
#include "gnuc_defines.h"
extern "C" {
#include <lua.h>
};
#include "Api.h"

//! Cache for lua_State objects
/*!
	
*/
class RAINMAN_API CLuaFileCache
{
public:
	CLuaFileCache(void);
	~CLuaFileCache(void);
	void Clear();

	//! Make a new lua state
	/*!
		Creates a new lua state that will be destroyed when
		this object is destroyed.
	*/
	lua_State* MakeState();

	//! Free a lua state
	void FreeState(lua_State* L);

	//! Assign a name to a state
	/*!
		State must have been created with MakeState()
	*/
	void AddToCache(const char* sName, lua_State* L);

	//! Find a state by name
	lua_State* Fetch(const char* sName);

	//! Copy a GameData table from a chached file to a lua state stack
	/*!
		L must be a state created with MakeState()
	*/
	void GameDataToStack(const char* sRef, lua_State* L);

	//! Copy a MetaData table from a chached file to a lua state stack
	/*!
		L must be a state created with MakeState()
	*/
	void MetaDataToStack(const char* sRef, lua_State* L);

protected:
	lua_State* m_pMother;

	struct _tEntry
	{
		_tEntry(char*, lua_State*, bool b = false);
		lua_State* L;
		char* sName;
		_tEntry* pNext;
		bool bUseful;
	};

	_tEntry m_oEntires, *m_pEntriesEnd;
};

#endif

