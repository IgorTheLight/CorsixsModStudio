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

#include "CLuaFileCache.h"
#include "memdebug.h"
#include "luax.h"
#include "stdlib.h"
#include "string.h"
#include "Exception.h"
extern "C" {
#include <lauxlib.h>
};

CLuaFileCache::CLuaFileCache(void) : m_oEntires(0,0)
{
	m_pMother = lua_open();
	m_pEntriesEnd = &m_oEntires;
}

CLuaFileCache::_tEntry::_tEntry(char* s, lua_State* L, bool b) : sName(s), L(L), pNext(0), bUseful(b) {}

lua_State* CLuaFileCache::MakeState()
{
	lua_State* L = lua_newthread(m_pMother); // {L -1}
	lua_newtable(L); // {L -2} {T -1}
	lua_replace(L, LUA_GLOBALSINDEX); // {L -1}
	lua_pushlightuserdata(m_pMother, (void*)L); // {L -2} {U -1}
	lua_insert(m_pMother, -2); // {U -2} {L -1}
	lua_settable(m_pMother, LUA_REGISTRYINDEX); //

	m_pEntriesEnd = m_pEntriesEnd->pNext = new _tEntry(0, L);
	return L;
}

void CLuaFileCache::FreeState(lua_State* L)
{
	_tEntry* pItr = m_oEntires.pNext, *pPrev = &m_oEntires;
	while(pItr)
	{
		if(pItr->L == L)
		{
			if(pItr->bUseful) throw new CRainmanException(__FILE__, __LINE__, "Cannot delete state as it is useful");
			if((pPrev->pNext = pItr->pNext) == 0) m_pEntriesEnd = pPrev;
			break;
		}
		pPrev = pItr;
		pItr = pItr->pNext;
	}
	if(!pItr) throw new CRainmanException(__FILE__, __LINE__, "State not found");
	lua_pushlightuserdata(m_pMother, (void*)L); // {U -1}
	lua_pushnil(m_pMother); // {U -2} {N -1}
	lua_settable(m_pMother, LUA_REGISTRYINDEX); //
	#ifdef LUA_GCCOLLECT
	lua_gc(m_pMother, LUA_GCCOLLECT, 0);
	#else
	lua_setgcthreshold(m_pMother, 0);
	#endif
}

void CLuaFileCache::Clear()
{
	_tEntry* pItr = m_oEntires.pNext, *pTmp;
	while(pItr)
	{
		if(pItr->sName) free(pItr->sName);
		pTmp = pItr;
		pItr = pItr->pNext;
		delete pTmp;
	}
	m_oEntires.pNext = 0;
	m_pEntriesEnd = &m_oEntires;
	lua_close(m_pMother);
	m_pMother = lua_open();
}

CLuaFileCache::~CLuaFileCache(void)
{
	Clear();
	lua_close(m_pMother);
}

void CLuaFileCache::AddToCache(const char* sName, lua_State* L)
{
	_tEntry* pItr = m_oEntires.pNext;
	while(pItr)
	{
		if(pItr->L == L)
		{
			if(pItr->sName) free(pItr->sName);
			pItr->sName = strdup(sName);
			return;
		}
		pItr = pItr->pNext;
	}
	throw new CRainmanException(__FILE__, __LINE__, "State not found");
}

lua_State* CLuaFileCache::Fetch(const char* sName)
{
	_tEntry* pItr = m_oEntires.pNext;
	while(pItr)
	{
		if(pItr->sName && stricmp(pItr->sName,sName) == 0)
		{
			pItr->bUseful = true;
			return pItr->L;
		}
		pItr = pItr->pNext;
	}
	return 0;
}

void CLuaFileCache::GameDataToStack(const char* sRef, lua_State* L)
{
	lua_State *Ls = Fetch(sRef);
	if(!Ls) throw new CRainmanException(0, __FILE__, __LINE__, "No state found with name \'%s\'", sRef);
	lua_pushstring(Ls, "GameData");
	lua_gettable(Ls, LUA_GLOBALSINDEX);
	lua_pushvalue(Ls, -1);
	lua_xmove(Ls, L, 1);
	lua_pop(Ls, 1);
	return;
}

void CLuaFileCache::MetaDataToStack(const char* sRef, lua_State* L)
{
	lua_State *Ls = Fetch(sRef);
	if(!Ls) throw new CRainmanException(0, __FILE__, __LINE__, "No state found with name \'%s\'", sRef);
	lua_pushstring(Ls, "MetaData");
	lua_gettable(Ls, LUA_GLOBALSINDEX);
	lua_xmove(Ls, L, 1);
	return;
}

