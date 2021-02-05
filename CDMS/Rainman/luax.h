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

#ifndef _LUA_X_H_
#define _LUA_X_H_
#include "gnuc_defines.h"
extern "C" {
#include <lua.h>
};

// string.split(s [, delimiter [, plain]])
// returns a table containing parts of 's', so that
// s == table.concat(string.split(s, delim, true), delim)
int luax51_string_split(lua_State* L);

// math.clamp(n [, lower [, upper]])
// equivalent to:
// if upper and n > upper then return upper end
// if lower and n < lower then return lower end
// return n
int luax51_math_clamp(lua_State* L);

// string.after(s [, delimiter])
int luax51_string_after(lua_State* L);

// string.afterlast(s [, delimiter])
int luax51_string_afterlast(lua_State* L);

// string.before(s [, delimiter])
int luax51_string_before(lua_State* L);

// string.beforelast(s [, delimiter])
int luax51_string_beforelast(lua_State* L);

//! Moves the GameData table to the top of the stack
void luax_GetGameData(lua_State* L);

//! Moves the MetaData table to the top of the stack
void luax_GetMetaData(lua_State* L);

//! Prints the _G table to file (doesn't print _G._G)
/*!
	\param[in] L The lua_State to print _G from
	\param[in] sFile The output file name
	\param[in] iLevel (Leave as default value)
	\param[in] bSkipG (Leave as default value)
*/
void luax_PrintGlobals(lua_State* L, const char* sFile,
					   /* Do NOT pass in these */ unsigned long iLevel = 0, bool bSkipG = true);

//! Pops a key from the stack and then gets it as a table or makes a new table
void luax_GetOrMakeTable(lua_State* L, int iParentTable);

//! Pops a key from the stack and then gets it raw as a table or makes a new table
void luax_GetRawOrMakeTable(lua_State* L, int iParentTable);

//! Removes (sets to nil) a global variable
void luax_DeleteGlobal(lua_State* L, const char* sName);

template <class T>
T* luax_newuserdata(lua_State* L) {return (T*)lua_newuserdata(L, sizeof(T));}

template <class T>
T* lua51x_newuserdata(lua_State* L) {return (T*)lua51_newuserdata(L, sizeof(T));}

/*
// Replaces the table at the top of the stack with its parent
	Return values:
	 2 = everything went swimmingly
	 1 = parent is _G
	-1 = problem
	-2 = problem

	If return is 2 then the table is popped from the stack, and
	it's parent is pushed onto the stack. Otherwise, the table
	is popped off the stack and nothing is pushed back on.
int luax_GetParent(lua_State* L);
*/

#endif

