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

#include "luax.h"
extern "C" {
#include <lauxlib.h>
#include <lualib.h>
};
#include <string.h>
#include "memdebug.h"
#include "Exception.h"

// Moves the GameData table to the top of the stack
void luax_GetGameData(lua_State* L)
{
	if(L == 0) throw new CRainmanException(__FILE__, __LINE__, "Invalid lua_State");
	lua_pushstring(L, "GameData");
	lua_gettable(L, LUA_GLOBALSINDEX);
	if(!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		throw new CRainmanException(__FILE__, __LINE__, "GameData is not table.");
	}
	return;
}

// Moves the MetaData table to the top of the stack
void luax_GetMetaData(lua_State* L)
{
	if(L == 0) throw new CRainmanException(__FILE__, __LINE__, "Invalid lua_State");
	lua_pushstring(L, "MetaData");
	lua_gettable(L, LUA_GLOBALSINDEX);
	if(!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		throw new CRainmanException(__FILE__, __LINE__, "MetaData is not table.");
	}
	return;
}

// Prints the _G table to file (doesn't print _G._G)
void luax_PrintGlobals(lua_State* L, const char* sFile, unsigned long iLevel, bool bSkipG)
{
	FILE* f;
	if(bSkipG)
	{
		lua_pushstring(L, "_G");
		lua_gettable(L, LUA_GLOBALSINDEX);
		f = fopen(sFile, "wb");
	}
	else
	{
		f = (FILE*)sFile;
	}

	lua_pushnil(L);  // first key
    while (lua_next(L, -2) != 0)
	{
		for(unsigned long i = 0; i < iLevel; ++i) fputs("  ", f);
		if(bSkipG && (lua_type(L, -2) == LUA_TSTRING) && (strcmp(lua_tostring(L, -2), "_G") == 0))
		{
		}
		else
		{
			switch(lua_type(L, -2))
			{
				case LUA_TNONE:
					fputs("(none)", f);
					break;

				case LUA_TNIL:
					fputs("(nil)", f);
					break;

				case LUA_TBOOLEAN:
					fputs(lua_toboolean(L, -2) ? "true" : "false", f);
					break;

				case LUA_TLIGHTUSERDATA:
					fputs("(light userdata)", f);
					break;

				case LUA_TNUMBER:
					fprintf(f, "%.2f", lua_tonumber(L, -2));
					break;

				case LUA_TSTRING:
					fprintf(f, "\"%s\"", lua_tostring(L, -2));
					break;

				case LUA_TTABLE:
					fputs("(table)", f);
					break;

				case LUA_TFUNCTION:
					fputs("(function)", f);
					break;

				case LUA_TUSERDATA:
					fputs("(userdata)", f);
					break;

				case LUA_TTHREAD:
					fputs("(thread)", f);
					break;
			}
			fputs(" = ", f);
			switch(lua_type(L, -1))
			{
				case LUA_TNONE:
					fputs("(none)", f);
					break;

				case LUA_TNIL:
					fputs("(nil)", f);
					break;

				case LUA_TBOOLEAN:
					fputs(lua_toboolean(L, -1) ? "true" : "false", f);
					break;

				case LUA_TLIGHTUSERDATA:
					fputs("(light userdata)", f);
					break;

				case LUA_TNUMBER:
					fprintf(f, "%.2f", lua_tonumber(L, -1));
					break;

				case LUA_TSTRING:
					fprintf(f, "\"%s\"", lua_tostring(L, -1));
					break;

				case LUA_TTABLE:
					fputs("{\n", f);
					luax_PrintGlobals(L, (const char*)f, iLevel + 1, false);
					for(unsigned long i = 0; i < iLevel; ++i) fputs("  ", f);
					fputs("}", f);
					break;

				case LUA_TFUNCTION:
					fputs("(function)", f);
					break;

				case LUA_TUSERDATA:
					fputs("(userdata)", f);
					break;

				case LUA_TTHREAD:
					fputs("(thread)", f);
					break;
			}
			fputs("\n", f);
		}
        lua_pop(L, 1);  // removes `value'; keeps `key' for next iteration
    }

	if(bSkipG)
	{
		lua_pop(L, 1); // stack preservation
		fclose(f);
	}
	return;
}

// Pops a key from the stack and then gets it as a table or makes a new table
void luax_GetOrMakeTable(lua_State* L, int iParentTable)
{
	lua_gettable(L, iParentTable);
	if(!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		lua_newtable(L);
	}
}

// Pops a key from the stack and then gets it raw as a table or makes a new table
void luax_GetRawOrMakeTable(lua_State* L, int iParentTable)
{
	lua_rawget(L, iParentTable);
	if(!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		lua_newtable(L);
	}
}

// Removes (sets to nil) a global variable
void luax_DeleteGlobal(lua_State* L, const char* sName)
{
	lua_pushstring(L, sName);
	lua_pushnil(L);
	lua_settable(L, LUA_GLOBALSINDEX);
}

/*
// Replaces the table at the top of the stack with its parent
int luax_GetParent(lua_State* L)
{
	// Convert the table to a pointer (light userdata)
	lua_topointer(L, -1);

	// Use the pointer as a key into the registry
	lua_gettable(L, LUA_REGISTRYINDEX);

	// Analyse the returned value
	if(lua_isnil(L, -1))
	{
		// No key (no parent stored)
		lua_pop(L, 1);
		return -1;
	}
	else if(lua_isuserdata(L, -1))
	{
		// Userdata is stored (global table)
		lua_pop(L, 1);
		return 1;
	}
	else if(!lua_istable(L, -1))
	{
		// Non-table is stored (problem)
		lua_pop(L, 1);
		return -2;
	}

	// All is good
	return 2;
}
*/

