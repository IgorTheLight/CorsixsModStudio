#include "CRgdFileMacro.h"
extern "C" {
#include "Lua51Aux.h"
};
#include "luax.h"
#include "Exception.h"
#include "Internal_Util.h"
#include "memdebug.h"

#define LuaArgMsg(type,fn,line_) "Expected " type " as first argument to " fn ". Did you do obj." fn " instead of obj:" fn " ?"

CRgdFileMacro::CRgdFileMacro()
{
	m_pL = 0;
	m_pHashTableToUse = 0;
	m_pCallbackTag = 0;
	m_fpPrintCallback = 0;
	m_fpOnSaveCallback = 0;
	m_pUcsResolverModule = 0;
	m_bIsDowMod = true;
}

CRgdFileMacro::~CRgdFileMacro()
{
	_cleanLua();
}

void CRgdFileMacro::_cleanLua()
{
	if(m_pL)
	{
		lua51_close(m_pL);
		m_pL = 0;
	}
}

void CRgdFileMacro::_tCRgdFile::construct(CRgdFileMacro* _pMacro, const char* _sFile, IFileStore* _pStore)
{
	iMagic = kMagic;
	pMacro = _pMacro;
	sFile = strdup(_sFile);
	pStore = _pStore;
	pRgd = 0;
}

int CRgdFileMacro::_tCRgdFile::luaf_save(lua_State* L)
{
	_tCRgdFile *self = _fromLua<_tCRgdFile>(L, 1);
	if(self == 0)
	{
		lua51_pushstring(L, LuaArgMsg("CRgdFile","save",__LINE__));
		lua51_error(L);
	}
	lua51_pushstring(L, self->sFile);
	return luaf_saveAs(L);
}

int CRgdFileMacro::_tCRgdFile::luaf_saveAs(lua_State* L)
{
	_tCRgdFile *self = _fromLua<_tCRgdFile>(L, 1);
	if(self == 0)
	{
		lua51_pushstring(L, LuaArgMsg("CRgdFile","saveAs",__LINE__));
		lua51_error(L);
	}

	if(self->pRgd == 0)
	{
		try {self->_loadRgd();}
		catch(CRainmanException *pE){
			lua51_pushlightuserdata(L, new CRainmanException(pE, __FILE__, __LINE__, "Cannot load RGD file \'%s\'", self->sFile));
			lua51_error(L);
		}
	}

	const char* sFile = lua51_tostring(L, 2);

	IFileStore::IOutputStream* pOutStr = 0;
	try
	{
		pOutStr = self->pStore->VOpenOutputStream(sFile, true);
	}
	catch(CRainmanException *pE)
	{
		lua51_pushlightuserdata(L, new CRainmanException(pE, __FILE__, __LINE__, "Cannot open file \'%s\' for saving", sFile));
		lua51_error(L);
	}

	self->pMacro->_onSave(sFile);

	try
	{
		self->pRgd->Save(pOutStr);
	}
	catch(CRainmanException *pE)
	{
		lua51_pushlightuserdata(L, new CRainmanException(pE, __FILE__, __LINE__, "Error while saving \'%s\'", sFile));
		lua51_error(L);
	}

	delete pOutStr;

	lua51_pushboolean(L, 1);
	return 1;
}

// T K+ V
int CRgdFileMacro::_tCRgdFile::luaf_SET(lua_State* L)
{
	int iTop = lua51_gettop(L);
	int iTStop = iTop - 2;
	if(iTStop < 1)
	{
		lua51_pushstring(L, "Invalid call to SET. Should be obj:SET(\"key\",...,\"value\")");
		lua51_error(L);
	}
	if(lua51_type(L, 1) != LUA_TTABLE && lua51_type(L, 1) != LUA_TUSERDATA)
	{
		lua51_pushboolean(L, 0);
		return 1;
	}
	for(int i = 2; i <= iTStop; ++i)
	{
		lua51_pushvalue(L, i);
		lua51_gettable(L, i - 1);
		lua51_replace(L, i);
		if(lua51_type(L, i) != LUA_TTABLE && lua51_type(L, i) != LUA_TUSERDATA)
		{
			lua51_pushboolean(L, 0);
			return 1;
		}
	}
	lua51_settable(L, iTStop);
	lua51_pushboolean(L, 1);
	return 1;
}

int CRgdFileMacro::_tCRgdFile::luaf_GET(lua_State* L)
{
	_tCRgdFile *self = _fromLua<_tCRgdFile>(L, 1);
	if(self == 0)
	{
		lua51_pushstring(L, LuaArgMsg("CRgdFile","GET",__LINE__));
		lua51_error(L);
	}

	if((lua51_tostring(L, 2) == 0) || stricmp(lua51_tostring(L, 2),"GameData") != 0) return 0;

	if(self->pRgd == 0)
	{
		try {self->_loadRgd();}
		catch(CRainmanException *pE){
			lua51_pushlightuserdata(L, new CRainmanException(pE, __FILE__, __LINE__, "Cannot load RGD file \'%s\'", self->sFile));
			lua51_error(L);
		}
	}

	int iTop = lua51_gettop(L);
	CRgdFile::_RgdEntry *pCurrentEntry = CRgdFileMacro::_getRootEntry(self->pRgd);

	for(int n=3; n <= iTop; ++n)
	{
		if( (pCurrentEntry->Type != IMetaNode::DT_Table) && (pCurrentEntry->Type != CRgdFile::sk_TableInt) ) return 0;
		if( pCurrentEntry->Data.t == 0) return 0;

		const char* sChildName = lua51_tostring(L, n);
		unsigned long iChildHash = 0;
		if(sChildName) iChildHash = self->pMacro->m_pHashTableToUse->ValueToHash(sChildName);

		for(std::vector<CRgdFile::_RgdEntry*>::iterator itr = pCurrentEntry->Data.t->begin(); itr != pCurrentEntry->Data.t->end(); ++itr)
		{
			if( (**itr).iHash == iChildHash )
			{
				pCurrentEntry = *itr;
				goto found_entry_child;
			}
		}
		return 0;
found_entry_child:;
	}
	if( (pCurrentEntry->Type == IMetaNode::DT_Table) || (pCurrentEntry->Type == CRgdFile::sk_TableInt) )
	{
		lua51_pushlightuserdata(L, (void*)self);			 // pRgdFile
		lua51_gettable(L, LUA_REGISTRYINDEX);				 // RgdFileT
		lua51_pushlightuserdata(L, (void*)pCurrentEntry);  // RgdFileT pEntry
		lua51_gettable(L, -2);							 // RgdFileT oEntry
		lua51_remove(L, -2);								 // oEntry
		if(lua51_type(L, -1) != LUA_TNIL) return 1;
		lua51_pop(L, 1);
	}
	CRgdFileMacro::_tCRgdTable::construct(L, self, pCurrentEntry);
	return 1;
}

int CRgdFileMacro::_tCRgdFile::luae_newindex(lua_State* L)
{
	lua51_pushstring(L, "RGD Files may only contain GameData as a root entry");
	lua51_error(L);
	return 0;
}

// T = V
int CRgdFileMacro::_tCRgdTable::luap_setself(lua_State* L)
{
	_tCRgdTable *self = _fromLua<_tCRgdTable>(L, 1);
	if(self == 0)
	{
		lua51_pushstring(L, LuaArgMsg("CRgdTable","SET",__LINE__));
		lua51_error(L);
	}
	_lua_udat_to_table(L, 2);
	CRgdFile::_CleanRgdTable(self->pRgdData);
	self->pRgdData->pExt = 0;
	self->pRgdData->Data.t = new std::vector<CRgdFile::_RgdEntry*>;

	lua51_pushstring(L, "children");
	lua51_gettable(L, 1);
	lua51_pushnil(L);
	while(lua51_next(L, -2))
	{
		lua51_pop(L, 1);
		lua51_pushnil(L);
		lua51_rawset(L, -3);
		lua51_pushnil(L);
	}
	lua51_replace(L, 1); // Tchildren VT 
	lua51_pushnil(L);
	while(lua51_next(L, -2))
	{
		lua51_pushvalue(L, -2);	// Tchildren VT K V K
		lua51_insert(L, -2);		// Tchildren VT K K V
		lua51_settable(L, 1);		// Tchildren VT K
	}
	lua51_settop(L, 1);

	return 1;
}

