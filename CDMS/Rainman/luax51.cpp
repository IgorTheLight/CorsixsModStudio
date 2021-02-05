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

extern "C" {
#include "Lua51.h"
#include "Lua51Aux.h"
};
#include <string.h>
#include "memdebug.h"
#include "Exception.h"

// string.split(s [, delimiter [, plain]])
int luax51_string_split(lua_State* L)
{
	const int kArgNone = 0;
	const int kArgString = 1;
	const int kArgDelimiter = 2;
	const int kArgPlain = 3;
	const int kStringFind = 4;
	const int kResultTable = 5;
	const int kRetStart = 6;
	const int kRetEnd = 7;

	// Force to 3 arguments
	switch(lua51_gettop(L))
	{
	case kArgNone:
		lua51_pushstring(L, "string.split expected string as first parameter, got nil");
		lua51_error(L);
	case kArgString:
		lua51_pushstring(L, " ");
	case kArgDelimiter:
		lua51_pushboolean(L, 0);
	default:
		lua51_settop(L, 3);
		if(lua51_tostring(L, kArgString) == 0)
		{
			lua51_pushstring(L, "string.split expected string as first parameter");
			lua51_error(L);
		}
		if(lua51_tostring(L, kArgDelimiter) == 0)
		{
			lua51_pushstring(L, "string.split expected string as second parameter");
			lua51_error(L);
		}
	};

	// Put string.find onto stack stack
	lua51_pushstring(L, "string");
	lua51_gettable(L, LUA_GLOBALSINDEX);
	lua51_pushstring(L, "find");
	lua51_gettable(L, kStringFind);
	lua51_replace(L, kStringFind);

	// Put new table onto stack
	lua51_newtable(L);
	int iTableSize = 0;

	// Put initial arguments onto stack for string.find
	size_t iInitPos = 1;
	lua51_pushvalue(L, kStringFind);
	lua51_pushvalue(L, kArgString);
	lua51_pushvalue(L, kArgDelimiter);
	lua51_pushnumber(L, 1);
	lua51_pushvalue(L, kArgPlain);

	// Call string.find
	lua51_call(L, 4, 2);

	while(lua51_type(L, kRetStart) != LUA_TNIL)
	{
		const char* sArgString = lua51_tostring(L, kArgString);
		size_t iStart = (size_t)lua51_tonumber(L, kRetStart);
		size_t iEnd = (size_t)lua51_tonumber(L, kRetStart);

		lua51_pushlstring(L, lua51_tostring(L, kArgString) + iInitPos - 1, ((size_t)lua51_tonumber(L, kRetStart)) - iInitPos);
		lua51_rawseti(L, kResultTable, ++iTableSize);
		iInitPos = ((size_t)lua51_tonumber(L, kRetEnd)) + 1;

		lua51_pop(L, 2);

		lua51_pushvalue(L, kStringFind);
		lua51_pushvalue(L, kArgString);
		lua51_pushvalue(L, kArgDelimiter);
		lua51_pushnumber(L, (lua_Number)iInitPos);
		lua51_pushvalue(L, kArgPlain);
		lua51_call(L, 4, 2);
	}

	lua51_pop(L, 2);
	lua51_pushlstring(L, lua51_tostring(L, kArgString) + iInitPos - 1, lua51_strlen(L, kArgString) - iInitPos + 1);
	lua51_rawseti(L, kResultTable, ++iTableSize);
	return 1;
}

int luax51_math_clamp(lua_State* L)
{
	const int kArgNone = 0;
	const int kArgN = 1;
	const int kArgMin = 2;
	const int kArgMax = 3;

	switch(lua51_gettop(L))
	{
	default:
		if(lua51_tonumber(L, kArgN) > lua51_tonumber(L, kArgMax))
		{
			lua51_settop(L, kArgMax);
			return 1;
		}

	case kArgMin:
		if(lua51_tonumber(L, kArgN) < lua51_tonumber(L, kArgMin))
		{
			lua51_settop(L, kArgMin);
			return 1;
		}

	case kArgN:
		lua51_settop(L, kArgN);
		return 1;

	case kArgNone:
		return 0;
	}
}

