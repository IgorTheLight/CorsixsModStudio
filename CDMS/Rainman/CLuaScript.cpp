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

#include "CLuaScript.h"
#include "CDoWModule.h"

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
};

#include "memdebug.h"
#include "Exception.h"
#include "string.h"
#include "stdlib.h"

CLuaScript::CLuaScript(void)
{
	m_pLua = 0;
	m_sLuaError = 0;
}

CLuaScript::~CLuaScript(void)
{
	_Clean();
}

void CLuaScript::Load(const char* sFile)
{
	_Clean();
	m_pLua = lua_open();
	if(!m_pLua)
	{
		m_sLuaError = strdup("Unable to create lua state");
		throw new CRainmanException(__FILE__, __LINE__, "Unable to create Lua state");
	}

	if(luaL_loadfile(m_pLua, sFile))
	{
		char *sLuaError = strdup(lua_tostring(m_pLua, -1));
		_Clean();
		m_sLuaError = sLuaError;
		throw new CRainmanException(0, __FILE__, __LINE__, "Lua error: %s", sLuaError);
	}

	//LuaBind_Globals(m_pLua);
}

void CLuaScript::Execute()
{
	if(!m_pLua)
	{
		m_sLuaError = strdup("No lua state");
		throw new CRainmanException(__FILE__, __LINE__, "No state");
	}

	// Lua standard libraries
	luaopen_base(m_pLua);
    luaopen_table(m_pLua);
    luaopen_string(m_pLua);
    luaopen_math(m_pLua);

	// Run the code
	if(lua_pcall(m_pLua, 0, 0, 0))
	{
		switch(lua_type(m_pLua, -1))
		{
		case LUA_TSTRING:
			{
				char *sLuaError = strdup(lua_tostring(m_pLua, -1));
				_Clean();
				m_sLuaError = sLuaError;
				throw new CRainmanException(0, __FILE__, __LINE__, "Lua error: %s", sLuaError);
			}
		case LUA_TLIGHTUSERDATA:
			{
				CRainmanException* pE = (CRainmanException*) lua_touserdata(m_pLua, -1);
				char *sLuaError = strdup(pE->getMessage());
				_Clean();
				m_sLuaError = sLuaError;
				throw new CRainmanException(pE, __FILE__, __LINE__, "Lua error");
			}
		default:
			{
				char *sLuaError = strdup("Unknown error");
				_Clean();
				m_sLuaError = sLuaError;
				throw new CRainmanException(0, __FILE__, __LINE__, "Lua unknown error");
			}
		};
	}

	// End
}

const char* CLuaScript::GetLuaError()
{
	return m_sLuaError;
}

void CLuaScript::_Clean()
{
	if(m_sLuaError)
	{
		free(m_sLuaError);
		m_sLuaError = 0;
	}
	if(m_pLua)
	{
		lua_close(m_pLua);
		m_pLua = 0;
	}
}