// T[K] = V
int CRgdFileMacro::_tCRgdFile::luae_setindex(lua_State* L)
{
	if(stricmp(lua51_tostring(L, 2),"GameData") != 0)
	{
		lua51_pushstring(L, "RGD Files may only contain GameData as a root entry");
		lua51_error(L);
	}
	lua51_insert(L, -2); // T V K
	lua51_gettable(L, 1); // T V O
	lua51_replace(L, 1); // O V
	if( (lua51_type(L, 2) != LUA_TTABLE) && (lua51_type(L, 2) != LUA_TUSERDATA) )
	{
		lua51_pushstring(L, "GameData can only hold a table");
		lua51_error(L);
	}
	return CRgdFileMacro::_tCRgdTable::luap_setself(L);
}

int CRgdFileMacro::_tCRgdFile::luae_gc(lua_State* L)
{
	_tCRgdFile *self = _fromLua<_tCRgdFile>(L, 1);

	/*
	lua51_getglobal(L, "on_gc");
	if(lua51_type(L, -1) != LUA_TNIL)
	{
		lua51_pushstring(L, self->sFile);
		lua51_pcall(L, 1, 0, 0);
	}
	*/

	if(self == 0)
	{
		lua51_pushstring(L, LuaArgMsg("CRgdFile","GameData",__LINE__));
		lua51_error(L);
	}
	if(self->pRgd)
	{
		delete self->pRgd;
	}
	free(self->sFile);
	lua51_pushlightuserdata(L, (void*)self);
	lua51_pushnil(L);
	lua51_settable(L, LUA_REGISTRYINDEX);
	return 0;
}

/*	T[K] = V OR T[K]
	where T is RgdFile->members
*/
int CRgdFileMacro::_tCRgdFile::luae_delayload(lua_State* L)
{
	_tCRgdFile *pThis = _fromLuaLight<_tCRgdFile>(L, lua_upvalueindex(1));

	if(pThis->pRgd == 0)
	{
		try {pThis->_loadRgd();}
		catch(CRainmanException *pE){
			lua51_pushlightuserdata(L, new CRainmanException(pE, __FILE__, __LINE__, "Cannot load RGD file \'%s\'", pThis->sFile));
			lua51_error(L);
		}
	}

	CRgdFile::_RgdEntry *pGameDataEntry = CRgdFileMacro::_getRootEntry(pThis->pRgd);

	bool bIs__index = false;

	if(lua51_gettop(L) == 2) // an index operation only has T and K on the stack
	{
		lua51_pushnil(L); // push a fake V to give the same stack indicies
		bIs__index = true;
	}

	// Push T's metatable
	lua51_getmetatable(L, -3); // T K V Tmeta

	// Create children table
	lua51_newtable(L); // T K V Tmeta ChildrenT

	// Register children table to be inside members
	lua51_pushstring(L, "__index");		// T K V Tmeta ChildrenT "__index"
	lua51_pushvalue(L, -2);				// T K V Tmeta ChildrenT "__index" ChildrenT
	lua51_settable(L, -4);				// T K V Tmeta ChildrenT
	lua51_pushstring(L, "__newindex");	// T K V Tmeta ChildrenT "__newindex"
	lua51_pushvalue(L, -2);				// T K V Tmeta ChildrenT "__newindex" ChildrenT
	lua51_settable(L, -4);				// T K V Tmeta ChildrenT
	lua51_pushstring(L, "__updateindex"); // T K V Tmeta ChildrenT "__updateindex"
	lua51_pushvalue(L, -2);				// T K V Tmeta ChildrenT "__updateindex" ChildrenT
	lua51_settable(L, -4);				// T K V Tmeta ChildrenT
	lua51_pushstring(L, "children");		// T K V Tmeta ChildrenT "children"
	lua51_pushvalue(L, -2);				// T K V Tmeta ChildrenT "children" ChildrenT
	lua51_rawset(L, -7);					// T K V Tmeta ChildrenT

	// Fill children table
	lua51_pushstring(L, "GameData");					 // T K V Tmeta ChildrenT "GameData"
	lua51_pushlightuserdata(L, (void*)pThis);			 // T K V Tmeta ChildrenT "GameData" pRgdFile
	lua51_gettable(L, LUA_REGISTRYINDEX);				 // T K V Tmeta ChildrenT "GameData" RgdFileT
	lua51_pushlightuserdata(L, (void*)pGameDataEntry); // T K V Tmeta ChildrenT "GameData" RgdFileT pEntry
	lua51_gettable(L, -2);							 // T K V Tmeta ChildrenT "GameData" RgdFileT oEntry
	lua51_remove(L, -2);								 // T K V Tmeta ChildrenT "GameData" oEntry
	if(lua51_type(L, -1) == LUA_TNIL)
	{
		lua51_pop(L, 1);													 // T K V Tmeta ChildrenT "GameData"
		CRgdFileMacro::_tCRgdTable::construct(L, pThis, pGameDataEntry); // T K V Tmeta ChildrenT "GameData" oEntry
	}
	lua51_settable(L, -3);							 // T K V Tmeta ChildrenT

	// Make children metatable
	lua51_newtable(L);					// T K V Tmeta ChildrenT ChildrenMeta
	lua51_pushstring(L, "__newindex");	// T K V Tmeta ChildrenT ChildrenMeta "__newindex"
	lua51_pushcfunction(L, luae_newindex);// T K V Tmeta ChildrenT ChildrenMeta "__newindex" luae_newindex
	lua51_settable(L, -3);				// T K V Tmeta ChildrenT ChildrenMeta
	lua51_pushstring(L, "__updateindex");	// T K V Tmeta ChildrenT ChildrenMeta "__updateindex"
	lua51_pushcfunction(L, luae_setindex);// T K V Tmeta ChildrenT ChildrenMeta "__updateindex" luae_setindex
	lua51_settable(L, -3);				// T K V Tmeta ChildrenT ChildrenMeta
	lua51_pushstring(L, "__metatable"); lua51_newtable(L); lua51_settable(L, -3);
	lua51_pushstring(L, "__call"); lua51_pushcfunction(L, luae_call); lua51_settable(L, -3);

	// Register children metatable, pop children
	lua51_setmetatable(L, -2);			// T K V Tmeta ChildrenT
	lua51_pop(L, 2);						// T K V

	// Run the table query again with the updated metatables and what not
	if(bIs__index)
	{
		lua51_pop(L, 1);		 // T K
		lua51_gettable(L, -2); // T V
		return 1;
	}
	else
	{
		lua51_settable(L, -3); // T
		return 0;
	}
}

