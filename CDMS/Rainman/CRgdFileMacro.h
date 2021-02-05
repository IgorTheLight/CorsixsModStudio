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

#ifndef _C_RGD_FILE_MACRO_H_
#define _C_RGD_FILE_MACRO_H_

extern "C" {
#include "Lua51.h"
};
#include "CRgdFile.h"
#include "CModuleFile.h"

class RAINMAN_API CRgdFileMacro
{
public:
	CRgdFileMacro();
	~CRgdFileMacro();

	enum eSecurityTypes
	{
		ST_DebugLib,
		ST_IOLib,
		ST_OSLib,
	};

	void setUcsResolver		(CModuleFile* pModule);
	void setIsDowMod		(bool bIsDowMod);
	void setHashTable		(CRgdHashTable* pTable);
	void setCallbackPrint	(void(*pFunction)(void*,const char*));
	void setCallbackSave	(bool(*pFunction)(void*,const char*));
	void setCallbackLoad	(bool(*pFunction)(void*,const char*));
	void setCallbackSecurity(bool(*pFunction)(void*,eSecurityTypes));
	void setCallbackTag		(void* pTag);

	void loadMacro	(const char* sCode);
	void runMacro	(const char* sFile, IFileStore* pStore);
	void runAtEnd	();
	void unloadMacro();

protected:
	static CRgdFile::_RgdEntry* _getRootEntry(CRgdFile* pFile);
	lua_State* m_pL;
	CRgdHashTable* m_pHashTableToUse;
	CModuleFile* m_pUcsResolverModule;
	bool m_bIsDowMod;
	void* m_pCallbackTag;
	void (*m_fpPrintCallback)(void* pTag, const char* sMsg);
	bool (*m_fpOnSaveCallback)(void* pTag, const char* sFile);
	bool (*m_fpOnLoadCallback)(void* pTag, const char* sFile);
	bool (*m_fpOnSecurityCallback)(void* pTag, eSecurityTypes eType);

	void _cleanLua();
	void _onPrint(const char* sMsg);
	bool _onSave(const char* sFile);
	bool _onLoad(const char* sFile);

	struct _tUserData
	{
		unsigned long iMagic; CRgdFileMacro* pMacro;
		template <class T> static T* _fromLua(lua_State*L, int i)
		{
			if( (lua51_type(L, i) == LUA_TTABLE) && lua51_getmetatable(L, i))
			{
				lua51_pushstring(L, "userdata");
				lua51_rawget(L, -2);
				if( lua51_type(L, -1) == LUA_TUSERDATA )
				{
					lua51_replace(L, i);
					lua51_pop(L, 1);
				}
				else
				{
					lua51_pop(L, 2);
				}
			}
			_tUserData* p = (_tUserData*)lua51_touserdata(L,i);
			if(p == 0 || p->iMagic != T::kMagic)
				return 0;
			return (T*)p;
		}
		template <class T> static T* _fromLuaLight(lua_State*L, int i){T* p = (T*)lua51_touserdata(L,i); return p;}
	};
	struct _tCRgdFile : public _tUserData
	{
		static _tCRgdFile* construct(lua_State* L, CRgdFileMacro* _pMacro, const char* _sFile, IFileStore* _pStore);
		void construct(CRgdFileMacro* _pMacro, const char* _sFile, IFileStore* _pStore);
		void _loadRgd();

		static const unsigned long kMagic = 'RGDF';
		CRgdFile* pRgd;
		char* sFile;
		IFileStore* pStore;

		static int luaf_save(lua_State* L);
		static int luaf_saveAs(lua_State* L);
		static int luaf_GET(lua_State* L);
		static int luaf_SET(lua_State* L);
		static int luae_gc(lua_State* L);
		static int luae_newindex(lua_State* L);
		static int luae_setindex(lua_State* L);
		static int luae_delayload(lua_State* L);
	};
	struct _tCRgdTable : public _tUserData
	{
		static _tCRgdTable* construct(lua_State* L, _tCRgdFile* pRgdFile, CRgdFile::_RgdEntry* pRgdData);
		void construct(_tCRgdFile* pRgdFile, CRgdFile::_RgdEntry* pRgdData);

		static const unsigned long kMagic = 'RGDT';
		CRgdFile::_RgdEntry* pRgdData;
		_tCRgdFile* pRgdFile;
		bool bLoaded;

		static int luaf_GET(lua_State* L);
		static int luaf_SET(lua_State* L);
		static int luae_newindex(lua_State* L);
		static int luae_setindex(lua_State* L);
		static int luaf_construct(lua_State* L);
		static int luae_delayload(lua_State* L);
		static int luap_setself(lua_State* L);
	};

	static int luaf_protect0(lua_State* L);
	static int luaf_protect1(lua_State* L);
	static int luaf_protect2(lua_State* L);
	static int luaf_ucs		(lua_State* L);
	static int luaf_loadrgd	(lua_State* L);
	static int luaf_print	(lua_State* L);
	static int luaf_pcall	(lua_State* L);
	static int luaf_xpcall	(lua_State* L);
	static int luaf_xpcalle (lua_State* L);
	static int luaf_next	(lua_State *L);
	static int luaf_pairs	(lua_State *L);
	static int luaf_table	(lua_State *L);
	static int luaf_tfilter	(lua_State *L);
	static int luaf_formatme(lua_State *L);
	static int luae_call	(lua_State *L);

	static void _lua_udat_to_table(lua_State *L, int i);
};

#endif