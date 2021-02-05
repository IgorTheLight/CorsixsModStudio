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

#ifndef _C_LUA_SCRIPT_H_
#define _C_LUA_SCRIPT_H_

#include "gnuc_defines.h"
extern "C" {
#include <lua.h>
};
#include "CLuaScript_Interface.h"
#include "Api.h"

class RAINMAN_API CLuaScript
{
public:
	CLuaScript(void);
	~CLuaScript(void);

	void Load(const char* sFile);
	void Execute();
	const char* GetLuaError(); // Obsolete?
protected:
	lua_State *m_pLua;
	char *m_sLuaError;

	void _Clean();
};

#endif