CRgdFileMacro::_tCRgdFile* CRgdFileMacro::_tCRgdFile::construct(lua_State* L, CRgdFileMacro* _pMacro, const char* _sFile, IFileStore* _pStore)
{
	_tCRgdFile* pThis = lua51x_newuserdata<_tCRgdFile>(L);

	int iTop = lua51_gettop(L);

	pThis->construct(_pMacro, _sFile, _pStore);
	lua51_newtable(L);
	lua51_pushstring(L, "__gc");
	lua51_pushcfunction(L, luae_gc);
	lua51_settable(L, -3);

	lua51_newtable(L); // Make Members Table

	// Register members table to be the table for the userdata
	lua51_pushstring(L, "__index");
	lua51_pushvalue(L, -2);
	lua51_settable(L, -4);
	lua51_pushstring(L, "__newindex");
	lua51_pushvalue(L, -2);
	lua51_settable(L, -4);
	lua51_pushstring(L, "__updateindex");
	lua51_pushvalue(L, -2);
	lua51_settable(L, -4);
	lua51_pushstring(L, "__metatable"); lua51_newtable(L); lua51_settable(L, -4);
	lua51_pushstring(L, "__call"); lua51_pushcfunction(L, luae_call); lua51_settable(L, -4);

	// Fill members
	{
		const char* sName  = strrchr(_sFile, '\\');
		if(sName  == 0)
			sName  = _sFile;
		else
			++sName;
		const char* sName2 = strrchr(_sFile,  '/');
		if(sName2 == 0)
			sName2 = _sFile;
		else
			++sName2;
		lua51_pushstring(L, "name");
		lua51_pushstring(L, sName > sName2 ? sName : sName2);
		lua51_settable(L, -3);
	}

	lua51_pushstring(L, "path");
	lua51_pushstring(L, _sFile);
	lua51_settable(L, -3);

	lua51_pushstring(L, "GET"); lua51_pushcfunction(L, luaf_GET); lua51_settable(L, -3);
	lua51_pushstring(L, "SET"); lua51_pushcfunction(L, luaf_SET); lua51_settable(L, -3);
	lua51_pushstring(L, "save"); lua51_pushcfunction(L, luaf_save); lua51_settable(L, -3);
	lua51_pushstring(L, "saveAs"); lua51_pushcfunction(L, luaf_saveAs); lua51_settable(L, -3);

	// Create members metatable
	lua51_newtable(L);
	lua51_pushstring(L, "__index");
	lua51_pushlightuserdata(L, (void*)pThis);
	lua51_pushcclosure(L, luae_delayload, 1);
	lua51_settable(L, -3);
	lua51_pushstring(L, "__newindex");
	lua51_pushlightuserdata(L, (void*)pThis);
	lua51_pushcclosure(L, luae_delayload, 1);
	lua51_settable(L, -3);
	lua51_pushstring(L, "__updateindex");
	lua51_pushlightuserdata(L, (void*)pThis);
	lua51_pushcclosure(L, luae_delayload, 1);
	lua51_settable(L, -3);
	lua51_pushstring(L, "__metatable"); lua51_newtable(L); lua51_settable(L, -3);

	// Register members metatable, pop members
	lua51_setmetatable(L, -2);
	lua51_pop(L, 1);

	// Register userdata metatable
	lua51_setmetatable(L, -2); // RGDF

	int iTopNow = lua51_gettop(L);

	lua51_pushlightuserdata(L, (void*)pThis); // RGDF pUD
	lua51_newtable(L); // RGDF pUD TRGD

	lua51_newtable(L); // RGDF pUD TRGD TRGDM
	lua51_pushstring(L, "__mode"); // RGDF pUD TRGD TRGDM "__mode"
	lua51_pushstring(L, "v"); // RGDF pUD TRGD TRGDM "__mode" "v"
	lua51_settable(L, -3); // RGDF pUD TRGD TRGDM
	lua51_setmetatable(L, -2); // RGDF pUD TRGD

	lua51_pushlightuserdata(L, (void*)pThis); // RGDF pUD TRGD pUD
	lua51_pushvalue(L, -4); // RGDF pUD TRGD pUD RGDF
	lua51_settable(L, -3); // RGDF pUD TRGD

	lua51_settable(L, LUA_REGISTRYINDEX); // RGDF

	return pThis;
}

int CRgdFileMacro::luaf_pcall (lua_State *L) {
  int status;
  lua51L_checkany(L, 1);
  status = lua51_pcall(L, lua51_gettop(L) - 1, LUA_MULTRET, 0);
  lua51_pushboolean(L, (status == 0));
  lua51_insert(L, 1);
  if(status != 0 && lua51_type(L, 2) == LUA_TLIGHTUSERDATA)
  {
	  CRainmanException *pE = (CRainmanException*)lua51_touserdata(L, 2);
	  lua51_pushstring(L, "");
	  for(const CRainmanException* e = pE; e; e = e->getPrecursor())
	  {
		  lua51_pushstring(L, e->getFile());
		  lua51_pushstring(L, " line ");
		  lua51_pushnumber(L, (lua_Number)e->getLine()); lua51_tostring(L, -1);
		  lua51_pushstring(L, ": ");
		  lua51_pushstring(L, e->getMessage());
		  lua51_pushstring(L, "\r\n");
		  lua51_concat(L, 7);
	  }
	  pE->destroy();
	  lua51_replace(L, 2);
  }
  return lua51_gettop(L);  /* return status + all results */
}

void CRgdFileMacro::_tCRgdFile::_loadRgd()
{
	IFileStore::IStream* pStream = 0;
	try { pStream = pStore->VOpenStream(sFile); }
	catch(CRainmanException* pE) {throw new CRainmanException(pE,__FILE__,__LINE__,"Unable to open RGD file \'%s\'",sFile);}

	try{
		pRgd = CHECK_MEM(new CRgdFile);
	}catch(CRainmanException *pE){
		delete pStream;
		pRgd = 0;
		throw pE;
	}

	pRgd->SetHashTable(pMacro->m_pHashTableToUse);

	try{
		pRgd->Load(pStream);
	}catch(CRainmanException *pE){
		delete pStream;
		delete pRgd;
		pRgd = 0;
		throw pE;
	}
	delete pStream;
}

CRgdFile::_RgdEntry* CRgdFileMacro::_getRootEntry(CRgdFile* pFile)
{
	return &(pFile->m_pDataChunk->RootEntry);
}

// T K+
int CRgdFileMacro::_tCRgdTable::luaf_GET(lua_State* L)
{
	_tCRgdTable *self = _fromLua<_tCRgdTable>(L, 1);
	if(self == 0)
	{
		lua51_pushstring(L, LuaArgMsg("CRgdTable","GET",__LINE__));
		lua51_error(L);
	}
	
	int iTop = lua51_gettop(L);
	CRgdFile::_RgdEntry *pCurrentEntry = self->pRgdData;

	for(int n=2; n <= iTop; ++n)
	{
		if( (pCurrentEntry->Type != IMetaNode::DT_Table) && (pCurrentEntry->Type != CRgdFile::sk_TableInt) ) return 0;
		if( pCurrentEntry->Data.t == 0) return 0;

		const char* sChildName = lua51_tostring(L, n);
		unsigned long iChildHash = 0;
		if(sChildName) iChildHash = self->pMacro->m_pHashTableToUse->ValueToHash(sChildName);

		for(std::vector<CRgdFile::_RgdEntry*>::iterator itr = pCurrentEntry->Data.t->begin(); itr != pCurrentEntry->Data.t->end(); ++itr)
		{
			if( (**itr).iHash == iChildHash )
			{
				pCurrentEntry = *itr;
				goto found_entry_child;
			}
		}
		return 0;
	found_entry_child:;
	}
	if( (pCurrentEntry->Type == IMetaNode::DT_Table) || (pCurrentEntry->Type == CRgdFile::sk_TableInt) )
	{
		lua51_pushlightuserdata(L, (void*)self);			 // pRgdFile
		lua51_gettable(L, LUA_REGISTRYINDEX);				 // RgdFileT
		lua51_pushlightuserdata(L, (void*)pCurrentEntry);  // RgdFileT pEntry
		lua51_gettable(L, -2);							 // RgdFileT oEntry
		lua51_remove(L, -2);								 // oEntry
		if(lua51_type(L, -1) != LUA_TNIL) return 1;
		lua51_pop(L, 1);
	}
	construct(L, self->pRgdFile, pCurrentEntry);
	return 1;
}

// T K* V
int CRgdFileMacro::_tCRgdTable::luaf_SET(lua_State* L)
{
	int iTop = lua51_gettop(L);
	int iTStop = iTop - 2;
	if(iTStop < 1)
	{
		if(iTop == 2)
		{
			_tCRgdTable *self = _fromLua<_tCRgdTable>(L, 1);
			if(self != 0)
			{
				return luap_setself(L);
			}
		}

		lua51_pushstring(L, "Invalid call to SET. Should be obj:SET(\"key\",...,\"value\")");
		lua51_error(L);
	}
	if(lua51_type(L, 1) != LUA_TTABLE && lua51_type(L, 1) != LUA_TUSERDATA)
	{
		lua51_pushboolean(L, 0);
		return 1;
	}
	for(int i = 2; i <= iTStop; ++i)
	{
		lua51_pushvalue(L, i);
		lua51_gettable(L, i - 1);
		lua51_replace(L, i);
		if(lua51_type(L, i) != LUA_TTABLE && lua51_type(L, i) != LUA_TUSERDATA)
		{
			lua51_pushboolean(L, 0);
			return 1;
		}
	}
	lua51_settable(L, iTStop);
	lua51_pushboolean(L, 1);
	return 1;
}

// T[K] = V (T[K] == nil)
int CRgdFileMacro::_tCRgdTable::luae_newindex(lua_State* L)
{
	if(lua51_type(L, 3) == LUA_TNIL) return 0;

	_tCRgdTable *self = _fromLua<_tCRgdTable>(L, 1);
	CRgdFile::_RgdEntry* pUs = self->pRgdData;
	CRgdFile::_RgdEntry* pNewChild = new CRgdFile::_RgdEntry;
	pUs->Data.t->push_back(pNewChild);

	pNewChild->pParentFile = pUs->pParentFile;
	pNewChild->sName = pUs->pParentFile->GetHashTable()->HashToValue(pNewChild->iHash = pUs->pParentFile->GetHashTable()->ValueToHash(lua51_tostring(L, 2)));
	pNewChild->Type = CRgdFile::DT_Bool;
	pNewChild->Data.b = false;
	pNewChild->pExt = 0;
	
	return luae_setindex(L);
}