int luax51_string_afterlast(lua_State* L)
{
	const int kArgNone = 0;
	const int kArgString = 1;
	const int kArgDelimiter = 2;

	size_t iArgStringLen;
	const char* sArgString = lua51L_checklstring(L, kArgString, &iArgStringLen);

	lua51_settop(L, kArgDelimiter);
	if(lua51_type(L, kArgDelimiter) == LUA_TNIL)
	{
		lua51_pushstring(L, "");
		return 1;
	}

	size_t iArgDelimLen;
	const char* sArgDelim = lua51L_checklstring(L, kArgDelimiter, &iArgDelimLen);

	if(iArgDelimLen >= iArgStringLen)
	{
		lua51_pushstring(L, "");
		return 1;
	}

	for(size_t iOffset = iArgStringLen - iArgDelimLen + 1; iOffset;)
	{
		--iOffset;
		if (memcmp(sArgString + iOffset, sArgDelim, iArgDelimLen) == 0)
		{
			lua51_pushlstring(L, sArgString + iOffset + iArgDelimLen, iArgStringLen - iOffset - iArgDelimLen);
			return 1;
		}
	}

	lua51_settop(L, kArgString);
	return 1;
}

int luax51_string_beforelast(lua_State* L)
{
	const int kArgNone = 0;
	const int kArgString = 1;
	const int kArgDelimiter = 2;

	size_t iArgStringLen;
	const char* sArgString = lua51L_checklstring(L, kArgString, &iArgStringLen);

	lua51_settop(L, kArgDelimiter);
	if(lua51_type(L, kArgDelimiter) == LUA_TNIL)
	{
		lua51_settop(L, kArgString);
		return 1;
	}

	size_t iArgDelimLen;
	const char* sArgDelim = lua51L_checklstring(L, kArgDelimiter, &iArgDelimLen);

	if(iArgDelimLen >= iArgStringLen)
	{
		lua51_pushstring(L, "");
		return 1;
	}

	for(size_t iOffset = iArgStringLen - iArgDelimLen + 1; iOffset;)
	{
		--iOffset;
		if (memcmp(sArgString + iOffset, sArgDelim, iArgDelimLen) == 0)
		{
			lua51_pushlstring(L, sArgString, iOffset);
			return 1;
		}
	}

	lua51_pushstring(L, "");
		return 1;
}

int luax51_string_after(lua_State* L)
{
	const int kArgNone = 0;
	const int kArgString = 1;
	const int kArgDelimiter = 2;
	const int kStringFind = 3;
	const int kEndPos = 2;

	size_t iArgStringLen;
	const char* sArgString = lua51L_checklstring(L, kArgString, &iArgStringLen);

	if (lua51_gettop(L) == kArgString) return 1;

	lua51_settop(L, kArgDelimiter);
	if(lua51_type(L, kArgDelimiter) == LUA_TNIL)
	{
		lua51_settop(L, kArgString);
		return 1;
	}
	lua51L_checklstring(L, kArgDelimiter, 0);

	lua51_getglobal(L, "string");
	lua51_pushstring(L, "find");
	lua51_gettable(L, kStringFind);
	lua51_replace(L, kStringFind);
	lua51_insert(L, 1);
	lua51_pushnumber(L, 1);
	lua51_pushboolean(L, 1);
	lua51_call(L, 4, 2);

	if(lua51_type(L, kEndPos) != LUA_TNUMBER)
	{
		lua51_pushstring(L, "");
		return 1;
	}

	size_t iEndPos = (size_t)lua51_tonumber(L, kEndPos);

	lua51_pushlstring(L, sArgString + iEndPos, iArgStringLen - iEndPos);
	return 1;
}

int luax51_string_before(lua_State* L)
{
	const int kArgNone = 0;
	const int kArgString = 1;
	const int kArgDelimiter = 2;
	const int kStringFind = 3;
	const int kStartPos = 1;

	size_t iArgStringLen;
	const char* sArgString = lua51L_checklstring(L, kArgString, &iArgStringLen);

	if (lua51_gettop(L) == kArgString)
	{
		lua51_pushstring(L, "");
		return 1;
	}

	lua51_settop(L, kArgDelimiter);
	if(lua51_type(L, kArgDelimiter) == LUA_TNIL)
	{
		lua51_pushstring(L, "");
		return 1;
	}
	lua51L_checklstring(L, kArgDelimiter, 0);

	lua51_getglobal(L, "string");
	lua51_pushstring(L, "find");
	lua51_gettable(L, kStringFind);
	lua51_replace(L, kStringFind);
	lua51_insert(L, 1);
	lua51_pushnumber(L, 1);
	lua51_pushboolean(L, 1);
	lua51_call(L, 4, 2);

	if(lua51_type(L, kStartPos) != LUA_TNUMBER)
	{
		lua51_pushlstring(L, sArgString, iArgStringLen);
		return 1;
	}

	size_t iStartPos = (size_t)lua51_tonumber(L, kStartPos);

	lua51_pushlstring(L, sArgString, iStartPos - 1);
	return 1;
}