void CRgdFileMacro::setUcsResolver(CModuleFile* pModule)
{
	m_pUcsResolverModule = pModule;
}

void CRgdFileMacro::setIsDowMod(bool bIsDowMod)
{
	m_bIsDowMod = bIsDowMod;
}

// T[K] = V (T[K] ~= nil)
int CRgdFileMacro::_tCRgdTable::luae_setindex(lua_State* L)
{
	_tCRgdTable *self = _fromLua<_tCRgdTable>(L, 1);

	CRgdFile::_RgdEntry* pUs = self->pRgdData, *pChild = 0;

	const char* sChildName = lua51_tostring(L, 2);
	unsigned long iChildHash = 0;
	if(sChildName) iChildHash = self->pMacro->m_pHashTableToUse->ValueToHash(sChildName);

	for(std::vector<CRgdFile::_RgdEntry*>::iterator itr = pUs->Data.t->begin(); itr != pUs->Data.t->end(); ++itr)
	{
		if( (**itr).iHash == iChildHash )
		{
			pChild = *itr;
			if(lua51_type(L, 3) == LUA_TNIL)
			{
				switch(pChild->Type)
				{
				case CRgdFile::DT_Table:
				case CRgdFile::sk_TableInt:
					CRgdFile::_CleanRgdTable(pChild);
					break;
				case CRgdFile::DT_String:
					if(pChild->Data.s) delete[] pChild->Data.s;
					break;
				case CRgdFile::DT_WString:
					if(pChild->Data.ws) delete[] pChild->Data.ws;
					break;
				};
				delete pChild;
				pUs->Data.t->erase(itr);
				return 0;
			}
			break;
		}
	}
	if(!pChild) return 0;
	if(iChildHash == 0x49D60FAE) pUs->pExt = pChild;

	switch(pChild->Type)
	{
	case CRgdFile::DT_Table:
	case CRgdFile::sk_TableInt:
		CRgdFile::_CleanRgdTable(pChild);
		break;
	case CRgdFile::DT_String:
		if(pChild->Data.s) delete[] pChild->Data.s;
		break;
	case CRgdFile::DT_WString:
		if(pChild->Data.ws) delete[] pChild->Data.ws;
		break;
	};

	switch(lua51_type(L, 3))
	{
	case LUA_TBOOLEAN:
		pChild->Type = CRgdFile::DT_Bool;
		pChild->Data.b = lua51_toboolean(L, 3);
		break;

	case LUA_TNUMBER:
		pChild->Type = CRgdFile::DT_Float;
		pChild->Data.f = lua51_tonumber(L, 3);
		break;

	case LUA_TSTRING:
		{
			const char* s = lua51_tostring(L, 3);
			if(*s == '$')
			{
				for(const char* s2 = ++s; *s2; ++s2)
				{
					if( (*s2 < '0') || (*s2 > '9') )
					{
						--s;
						break;
					}
				}
				if(*s != '$')
				{
					if(self->pMacro->m_bIsDowMod)
					{
						pChild->Type = CRgdFile::DT_WString;
						pChild->Data.ws = new wchar_t[strlen(s) + 2];
						pChild->Data.ws[0] = '$';
						for(int i = 0; s[i-1]; ++i) pChild->Data.ws[i+1] = s[i];
						break;
					}
					else
					{
						pChild->Type = CRgdFile::DT_Integer;
						pChild->Data.i = 0;
						while(*s)
						{
							pChild->Data.i *= 10;
							pChild->Data.i += (*s - '0');
							++s;
						}
						break;
					}
				}
			}
			pChild->Type = CRgdFile::DT_String;
			pChild->Data.s = new char[strlen(s) + 1];
			strcpy(pChild->Data.s, s);
			break;
		}

	case LUA_TTABLE:
	case LUA_TUSERDATA:
		{
			pChild->Data.t = new std::vector<CRgdFile::_RgdEntry*>;
			if(pChild->Type != CRgdFile::DT_Table)
			{
				pChild->Type = CRgdFile::DT_Table;
				lua51_pushstring(L, "children");
				lua51_gettable(L, 1);
				lua51_pushvalue(L, 2);
				construct(L, self->pRgdFile, pChild);
				lua51_rawset(L, -3);
				lua51_pop(L, 1);
			}

			lua51_insert(L, 2); // T V K
			lua51_gettable(L, 1); // T V O
			lua51_replace(L, 1); // O V
			return luap_setself(L);
		}

	default:
		lua51_pushstring(L, "RGD Tables can only hold numbers, strings, booleans or tables");
		lua51_error(L);
	};

	lua51_pushstring(L, "children");
	lua51_gettable(L, 1); // T K V children
	lua51_pushvalue(L, 2); // T K V children K 
	lua51_pushvalue(L, 3); // T K V children K V
	lua51_rawset(L, -3); // T K V children
	lua51_pop(L, 1);

	return 1;
}

int CRgdFileMacro::_tCRgdTable::luaf_construct(lua_State* L)
{
	_tCRgdFile *pRgdFile = _fromLuaLight<_tCRgdFile>(L, lua_upvalueindex(1));
	CRgdFile::_RgdEntry *pEntry = _fromLuaLight<CRgdFile::_RgdEntry>(L, lua_upvalueindex(2));

	lua51_pushlightuserdata(L, (void*)pRgdFile); // P
	lua51_gettable(L, LUA_REGISTRYINDEX); // T
	lua51_pushlightuserdata(L, (void*)pEntry); // T K
	lua51_gettable(L, -2); // T V
	if(lua51_type(L, -1) == LUA_TNIL)
	{
		lua51_pop(L, 2);
		construct(L, pRgdFile, pEntry);
	}
	return 1;
}

/*	T[K] = V OR T[K]
	where T is RgdTable->members
*/
int CRgdFileMacro::_tCRgdTable::luae_delayload(lua_State* L)
{
	_tCRgdTable *pThis = _fromLuaLight<_tCRgdTable>(L, lua_upvalueindex(1));
	if(pThis->bLoaded == false) pThis->bLoaded = true;

	bool bIs__index = false;

	if(lua51_gettop(L) == 2) // an index operation only has T and K on the stack
	{
		lua51_pushnil(L); // push a fake V to give the same stack indicies
		bIs__index = true;
	}

	// Push T's metatable
	lua51_getmetatable(L, -3); // T K V Tmeta

	// Create children table
	lua51_newtable(L); // T K V Tmeta ChildrenT

	// Register children table to be inside members
	lua51_pushstring(L, "__index");		// T K V Tmeta ChildrenT "__index"
	lua51_pushvalue(L, -2);				// T K V Tmeta ChildrenT "__index" ChildrenT
	lua51_settable(L, -4);				// T K V Tmeta ChildrenT
	lua51_pushstring(L, "__newindex");	// T K V Tmeta ChildrenT "__newindex"
	lua51_pushvalue(L, -2);				// T K V Tmeta ChildrenT "__newindex" ChildrenT
	lua51_settable(L, -4);				// T K V Tmeta ChildrenT
	lua51_pushstring(L, "__updateindex"); // T K V Tmeta ChildrenT "__updateindex"
	lua51_pushvalue(L, -2);				// T K V Tmeta ChildrenT "__updateindex" ChildrenT
	lua51_settable(L, -4);				// T K V Tmeta ChildrenT
	lua51_pushstring(L, "children");		// T K V Tmeta ChildrenT "children"
	lua51_pushvalue(L, -2);				// T K V Tmeta ChildrenT "children" ChildrenT
	lua51_rawset(L, -7);					// T K V Tmeta ChildrenT

	// Put table object lookup table onto stack
	lua51_pushlightuserdata(L, (void*)pThis->pRgdFile);// T K V Tmeta ChildrenT pRgdFile
	lua51_gettable(L, LUA_REGISTRYINDEX);				 // T K V Tmeta ChildrenT RgdFileT

	// Fill children table
	for(std::vector<CRgdFile::_RgdEntry*>::iterator itr = pThis->pRgdData->Data.t->begin(); itr != pThis->pRgdData->Data.t->end(); ++itr)
	{
		if((**itr).sName)
		{
			lua51_pushstring(L, (**itr).sName); // T K V Tmeta ChildrenT RgdFileT key
		}
		else
		{
			char sHex[11] = {'0','x','1','2','3','4','5','6','7','8','\0'};
			for(int n = 0; n < 8; ++n) sHex[9-n] = "0123456789ABCDEF"[((**itr).iHash >> (n * 4)) & 0xF];
			lua51_pushstring(L, sHex); // T K V Tmeta ChildrenT RgdFileT key
		}
		if((**itr).Type == IMetaNode::DT_Table || (**itr).Type == CRgdFile::sk_TableInt)
		{
			lua51_pushlightuserdata(L, (void*)(*itr)); // T K V Tmeta ChildrenT RgdFileT key pEntry
			lua51_gettable(L, -3); // T K V Tmeta ChildrenT RgdFileT key oEntry
			if(lua51_type(L, -1) == LUA_TNIL)
			{
				lua51_pop(L, 1); // T K V Tmeta ChildrenT RgdFileT key
				construct(L, pThis->pRgdFile, *itr); // T K V Tmeta ChildrenT RgdFileT key val
			}
		}
		else
		{
			construct(L, 0, *itr); // T K V Tmeta ChildrenT RgdFileT key val
		}
		lua51_settable(L, -4); // T K V Tmeta ChildrenT RgdFileT
	}

	// Make children metatable
	lua51_newtable(L); // T K V Tmeta ChildrenT RgdFileT ChildrenMeta
	lua51_pushstring(L, "__newindex"); lua51_pushcfunction(L, luae_newindex); lua51_settable(L, -3);
	lua51_pushstring(L, "__updateindex"); lua51_pushcfunction(L, luae_setindex); lua51_settable(L, -3);
	lua51_pushstring(L, "__metatable"); lua51_newtable(L); lua51_settable(L, -3);
	lua51_pushstring(L, "__call"); lua51_pushcfunction(L, luae_call); lua51_settable(L, -3);

	lua51_pushstring(L, "userdata"); // "userdata"
	lua51_pushlightuserdata(L, pThis->pRgdFile); // "userdata" FileP
	lua51_gettable(L, LUA_REGISTRYINDEX); // "userdata" FileO
	lua51_pushlightuserdata(L, pThis->pRgdData); // "userdata" FileO DataP
	lua51_gettable(L, -2); // "userdata" FileO DataO
	lua51_replace(L, -2); // "userdata" DataO
	lua51_settable(L, -3); //

	// Register metatable and pop a few things
	lua51_setmetatable(L, -3); // T K V Tmeta ChildrenT RgdFileT
	lua51_pop(L, 3);			 // T K V

	// Run the table query again with the updated metatables and what not
	if(bIs__index)
	{
		lua51_pop(L, 1);		 // T K
		lua51_gettable(L, -2); // T V
		return 1;
	}
	else
	{
		lua51_settable(L, -3); // T
		return 0;
	}
}

CRgdFileMacro::_tCRgdTable* CRgdFileMacro::_tCRgdTable::construct(lua_State* L, _tCRgdFile* pRgdFile, CRgdFile::_RgdEntry* pRgdData)
{
	if(pRgdData->Type == IMetaNode::DT_Table || pRgdData->Type == CRgdFile::sk_TableInt)
	{
		_tCRgdTable* pThis = lua51x_newuserdata<_tCRgdTable>(L); // O
		pThis->construct(pRgdFile, pRgdData);

		int iTop = lua51_gettop(L);

		lua51_pushlightuserdata(L, (void*)pRgdFile); // O P
		lua51_gettable(L, LUA_REGISTRYINDEX); // O T
		lua51_pushlightuserdata(L, (void*)pRgdData); // O T K
		lua51_pushvalue(L, -3); // O T K O
		lua51_settable(L, -3); // O T

		lua51_pushlightuserdata(L, (void*)pRgdFile); // O T P
		lua51_gettable(L, -2); // O T F
		lua51_insert(L, -2); // O F T
		lua51_pop(L ,1); // O F

		// Make metatable
		lua51_newtable(L); // O F M
		lua51_insert(L, -2); // O M F
		lua51_pushstring(L, "fileobj"); // O M F "fileobj"
		lua51_insert(L, -2); // O M "fileobj" F
		lua51_settable(L, -3); // O M

		// Make members table
		lua51_newtable(L);

		// Register members table to be the table for the userdata
		lua51_pushstring(L, "__index");
		lua51_pushvalue(L, -2);
		lua51_settable(L, -4);
		lua51_pushstring(L, "__newindex");
		lua51_pushvalue(L, -2);
		lua51_settable(L, -4);
		lua51_pushstring(L, "__updateindex");
		lua51_pushvalue(L, -2);
		lua51_settable(L, -4);
		lua51_pushstring(L, "__metatable");
		lua51_newtable(L);
		lua51_settable(L, -4);
		lua51_pushstring(L, "__call"); lua51_pushcfunction(L, luae_call); lua51_settable(L, -4);

		// Fill members
		lua51_pushstring(L, "GET"); lua51_pushcfunction(L, luaf_GET); lua51_settable(L, -3);
		lua51_pushstring(L, "SET"); lua51_pushcfunction(L, luaf_SET); lua51_settable(L, -3);

		// Create members metatable
		lua51_newtable(L);
		lua51_pushstring(L, "__index");
		lua51_pushlightuserdata(L, (void*)pThis);
		lua51_pushcclosure(L, luae_delayload, 1);
		lua51_settable(L, -3);
		lua51_pushstring(L, "__newindex");
		lua51_pushlightuserdata(L, (void*)pThis);
		lua51_pushcclosure(L, luae_delayload, 1);
		lua51_settable(L, -3);
		lua51_pushstring(L, "__updateindex");
		lua51_pushlightuserdata(L, (void*)pThis);
		lua51_pushcclosure(L, luae_delayload, 1);
		lua51_settable(L, -3);
		lua51_pushstring(L, "__metatable");
		lua51_newtable(L);
		lua51_settable(L, -3);

		// Register members metatable, pop members
		lua51_setmetatable(L, -2);
		lua51_pop(L, 1);

		// Register userdata metatable
		lua51_setmetatable(L, -2);

		int iTop2 = lua51_gettop(L);

		return pThis;
	}
	else
	{
		switch(pRgdData->Type)
		{
		case IMetaNode::DT_Float: lua51_pushnumber(L, (lua_Number)pRgdData->Data.f); break;
		case IMetaNode::DT_Integer:
			{
				char sBuf[32];
				sprintf(sBuf, "$%lu", pRgdData->Data.i);
				lua51_pushstring(L, sBuf);
				break;
			}
		case IMetaNode::DT_Bool: lua51_pushboolean(L, pRgdData->Data.b ? 1 : 0); break;
		case IMetaNode::DT_String: lua51_pushstring(L, pRgdData->Data.s); break;
		case IMetaNode::DT_WString:
			{
				char *sBuf = new char[wcslen(pRgdData->Data.ws) + 1], *psBuf;
				psBuf = sBuf - 1;
				wchar_t* pBuf = pRgdData->Data.ws - 1;
				do
				{
					++psBuf;
					++pBuf;
					*psBuf = (char)*pBuf;
				} while(*pBuf);
				lua51_pushstring(L, sBuf);
				delete[] sBuf;
				break;
			}
		default:
			lua51_pushnil(L); break;
		}
		return 0;
	}
}

int CRgdFileMacro::luaf_ucs(lua_State* L)
{
	CRgdFileMacro* pThis = (CRgdFileMacro*)lua51_touserdata(L, lua_upvalueindex(1));
	lua51_settop(L, 1);
	if(lua51_type(L, 1) == LUA_TSTRING)
	{
		size_t iStrLen;
		const char* sStr = lua51_tolstring(L, 1, &iStrLen);

		if(*sStr == '$')
		{
			unsigned long iN = 0;
			for(size_t i = 1; i < iStrLen; ++i)
			{
				if(sStr[i] < '0' || sStr[i] > '9') return 1;
				iN *= 10;
				iN += (sStr[i] - '0');
			}
			if(pThis->m_pUcsResolverModule)
			{
				const wchar_t* pValue = pThis->m_pUcsResolverModule->ResolveUCS(iN);
				if(pValue == 0)
				{
					char sBuffer[256];
					sprintf(sBuffer, "$%lu no key", iN);
					lua51_pushstring(L, sBuffer);
					return 1;
				}
				else
				{
					size_t iL = wcslen(pValue) + 1;
					char* sValue = new char[iL];
					for(size_t i = 0; i < iL; ++i) sValue[i] = pValue[i];
					lua51_pushlstring(L, sValue, iL-1);
					delete[] sValue;
					return 1;
				}
			}
			else
			{
				char sBuffer[256];
				sprintf(sBuffer, "$%lu unable to be resolved", iN);
				lua51_pushstring(L, sBuffer);
				return 1;
			}
		}
	}
	else if(lua51_type(L, 1) == LUA_TNUMBER)
	{
		unsigned long iN = (unsigned long)lua51_tonumber(L, 1);
		if(pThis->m_pUcsResolverModule)
		{
			const wchar_t* pValue = pThis->m_pUcsResolverModule->ResolveUCS(iN);
			if(pValue == 0)
			{
				char sBuffer[256];
				sprintf(sBuffer, "$%lu no key", iN);
				lua51_pushstring(L, sBuffer);
				return 1;
			}
			else
			{
				size_t iL = wcslen(pValue) + 1;
				char* sValue = new char[iL];
				for(size_t i = 0; i < iL; ++i) sValue[i] = pValue[i];
				lua51_pushlstring(L, sValue, iL-1);
				delete[] sValue;
				return 1;
			}
		}
		else
		{
			char sBuffer[256];
			sprintf(sBuffer, "$%lu unable to be resolved", iN);
			lua51_pushstring(L, sBuffer);
			return 1;
		}
	}
	return 1;
}

int CRgdFileMacro::luaf_print(lua_State* L)
{
	CRgdFileMacro* pThis = (CRgdFileMacro*)lua51_touserdata(L, lua_upvalueindex(1));
	int n = lua51_gettop(L);  /* number of arguments */
	int i;
	lua51_getglobal(L, "tostring");
	for (i=1; i<=n; i++) {
		const char *s;
		lua51_pushvalue(L, -1);  /* function to be called */
		lua51_pushvalue(L, i);   /* value to print */
		lua51_call(L, 1, 1);
		s = lua51_tostring(L, -1);  /* get result */
		if (s == NULL) return lua51L_error(L, "`tostring' must return a string to `print'");
		//pThis->_onPrint(s);
		lua51_replace(L, i);  /* pop result */
	}
	lua51_pop(L, 1);
	lua51_pushstring(L, "\r\n");
	lua51_concat(L, n + 1);
	pThis->_onPrint(lua51_tostring(L, -1));
	return 0;
}

void CRgdFileMacro::_lua_udat_to_table(lua_State *L, int i)
{
	if(lua51_type(L, i) == LUA_TUSERDATA)
	{
		_tUserData* p = (_tUserData*)lua51_touserdata(L,i);
		if(p != 0 && (p->iMagic == _tCRgdFile::kMagic || p->iMagic == _tCRgdTable::kMagic) )
		{
			lua51_pushstring(L, "children");
			lua51_gettable(L, i);
			lua51_replace(L, i);
		}
	}
}

int CRgdFileMacro::luaf_next (lua_State *L) {
  _lua_udat_to_table(L, 1);
  lua51L_checktype(L, 1, LUA_TTABLE);
  if(lua51_gettop(L) < 2) lua51_settop(L, 2);  /* create a 2nd argument if there isn't one */
  if (lua51_next(L, 1))
    return 2;
  else {
    lua51_pushnil(L);
    return 1;
  }
}

void CRgdFileMacro::_tCRgdTable::construct(_tCRgdFile* _pRgdFile, CRgdFile::_RgdEntry* _pRgdData)
{
	iMagic = kMagic;
	pRgdFile = _pRgdFile;
	pMacro = pRgdFile->pMacro;
	pRgdData = _pRgdData;
	bLoaded = false;
}

void CRgdFileMacro::_onPrint(const char* sMsg)
{
	if(m_fpPrintCallback) m_fpPrintCallback(m_pCallbackTag, sMsg);
}

bool CRgdFileMacro::_onSave(const char* sFile)
{
	if(m_fpOnSaveCallback) return m_fpOnSaveCallback(m_pCallbackTag, sFile);
	return true;
}

bool CRgdFileMacro::_onLoad(const char* sFile)
{
	if(m_fpOnLoadCallback) return m_fpOnLoadCallback(m_pCallbackTag, sFile);
	return true;
}

void CRgdFileMacro::setHashTable(CRgdHashTable* pTable) {m_pHashTableToUse = pTable;}
void CRgdFileMacro::setCallbackPrint(void(*pFunction)(void*,const char*)) {m_fpPrintCallback = pFunction;}
void CRgdFileMacro::setCallbackSave(bool(*pFunction)(void*,const char*)) {m_fpOnSaveCallback = pFunction;}
void CRgdFileMacro::setCallbackLoad(bool(*pFunction)(void*,const char*)) {m_fpOnLoadCallback = pFunction;}
void CRgdFileMacro::setCallbackTag(void* pTag) {m_pCallbackTag = pTag;}

int CRgdFileMacro::luaf_pairs(lua_State *L)
{
  _lua_udat_to_table(L, 1);
  lua51L_checktype(L, 1, LUA_TTABLE);
  lua51_pushliteral(L, "next");
  lua51_rawget(L, LUA_GLOBALSINDEX);  /* return generator, */
  lua51_pushvalue(L, 1);  /* state, */
  lua51_pushnil(L);  /* and initial value */
  return 3;
}

int CRgdFileMacro::luae_call (lua_State *L)
{
	int iArgCount = lua51_gettop(L);

	if(iArgCount == 1) // just T/O
	{
		return luaf_pairs(L);
	}
	if(iArgCount == 3) // T/O S K
	{
		lua51_remove(L, 2);
		return luaf_next(L);
	}
	return 0;
}

int CRgdFileMacro::luaf_xpcall	(lua_State* L)
{
	int status;
	lua51L_checkany(L, 2);
	lua51_settop(L, 2);
	lua51_pushcclosure(L, luaf_xpcalle, 1);
	lua51_insert(L, 1);  /* put error function under function to be called */
	status = lua51_pcall(L, 0, LUA_MULTRET, 1);
	lua51_pushboolean(L, (status == 0));
	lua51_replace(L, 1);
	return lua51_gettop(L);  /* return status + all results */
}

int CRgdFileMacro::luaf_xpcalle (lua_State* L)
{
	for(int i = 1; i <= lua51_gettop(L); ++i)
	{
		if(lua51_type(L, i) == LUA_TLIGHTUSERDATA)
		{
			CRainmanException *pE = (CRainmanException*)lua51_touserdata(L, i);
			lua51_pushstring(L, "");
			for(const CRainmanException* e = pE; e; e = e->getPrecursor())
			{
				lua51_pushstring(L, e->getFile());
				lua51_pushstring(L, " line ");
				lua51_pushnumber(L, (lua_Number)e->getLine()); lua51_tostring(L, -1);
				lua51_pushstring(L, ": ");
				lua51_pushstring(L, e->getMessage());
				lua51_pushstring(L, "\r\n");
				lua51_concat(L, 7);
			}
			pE->destroy();
			lua51_replace(L, i);
		}
	}
	lua51_pushvalue(L, lua_upvalueindex(1));
	lua51_insert(L, 1);
	lua51_call(L, lua51_gettop(L) - 1, LUA_MULTRET);
	return lua51_gettop(L);
}

void CRgdFileMacro::unloadMacro()
{
	if(m_pL) _cleanLua();
}

void CRgdFileMacro::setCallbackSecurity(bool(*pFunction)(void*,eSecurityTypes))
{
	m_fpOnSecurityCallback = pFunction;
}

int CRgdFileMacro::luaf_loadrgd(lua_State* L)
{
	CRgdFileMacro* pThis = (CRgdFileMacro*)lua51_touserdata(L, lua_upvalueindex(1));
	if(!pThis->_onLoad(lua51_tostring(L, 1))) return 0;
	_tCRgdFile::construct(L, pThis, lua51_tostring(L, 1), pThis->m_pUcsResolverModule);
	lua51_pushstring(L, "GameData");
	lua51_gettable(L, -2);
	lua51_pop(L, 1);
	return 1;
}

int CRgdFileMacro::luaf_table(lua_State *L)
{
	if(lua51_gettop(L) < 2)
		lua51_newtable(L);
	lua51L_checktype(L, 2, LUA_TTABLE);
	lua51_settop(L, 2); // table t
	lua51_insert(L, 1); // t table
	lua51_newtable(L); // t table mt
	lua51_insert(L, 2); // t mt table
	lua51_setfield(L, 2, "__index"); // t mt
	lua51_setmetatable(L, 1); // t
	return 1;
}

int CRgdFileMacro::luaf_tfilter	(lua_State *L)
{
	lua51L_checkany(L, 2); // ot f
	lua51_pushvalue(L, 1); // ot f ot
	_lua_udat_to_table(L, 1); // t f ot
	lua51L_checktype(L, 1, LUA_TTABLE);

	lua51_pushnil(L); // t f ot k
	int iNext = lua51_next(L, 1);
	while(iNext != 0)
	{ // t f ot k v
		lua51_pushvalue(L, 2);	// t f ot k v f
		lua51_pushvalue(L, 4);	// t f ot k v f k
		lua51_pushvalue(L, 5);	// t f ot k v f k v
		lua51_call(L, 2, 1);	// t f ot k v r
		if(lua51_toboolean(L, 6) == 0)
		{
			lua51_pop(L, 2); // t f ot k
			lua51_pushvalue(L, 4); // t f ot k k
			iNext = lua51_next(L, 1); // t f ot k [knew vnew]
			lua51_pushvalue(L, 4); // t f ot k [knew vnew] k
			lua51_remove(L, 4); // t f ot [knew vnew] k
			lua51_pushnil(L); // t f ot [knew vnew] k nil
			lua51_settable(L, 3); // t f ot [knew vnew]
		}
		else
		{
			lua51_pop(L, 2); // t f ot k
			iNext = lua51_next(L, 1); // t f ot [knew vnew]
		}
	}
	return 1;
}

int CRgdFileMacro::luaf_formatme(lua_State *L)
{
	lua51L_checkany(L, 2);	// o format ...
	lua51_pushvalue(L, 1);	// o format ... o
	lua51_pushvalue(L, 2);	// o format ... o format
	lua51_replace(L, 1);	// format format ... o
	lua51_replace(L, 2);	// format o ...
	lua51_getglobal(L, "string");		// format o ... string
	lua51_getfield(L, -1, "format");	// format o ... string string.format
	lua51_insert(L, 1);		// string.format format o ... string
	lua51_pop(L, 1);		// string.format format o ...
	lua51_call(L, lua51_gettop(L) - 1, LUA_MULTRET);
	return lua51_gettop(L);
}

void CRgdFileMacro::loadMacro(const char* sCode)
{
	if(m_pL) _cleanLua();
	m_pL = CHECK_MEM(lua51_open());

	int iTop = lua51_gettop(m_pL);

	lua51_pushcfunction(m_pL, lua51open_base); lua51_call(m_pL, 0, 0);
	lua51_pushcfunction(m_pL, lua51open_table); lua51_call(m_pL, 0, 0);
	lua51_pushcfunction(m_pL, lua51open_math); lua51_call(m_pL, 0, 0);
	lua51_pushcfunction(m_pL, lua51open_io); lua51_call(m_pL, 0, 0);
	lua51_pushcfunction(m_pL, lua51open_os); lua51_call(m_pL, 0, 0);
	lua51_pushcfunction(m_pL, lua51open_string); lua51_call(m_pL, 0, 0);
	lua51_pushcfunction(m_pL, lua51open_debug); lua51_call(m_pL, 0, 0);

	lua51_getglobal(m_pL, "math");
	lua51_createtable(m_pL, 0, 1);  /* create metatable for numbers */
	lua51_pushnumber(m_pL, 0);  /* dummy number */
	lua51_pushvalue(m_pL, -2);
	lua51_setmetatable(m_pL, -2);  /* set number metatable */
	lua51_pop(m_pL, 1);  /* pop dummy number */
	lua51_pushvalue(m_pL, -2);  /* math library... */
	lua51_setfield(m_pL, -2, "__index");  /* ...is the __index metamethod */
	lua51_pop(m_pL, 2);  /* pop metatable */

	int iTop2 = lua51_gettop(m_pL);

	lua51_pushstring(m_pL, "print");
	lua51_pushlightuserdata(m_pL, (void*)this);
	lua51_pushcclosure(m_pL, luaf_print, 1);
	lua51_settable(m_pL, LUA_GLOBALSINDEX);

	lua51_pushstring(m_pL, "UCS");
	lua51_pushlightuserdata(m_pL, (void*)this);
	lua51_pushcclosure(m_pL, luaf_ucs, 1);
	lua51_settable(m_pL, LUA_GLOBALSINDEX);

	lua51_pushstring(m_pL, "loadRgd");
	lua51_pushlightuserdata(m_pL, (void*)this);
	lua51_pushcclosure(m_pL, luaf_loadrgd, 1);
	lua51_settable(m_pL, LUA_GLOBALSINDEX);

	lua51_pushstring(m_pL, "next");
	lua51_pushcclosure(m_pL, luaf_next, 0);
	lua51_settable(m_pL, LUA_GLOBALSINDEX);
	lua51_pushstring(m_pL, "pairs");
	lua51_pushcclosure(m_pL, luaf_pairs, 0);
	lua51_settable(m_pL, LUA_GLOBALSINDEX);
	lua51_pushstring(m_pL, "pcall");
	lua51_pushcclosure(m_pL, luaf_pcall, 0);
	lua51_settable(m_pL, LUA_GLOBALSINDEX);
	lua51_pushstring(m_pL, "xpcall");
	lua51_pushcclosure(m_pL, luaf_xpcall, 0);
	lua51_settable(m_pL, LUA_GLOBALSINDEX);

	lua51_getglobal(m_pL, "table");
	lua51_pushcfunction(m_pL, luaf_tfilter); lua51_setfield(m_pL, -2, "filter");
	if(lua51_getmetatable(m_pL, -1) == 0)
	{
		lua51_createtable(m_pL, 0, 1);
		lua51_pushvalue(m_pL, -1);
		lua51_setmetatable(m_pL, -3);
	}
	lua51_pushcfunction(m_pL, luaf_table);
	lua51_setfield(m_pL, -2, "__call");
	lua51_pop(m_pL, 2);

	lua51_getglobal(m_pL, "string");
	lua51_pushstring(m_pL, "split"); lua51_pushcclosure(m_pL, luax51_string_split, 0); lua51_settable(m_pL, -3);
	lua51_pushstring(m_pL, "after"); lua51_pushcclosure(m_pL, luax51_string_after, 0); lua51_settable(m_pL, -3);
	lua51_pushstring(m_pL, "afterlast"); lua51_pushcclosure(m_pL, luax51_string_afterlast, 0); lua51_settable(m_pL, -3);
	lua51_pushstring(m_pL, "before"); lua51_pushcclosure(m_pL, luax51_string_before, 0); lua51_settable(m_pL, -3);
	lua51_pushstring(m_pL, "beforelast"); lua51_pushcclosure(m_pL, luax51_string_beforelast, 0); lua51_settable(m_pL, -3);
	lua51_pushstring(m_pL, "formatself"); lua51_pushcclosure(m_pL, luaf_formatme, 0); lua51_settable(m_pL, -3);
	lua51_pop(m_pL, 1);

	lua51_getglobal(m_pL, "debug");
	lua51_pushstring(m_pL, "debug"); lua51_pushnil(m_pL); lua51_settable(m_pL, -3);
	lua51_pushnil(m_pL);
	while(lua51_next(m_pL, -2) != 0)
	{
		if(lua51_type(m_pL, -1) == LUA_TFUNCTION)
		{
			if( (lua51_type(m_pL, -2) != LUA_TSTRING) || (strcmp(lua51_tostring(m_pL, -2),"traceback") != 0) )
			{
				lua51_pushvalue(m_pL, -2);
				lua51_pushvalue(m_pL, -2);
				lua51_pushlightuserdata(m_pL, (void*)this);
				lua51_pushcclosure(m_pL, luaf_protect0, 2);
				lua51_getfenv(m_pL, -3);
				lua51_setfenv(m_pL, -2);
				lua51_settable(m_pL, -5);
			}
		}
		lua51_pop(m_pL, 1);
	}
	lua51_pop(m_pL, 1);

	lua51_getglobal(m_pL, "io");
	lua51_pushstring(m_pL, "stdin"); lua51_pushnil(m_pL); lua51_settable(m_pL, -3);
	lua51_pushstring(m_pL, "stdout"); lua51_pushnil(m_pL); lua51_settable(m_pL, -3);
	lua51_pushstring(m_pL, "stderr"); lua51_pushnil(m_pL); lua51_settable(m_pL, -3);
	lua51_pushnil(m_pL);
	while(lua51_next(m_pL, -2) != 0)
	{
		if(lua51_type(m_pL, -1) == LUA_TFUNCTION)
		{ // io k f
			lua51_pushvalue(m_pL, -2); // io k f k
			lua51_pushvalue(m_pL, -2); // io k f k f
			lua51_pushlightuserdata(m_pL, (void*)this); // io k f k f us
			lua51_pushcclosure(m_pL, luaf_protect1, 2); // io k f k pf
			lua51_getfenv(m_pL, -3); // io k f k pf env
			lua51_setfenv(m_pL, -2); // io k f k pf
			lua51_settable(m_pL, -5); // io k f
		}
		lua51_pop(m_pL, 1);
	}
	lua51_pop(m_pL, 1);

	lua51_getglobal(m_pL, "os");
	lua51_pushnil(m_pL);
	while(lua51_next(m_pL, -2) != 0)
	{
		if(lua51_type(m_pL, -1) == LUA_TFUNCTION)
		{
			lua51_pushvalue(m_pL, -2);
			lua51_pushvalue(m_pL, -2);
			lua51_pushlightuserdata(m_pL, (void*)this);
			lua51_pushcclosure(m_pL, luaf_protect2, 2);
			lua51_getfenv(m_pL, -3);
			lua51_setfenv(m_pL, -2);
			lua51_settable(m_pL, -5);
		}
		lua51_pop(m_pL, 1);
	}
	lua51_pop(m_pL, 1);

	lua51_getglobal(m_pL, "math");
	lua51_pushstring(m_pL, "formatself"); lua51_pushcclosure(m_pL, luaf_formatme, 0); lua51_settable(m_pL, -3);
	lua51_pushstring(m_pL, "clamp"); lua51_pushcclosure(m_pL, luax51_math_clamp, 0); lua51_settable(m_pL, -3);
	lua51_pop(m_pL, 1);

	int iTop3 = lua51_gettop(m_pL);

	int iLoadErr = lua51L_loadbuffer(m_pL, sCode, strlen(sCode), "macro");
	if(iLoadErr == LUA_ERRSYNTAX) throw new CRainmanException(0, __FILE__, __LINE__, "Syntax error in macro: %s", lua51_tostring(m_pL, -1));
	if(iLoadErr == LUA_ERRMEM) throw new CRainmanException(0, __FILE__, __LINE__, "Memory error in macro: %s", lua51_tostring(m_pL, -1));
	iLoadErr = lua51_pcall(m_pL, 0, 0, 0);
	if(iLoadErr)
	{
		if(lua51_type(m_pL, -1) == LUA_TLIGHTUSERDATA)
		{
			CRainmanException* pErr = (CRainmanException*)lua51_touserdata(m_pL, -1);
			lua51_pop(m_pL, 1);
			throw new CRainmanException(pErr, __FILE__, __LINE__, "Error loading macro");
		}
		else
		{
			const char* sErrM;
			switch(iLoadErr)
			{
				case LUA_ERRRUN: sErrM = "Runtime error loading macro : %s"; break;
				case LUA_ERRMEM: sErrM =  "Memory error loading macro : %s"; break;
				case LUA_ERRERR: sErrM =   "Error error loading macro : %s"; break;
				default        : sErrM = "Unknown error loading macro : %s"; break;
			}
			PAUSE_THROW(0, __FILE__, __LINE__, sErrM, lua51_tostring(m_pL, -1));
			lua51_pop(m_pL, 1);
			UNPAUSE_THROW;
		}
	}

	int iTop4 = lua51_gettop(m_pL);
	iTop4 =  iTop4;
}

int CRgdFileMacro::luaf_protect0(lua_State* L)
{
	CRgdFileMacro *self = (CRgdFileMacro*)lua51_touserdata(L, lua_upvalueindex(2));
	if(self->m_fpOnSecurityCallback && self->m_fpOnSecurityCallback(self->m_pCallbackTag, ST_DebugLib) == false)
	{
		return 0;
	}
	return lua51_tocfunction(L, lua_upvalueindex(1))(L);
}

int CRgdFileMacro::luaf_protect1(lua_State* L)
{
	CRgdFileMacro *self = (CRgdFileMacro*)lua51_touserdata(L, lua_upvalueindex(2));
	if(self->m_fpOnSecurityCallback && self->m_fpOnSecurityCallback(self->m_pCallbackTag, ST_IOLib) == false)
	{
		return 0;
	}
	return lua51_tocfunction(L, lua_upvalueindex(1))(L);
}

int CRgdFileMacro::luaf_protect2(lua_State* L)
{
	CRgdFileMacro *self = (CRgdFileMacro*)lua51_touserdata(L, lua_upvalueindex(2));
	if(self->m_fpOnSecurityCallback && self->m_fpOnSecurityCallback(self->m_pCallbackTag, ST_OSLib) == false)
	{
		return 0;
	}
	return lua51_tocfunction(L, lua_upvalueindex(1))(L);
}

void CRgdFileMacro::runAtEnd()
{
	if(m_pL == 0) return;
	lua51_getglobal(m_pL, "at_end");
	if(lua51_type(m_pL, -1) == LUA_TNIL)
	{
		lua51_pop(m_pL, 1);
		return;
	}

	int iErr;
	if(iErr = lua51_pcall(m_pL, 0, 0, 0))
	{
		if(lua51_type(m_pL, -1) == LUA_TLIGHTUSERDATA)
		{
			CRainmanException* pErr = (CRainmanException*)lua51_touserdata(m_pL, -1);
			lua51_pop(m_pL, 1);
			throw new CRainmanException(pErr, __FILE__, __LINE__, "Error running end event");
		}
		else
		{
			const char* sErrM;
			switch(iErr)
			{
				case LUA_ERRRUN: sErrM = "Runtime error running end event : %s"; break;
				case LUA_ERRMEM: sErrM =  "Memory error running end event : %s"; break;
				case LUA_ERRERR: sErrM =   "Error error running end event : %s"; break;
				default        : sErrM = "Unknown error running end event : %s"; break;
			}
			PAUSE_THROW(0, __FILE__, __LINE__, sErrM, lua51_tostring(m_pL, -1));
			lua51_pop(m_pL, 1);
			UNPAUSE_THROW;
		}
	}
}

void CRgdFileMacro::runMacro(const char* sFile, IFileStore* pStore)
{
	if(m_pL == 0) return;
	lua51_getglobal(m_pL, "each_file");
	if(lua51_type(m_pL, -1) == LUA_TNIL)
	{
		lua51_pop(m_pL, 1);
		return;
	}
	try
	{
		_tCRgdFile::construct(m_pL, this, sFile, pStore);
	}
	catch(CRainmanException *pE)
	{
		lua51_pop(m_pL, 1);
		throw new CRainmanException(pE, __FILE__, __LINE__, "Error pushing RGD \'%s\' into LUA", sFile);
	}

	int iErr;
	if(iErr = lua51_pcall(m_pL, 1, 0, 0))
	{
		if(lua51_type(m_pL, -1) == LUA_TLIGHTUSERDATA)
		{
			CRainmanException* pErr = (CRainmanException*)lua51_touserdata(m_pL, -1);
			lua51_pop(m_pL, 1);
			throw new CRainmanException(pErr, __FILE__, __LINE__, "Error processing RGD \'%s\'", sFile);
		}
		else
		{
			const char* sErrM;
			switch(iErr)
			{
				case LUA_ERRRUN: sErrM = "Runtime error processing RGD \'%s\' : %s"; break;
				case LUA_ERRMEM: sErrM =  "Memory error processing RGD \'%s\' : %s"; break;
				case LUA_ERRERR: sErrM =   "Error error processing RGD \'%s\' : %s"; break;
				default        : sErrM = "Unknown error processing RGD \'%s\' : %s"; break;
			}
			PAUSE_THROW(0, __FILE__, __LINE__, sErrM, sFile, lua51_tostring(m_pL, -1));
			lua51_pop(m_pL, 1);
			UNPAUSE_THROW;
		}
	}
}