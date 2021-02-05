#include "CLuaFile2.h"
extern "C" {
#include <lauxlib.h>
#include <lualib.h>
};
#include <string.h>
#include <stdlib.h>
#include "luax.h"
#include "memdebug.h"
#include "Exception.h"
#include "Internal_Util.h"
#include "..\zLib/zlib.h"
extern "C" {
typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned       char ub1;
ub4 hash(ub1 * k,ub4 length,ub4 initval);
ub4 hash3(ub1 * k,ub4 length,ub4 initval);
}
#include <algorithm>

CLuaFile2::CLuaFile2()
{
	m_pCache = 0;
	m_sFileName = 0;
	m_pRefMap = 0;
	m_bOwnCache = false;
	m_bOwnRefMap = false;
	m_sRootFolder = strdup("");
}

CLuaFile2::~CLuaFile2()
{
	_clean();
	if(m_bOwnCache && m_pCache) delete m_pCache;
	if(m_sRootFolder) free(m_sRootFolder);
}

void CLuaFile2::_clean()
{
	if(m_sFileName) free(m_sFileName);
	m_sFileName = 0;
	if(m_pRefMap && m_bOwnRefMap) delete m_pRefMap;
	m_bOwnRefMap = false;
}

bool CLuaFile2::_checkCache()
{
	if(m_pCache) return true;
	m_pCache = new CLuaFileCache;
	if(m_pCache) return m_bOwnCache = true;
	return false;
}

bool CLuaFile2::setCache(CLuaFileCache* pCache, bool bOwn)
{
	if(m_pCache) return false;
	m_pCache = pCache;
	m_bOwnCache = bOwn;
	return true;
}

void CLuaFile2::setRootFolder(const char* sRootFolder)
{
	if(m_sRootFolder) free(m_sRootFolder);
	m_sRootFolder = strdup(sRootFolder);
}

void CLuaFile2::newFile(const char* sFileName)
{
	_clean();
	if(!_checkCache()) throw new CRainmanException(__FILE__, __LINE__, "No cache");
	L = m_pCache->MakeState();
	if(!L) throw new CRainmanException(__FILE__, __LINE__, "Unable to create lua state");

	m_sFileName = CHECK_MEM(strdup(sFileName));

	luaopen_base(L);
    luaopen_string(L);
    luaopen_math(L);

	luax_DeleteGlobal(L, "dofile");
	luax_DeleteGlobal(L, "getfenv");
	luax_DeleteGlobal(L, "setfenv");
	luax_DeleteGlobal(L, "getmetatable");
	luax_DeleteGlobal(L, "setmetatable");
	luax_DeleteGlobal(L, "loadlib");
	luax_DeleteGlobal(L, "loadfile");
	luax_DeleteGlobal(L, "print");
	luax_DeleteGlobal(L, "rawget");
	luax_DeleteGlobal(L, "rawset");
	luax_DeleteGlobal(L, "require");
}

void CLuaFile2::loadFile(IFileStore::IStream *pStream, IFileStore* pFiles, const char* sFileName)
{
	if(pStream == 0 || pFiles == 0 || sFileName == 0) QUICK_THROW("Invalid argument")
	_AutoClean AutoClean_(this);

	try { newFile(sFileName); }
	CATCH_THROW("Error making blank slate")

	// Check for circular reference
	m_bOwnRefMap = false;
	if(!m_pRefMap)
	{
		m_pRefMap = CHECK_MEM(new _tRefMap);
		m_bOwnRefMap = true;
	}
	unsigned long iNameCrc = crc32_case_idt(0, (const Bytef *)sFileName, (uInt)strlen(sFileName));
	if((*m_pRefMap)[iNameCrc] > (int)1) throw new CRainmanException(0, __FILE__, __LINE__, "Circular reference detected (%s)", sFileName);
	++(*m_pRefMap)[iNameCrc];

	// Load
	long iDataLength;
	try
	{
		pStream->VSeek(0, IFileStore::IStream::SL_End);
		iDataLength  = pStream->VTell();
	}
	CATCH_THROW("Seek/Tell failed")

	char* sBuffer = CHECK_STR(new char[iDataLength]);
	AutoDelete<char> sBuffer_(sBuffer, true);

	try
	{
		pStream->VSeek(0, IFileStore::IStream::SL_Root);
		pStream->VRead(1, (unsigned long)iDataLength, sBuffer);
	}
	CATCH_THROW("Seek/Read failed")
	int iLuaError = luaL_loadbuffer(L, sBuffer, iDataLength, sFileName);
	sBuffer_.del();
	if(iLuaError == LUA_ERRMEM) QUICK_THROW("Lua memory error")
	else if(iLuaError == LUA_ERRSYNTAX) throw new CRainmanException(0, __FILE__, __LINE__, "Lua error (LUA_ERRSYNTAX): %s", lua_tostring(L, -1));

	// Prepare lua state
	#define quick_fn_register(n,f) lua_pushstring(L, n), \
	lua_pushlightuserdata(L, (void*)pFiles), \
	lua_pushcclosure(L, f, 1), \
	lua_settable(L, LUA_GLOBALSINDEX);
	quick_fn_register("Inherit", _luaInherit);
	quick_fn_register("InheritMeta", _luaInheritMeta);
	quick_fn_register("Reference", _luaReference);
	#undef quick_fn_register
	lua_pushstring(L, "CLuaFile2");
	lua_pushlightuserdata(L, (void*)this); // push 1 : +1
	lua_settable(L, LUA_GLOBALSINDEX);

	// Run
	iLuaError = lua_pcall(L, 0, 0, 0);
	if(iLuaError)
	{
		switch(lua_type(L, -1))
		{
		case LUA_TSTRING:
			throw new CRainmanException(0, __FILE__, __LINE__, "Lua error (%i): %s", iLuaError, lua_tostring(L, -1));
		case LUA_TLIGHTUSERDATA:
			throw new CRainmanException((CRainmanException*) lua_touserdata(L, -1), __FILE__, __LINE__, "Lua exception (%i)", iLuaError);
		default:
			throw new CRainmanException(0, __FILE__, __LINE__, "Lua unknown error (%i)", iLuaError);
		};
	}

	// Todo: parse stuff out
}

CLuaFile2* CLuaFile2::_getOwner(lua_State* L)
{
	lua_pushstring(L, "CLuaFile2");
	lua_gettable(L, LUA_GLOBALSINDEX);
	CLuaFile2* p = (CLuaFile2*)lua_touserdata(L, -1);
	lua_pop(L, 1);
	return p;
}

int CLuaFile2::_luaInherit(lua_State* L)
{
	return _getOwner(L)->_luaParent("Inherit", "GameData");
}

int CLuaFile2::_luaInheritMeta(lua_State* L)
{
	return _getOwner(L)->_luaParent("InheritMeta", "MetaData");
}

int CLuaFile2::_luaReference(lua_State* L)
{
	return _getOwner(L)->_luaParent("Reference", "GameData");
}

static void luaEnquiry(lua_State* L, int i)
{
	int iType = lua_type(L, i);
	const char* sType = 0, *sVal = 0;
	if(iType == LUA_TNONE) sType = "none";
	if(iType == LUA_TNIL) sType = "nil";
	if(iType == LUA_TNUMBER) sType = "num";
	if(iType == LUA_TBOOLEAN) sType = "bool";
	if(iType == LUA_TSTRING){ sType = "str"; sVal = lua_tostring(L, i);}
	if(iType == LUA_TTABLE) sType = "table";
	if(iType == LUA_TFUNCTION) sType = "fn";
	if(iType == LUA_TUSERDATA) sType = "userd";
	if(iType == LUA_TTHREAD) sType = "thread";
	if(iType == LUA_TLIGHTUSERDATA) sType = "ptr";
}

int CLuaFile2::_luaIndexEvent(lua_State* L)
{
	/*
		Equivalent code in lua:
		function _luaIndexEvent(T, K) -- T doesn't have K on T[K]
			local V = T["$PARENT"][K] -- Look in the inherited table for K
			if(type(V) ~= "table") return temp -- Return straight away any simple objects
			local R = {} -- Table to buffer access to V
			T.K = R -- Save R into T for next time
			R.$PARENT = V
			R.$REF = $T.REF -- So we know where V came from
			R.$FUNC = "" -- So we know how $PARENT came about
			R.(metatable).__index = _luaIndexEvent
			return R
		end
	*/

	// local V = T["$PARENT"][K]
	lua_pushstring(L, "$PARENT"); // "$PARENT" K T
	lua_gettable(L, -3); // $PARENT K T
	lua_insert(L, -2); // K $PARENT T
	lua_pushvalue(L, -1); // K K $PARENT T
	lua_gettable(L, -3); // V K $PARENT T
	// if(type(V) ~= "table") return V
	if(lua_istable(L, -1) == 0) return 1;
	lua_insert(L, -2); // K V $PARENT T
	// local R = {}
	lua_newtable(L); // R K V $PARENT T
	// T.K = R
	lua_insert(L, -2); // K R V $PARENT T
	lua_pushvalue(L, -2); // R K R V $PARENT T
	lua_settable(L, -6);
	// R.$PARENT = V
	lua_pushstring(L, "$PARENT"); // "$PARENT" R V $PARENT T
	lua_pushvalue(L, -3); // V "$PARENT" R V $PARENT T
	lua_settable(L, -3); // R V $PARENT T
	// R.$REF = $T.REF
	lua_pushstring(L, "$REF"); // "$REF" R V $PARENT T
	lua_pushvalue(L, -1); // "$REF" "$REF" R V $PARENT T
	lua_gettable(L, -6); // $REF "$REF" R V $PARENT T
	lua_settable(L, -3); // R V $PARENT T
	// R.$FUNC = ""
	lua_pushstring(L, "$FUNC"); // "$FUNC" R V $PARENT T
	lua_pushstring(L, ""); // "" "$FUNC" R V $PARENT T
	lua_settable(L, -3); // R V $PARENT T
	// R.(metatable).__index = _luaIndexEvent
	lua_newtable(L);
	lua_pushstring(L, "__index");
	lua_pushcclosure(L, _luaIndexEvent, 0);
	lua_settable(L, -3);
	lua_setmetatable(L, -2);
	// return R
	return 1;
}

int CLuaFile2::_luaParent(const char* sFnName, const char* sTableToGrab)
{
	try
	{
		// Grab other required stuff
		IFileStore* pFiles = (IFileStore*)lua_touserdata(L, lua_upvalueindex(1));
		const char* sFileName = lua_tostring(L, -1);
		if(pFiles == 0 || sFileName == 0) QUICK_THROW("Invalid argument")

		// Load parented file
		CLuaFile2 oParentFile;
		if( (oParentFile.L = m_pCache->Fetch(sFileName)) == 0)
		{
			if(sFileName[0] == 0)
			{
				oParentFile.L = m_pCache->MakeState();
				lua_pushstring(oParentFile.L, sTableToGrab);
				lua_newtable(oParentFile.L);
				lua_settable(oParentFile.L, LUA_GLOBALSINDEX);
			}
			else
			{
				// Open parented file
				char* sFileNameFull = CHECK_MEM(new char[strlen(sFileName) + strlen(m_sRootFolder) + 1]);
				AutoDelete<char> sFileNameFull_(sFileNameFull, true);
				strcpy(sFileNameFull, m_sRootFolder);
				strcat(sFileNameFull, sFileName);
				IFileStore::IStream* pFileIn = 0;
				try {pFileIn = pFiles->VOpenStream(sFileNameFull);}
				catch (CRainmanException *pE) {throw new CRainmanException(pE, __FILE__, __LINE__, "Unable to open file \'%s\'", sFileNameFull);}
				AutoDelete<IFileStore::IStream> pFileIn_(pFileIn, false);

				// Continue loading
				oParentFile.setCache(m_pCache, false);
				oParentFile.setRootFolder(m_sRootFolder);
				oParentFile.m_pRefMap = m_pRefMap;
				try{oParentFile.loadFile(pFileIn, pFiles, sFileName);}
				catch (CRainmanException *pE) {throw new CRainmanException(pE, __FILE__, __LINE__, "Error loading file \'%s\'", sFileNameFull);}
				pFileIn_.del();

				try {m_pCache->AddToCache(sFileName, oParentFile.L);} IGNORE_EXCEPTIONS
			}
		}

		// Move parented data over
		/*
			T
			|-$PARENT = GameData or MetaData from $REF
			|-$REF = "tables\xyz_table.lua" or similar
			|-$FUNC = "Inherit" or "Reference" or "InheritMeta"
			|-(metatable)
			  |-__index = _luaIndexEvent
		*/
		lua_newtable(L); // (our T)
		lua_pushstring(L, "$PARENT"); // "$PARENT" (our T)
		lua_pushstring(oParentFile.L, sTableToGrab);
		lua_gettable(oParentFile.L, LUA_GLOBALSINDEX);
		lua_xmove(oParentFile.L, L, 1); // (parent T) "$PARENT" (our T)
		lua_settable(L, -3); // (our T)
		lua_pushstring(L, "$REF"); // "$REF" (our T)
		lua_pushstring(L, sFileName); // "(parent filename)" "$REF" (our T)
		lua_settable(L, -3); // (our T)
		lua_pushstring(L, "$FUNC"); // "$FUNC" (our T)
		lua_pushstring(L, sFnName); // "(function)" "$FUNC" (our T)
		lua_settable(L, -3); // (our T)
		lua_newtable(L); // (metatable) (our T)
		lua_pushstring(L, "__index"); // ("__index") (metatable) (our T)
		lua_pushcclosure(L, _luaIndexEvent, 0); // _luaIndexEvent ("__index") (metatable) (our T)
		lua_settable(L, -3); // (metatable) (our T)
		lua_setmetatable(L, -2); // (our T)

		// Move any exceptions into lua land ...
	}
	catch(CRainmanException *pE)
	{
		lua_pushlightuserdata(L, (void*)pE);
		lua_error(L);
	}
	return 1;
}

CLuaFile2::_LuaLocator::_LuaLocator(CLuaFile2::_LuaLocator* P)
{
	pRefCount = P->pRefCount;
	++(*pRefCount);
}

CLuaFile2::_LuaLocator::_LuaLocator(lua_State* L, int Index)
{
	pRefCount = new unsigned long;
	(*pRefCount) = 1;

	lua_pushvalue(L, Index); // V
	lua_pushlightuserdata(L, (void*)pRefCount); // K V
	lua_insert(L, -2); // V K
	lua_settable(L, LUA_REGISTRYINDEX); //
}

void CLuaFile2::_LuaLocator::kill(lua_State* L)
{
	if((*pRefCount) == 1)
	{
		delete pRefCount;
		lua_pushlightuserdata(L, (void*)pRefCount);
		lua_pushnil(L);
		lua_settable(L, LUA_REGISTRYINDEX);
	}
	else
	{
		--(*pRefCount);
	}
}

void CLuaFile2::_LuaLocator::push(lua_State* L)
{
	lua_pushlightuserdata(L, (void*)pRefCount);
	lua_gettable(L, LUA_REGISTRYINDEX);
}


CLuaStateNode::CLuaStateNode(lua_State* pL, int iVal, int iKey)
:m_L(pL, iVal)
{
	L = pL;
	sName = 0;
	lua_pushvalue(pL, iKey);
	sName = strdup(lua_tostring(pL, -1));
	lua_pop(pL, 1);
}

CLuaStateNode::~CLuaStateNode()
{
	m_L.kill(L);
	free(sName);
}

IMetaNode::eDataTypes CLuaStateNode::VGetType()
{
	m_L.push(L);
	eDataTypes eRet = DT_NoData;
	switch(lua_type(L, -1))
	{
	case LUA_TBOOLEAN:
		eRet = DT_Bool;
		break;

	case LUA_TNUMBER:
		eRet = DT_Float;
		break;

	case LUA_TSTRING:
		eRet = DT_String;
		break;

	case LUA_TTABLE:
		eRet = DT_Table;
		break;
	};
	lua_pop(L, 1);
	return eRet;
}

const char* CLuaStateNode::VGetName()
{
	return sName;
}

unsigned long CLuaStateNode::VGetNameHash()
{
	return (unsigned long) hash((ub1*) sName, (ub4)strlen(sName), 0);
}

float CLuaStateNode::VGetValueFloat()
{
	m_L.push(L);
	if(lua_type(L, -1) != LUA_TNUMBER)
	{
		lua_pop(L, 1);
		QUICK_THROW("Wrong type")
	}
	float fT = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return fT;
}

unsigned long CLuaStateNode::VGetValueInteger()
{
	QUICK_THROW("Wrong type")
}

bool CLuaStateNode::VGetValueBool()
{
	m_L.push(L);
	if(lua_type(L, -1) != LUA_TBOOLEAN)
	{
		lua_pop(L, 1);
		QUICK_THROW("Wrong type")
	}
	bool r = lua_toboolean(L, -1);
	lua_pop(L, 1);
	return r;
}

const char* CLuaStateNode::VGetValueString()
{
	m_L.push(L);
	if(lua_type(L, -1) != LUA_TSTRING)
	{
		lua_pop(L, 1);
		QUICK_THROW("Wrong type")
	}
	const char* r = lua_tostring(L, -1);
	lua_pop(L, 1);
	return r;
}

const wchar_t* CLuaStateNode::VGetValueWString()
{
	QUICK_THROW("Wrong type")
}

IMetaNode::IMetaTable* CLuaStateNode::VGetValueMetatable()
{
	m_L.push(L);
	if(lua_type(L, -1) != LUA_TTABLE)
	{
		lua_pop(L, 1);
		QUICK_THROW("Wrong type")
	}
	IMetaNode::IMetaTable* r = new CLuaStateTable(L);
	lua_pop(L, 1);
	return r;
}

void CLuaStateNode::VSetType(eDataTypes eType) {QUICK_THROW("Unsupported");}
void CLuaStateNode::VSetName(const char* sName){QUICK_THROW("Unsupported");}
void CLuaStateNode::VSetNameHash(unsigned long iHash){QUICK_THROW("Unsupported");}
void CLuaStateNode::VSetValueFloat(float fValue){QUICK_THROW("Unsupported");}
void CLuaStateNode::VSetValueInteger(unsigned long iValue){QUICK_THROW("Unsupported");}
void CLuaStateNode::VSetValueBool(bool bValue){QUICK_THROW("Unsupported");}
void CLuaStateNode::VSetValueString(const char* sValue){QUICK_THROW("Unsupported");}
void CLuaStateNode::VSetValueWString(const wchar_t* wsValue){QUICK_THROW("Unsupported");}

CMemoryStore::COutStream* CLuaStateNode::VGetNodeAsRainmanRgd(){QUICK_THROW("Unsupported");}
void CLuaStateNode::SGetNodeFromRainmanRgd(IFileStore::IStream* pInput, bool bSetName){QUICK_THROW("Unsupported");}

CLuaStateTable::CLuaStateTable(lua_State *L, bool bCustom)
{
	mL = L;
	bDeleteNodes = !bCustom;
	if(!bCustom)
	{
		lua_pushnil(L);
		while (lua_next(L, -2) != 0)
		{
			m_vNodes.push_back(new CLuaStateNode(L, -1, -2));
			lua_pop(L, 1);
		}
	}
}

CLuaStateTable::~CLuaStateTable()
{
	if(bDeleteNodes)
	{
		for(std::vector<CLuaStateNode*>::iterator itr = m_vNodes.begin(); itr != m_vNodes.end(); ++itr)
		{
			delete *itr;
		}
	}
}

void CLuaStateTable::add(CLuaStateNode* p)
{
	m_vNodes.push_back(p);
}

unsigned long CLuaStateTable::VGetChildCount()
{
	return (unsigned long)m_vNodes.size();
}

IMetaNode::IMetaTable* CLuaFile2::asMetaTable()
{
	lua_pushstring(L, "_G");
	lua_gettable(L, LUA_GLOBALSINDEX);
	CTable* p = new CTable(L, true);
	lua_pop(L, 1);
	return p;
}

IMetaNode* CLuaStateTable::VGetChild(unsigned long iIndex)
{
	CLuaStateNode* pCur = m_vNodes[iIndex];

	lua_pushstring(mL, pCur->sName);
	pCur->m_L.push(mL);
	CLuaStateNode* pNew = new CLuaStateNode(mL, -1, -2);
	lua_pop(mL, 2);
	return pNew;
}

IMetaNode::eDataTypes CLuaStateTable::VGetReferenceType()
{
	return IMetaNode::DT_NoData;
}
const char* CLuaStateTable::VGetReferenceString() {return "";}
const wchar_t* CLuaStateTable::VGetReferenceWString() {return L"";}

void CLuaStateTable::VSetReferenceType(IMetaNode::eDataTypes eType){QUICK_THROW("Unsupported");}
void CLuaStateTable::VSetReferenceString(const char* sValue){QUICK_THROW("Unsupported");}
void CLuaStateTable::VSetReferenceWString(const wchar_t* wsValue){QUICK_THROW("Unsupported");}
IMetaNode* CLuaStateTable::VAddChild(const char* sName){QUICK_THROW("Unsupported");}
void CLuaStateTable::VDeleteChild(unsigned long iIndex){QUICK_THROW("Unsupported");}

CLuaStateStackNode::CLuaStateStackNode(lua_State* L)
{
	m_L = L;

	// Make faux nodes for each value in L's stack
	for(int i=-1;;i-=1)
	{
		if(lua_type(L, i) == LUA_TNONE) break;
		char sNam[4];
		sNam[0] = '-';
		sNam[1] = '0' + ((i < -9) ? (-i)/10 : -i);
		sNam[2] = (i < -9) ? ('0' + ((-i)%10)) : 0;
		sNam[3] = 0;
		lua_pushstring(L, sNam);
		m_vNodes.push_back(new CLuaStateNode(L, i-1, -1));
		lua_pop(L, 1);
	}
}

CLuaStateStackNode::~CLuaStateStackNode()
{
	for(std::vector<CLuaStateNode*>::iterator itr = m_vNodes.begin(); itr != m_vNodes.end(); ++itr)
	{
		delete *itr;
	}
}

IMetaNode::eDataTypes CLuaStateStackNode::VGetType() {return IMetaNode::DT_Table;}
const char* CLuaStateStackNode::VGetName() {return "STACK";}
unsigned long CLuaStateStackNode::VGetNameHash() {return (unsigned long) hash((ub1*)"STACK", (ub4)5, 0);}
float CLuaStateStackNode::VGetValueFloat() {QUICK_THROW("Unsupported");}
unsigned long CLuaStateStackNode::VGetValueInteger() {QUICK_THROW("Unsupported");}
bool CLuaStateStackNode::VGetValueBool() {QUICK_THROW("Unsupported");}
const char* CLuaStateStackNode::VGetValueString() {QUICK_THROW("Unsupported");}
const wchar_t* CLuaStateStackNode::VGetValueWString(){QUICK_THROW("Unsupported");}
IMetaNode::IMetaTable* CLuaStateStackNode::VGetValueMetatable()
{
	CLuaStateTable* p = new CLuaStateTable(m_L, true);
	for(std::vector<CLuaStateNode*>::iterator itr = m_vNodes.begin(); itr != m_vNodes.end(); ++itr)
	{
		p->add(*itr);
	}
	return p;
}

void CLuaStateStackNode::VSetType(eDataTypes eType){QUICK_THROW("Unsupported");}
void CLuaStateStackNode::VSetName(const char* sName){QUICK_THROW("Unsupported");}
void CLuaStateStackNode::VSetNameHash(unsigned long iHash){QUICK_THROW("Unsupported");}
void CLuaStateStackNode::VSetValueFloat(float fValue){QUICK_THROW("Unsupported");}
void CLuaStateStackNode::VSetValueInteger(unsigned long iValue){QUICK_THROW("Unsupported");}
void CLuaStateStackNode::VSetValueBool(bool bValue){QUICK_THROW("Unsupported");}
void CLuaStateStackNode::VSetValueString(const char* sValue){QUICK_THROW("Unsupported");}
void CLuaStateStackNode::VSetValueWString(const wchar_t* wsValue){QUICK_THROW("Unsupported");}
CMemoryStore::COutStream* CLuaStateStackNode::VGetNodeAsRainmanRgd(){QUICK_THROW("Unsupported");}
void CLuaStateStackNode::SGetNodeFromRainmanRgd(IFileStore::IStream* pInput, bool bSetName){QUICK_THROW("Unsupported");}

CLuaFile2::CNode::CNode(lua_State* Lua, int iKey, int iTable) : m_oTablePtr(Lua, iTable), m_oKeyPtr(Lua, iKey) {}
CLuaFile2::CNode::CNode(lua_State* Lua, CLuaFile2::_LuaLocator* pTable, CLuaFile2::_LuaLocator* pKey) : m_oTablePtr(pTable), m_oKeyPtr(pKey) {}

bool CLuaFile2::CTable::VSupportsRefresh() {return true;}
void CLuaFile2::CTable::VDoRefresh()
{
	if(m_sRef) free(m_sRef);
	m_sRef = 0;
	for(std::vector<CLuaFile2::CNode*>::iterator itr = m_vNodes.begin(); itr != m_vNodes.end(); ++itr)
	{
		delete *itr;
	}
	m_vNodes.clear();

	m_oTablePtr.push(L);
	_DoLoad();
	lua_pop(L, 1);
}

void CLuaFile2::CTable::_DoLoad()
{// T
	lua_pushvalue(L, -1); // T T
	do
	{
		lua_pushnil(L); // nil T T
		while (lua_next(L, -2) != 0)
		{ // V K T T
			// get name / hash
			lua_pushvalue(L, -2); // K V K T T
			const char* sName = lua_tostring(L, -1);
			unsigned long iHash = (unsigned long) hash((ub1*) sName, (ub4)strlen(sName), 0);

			if(sName[0] == '$')
			{
				if(iHash == 0x1DA0FE3C) // $FUNC
				{
					if(m_sRef == 0 && lua_strlen(L, -2))
					{
						lua_pushstring(L, "$REF"); // "$REF" K V K T T
						lua_gettable(L, -5); // $REF K V K T T
						m_sRef = strdup(lua_tostring(L, -1));
						lua_pop(L, 1); // K V K T T
					}
					lua_pop(L, 1); // V K T T
					goto try_next;
				}
				else if(iHash == 0x7BEEC89D || iHash == 0x49D60FAE) // $PARENT , $REF
				{
					lua_pop(L, 1); // V K T T
					goto try_next;
				}
			}
			if(m_bGlobals)
			{
				if(/*iHash == 0xCB4DD18B || */iHash == 0x417ACE4F || iHash == 0x2DF117FB) // MetaData, _G , CLuaFile2
				{
					lua_pop(L, 1); // V K T T
					goto try_next;
				}
			}

			// look for existing
			for(std::vector<CLuaFile2::CNode*>::iterator itr = m_vNodes.begin(); itr != m_vNodes.end(); ++itr)
			{
				if((**itr).m_iNodeHash == iHash)
				{
					lua_pop(L, 1); // V K T T
					goto try_next;
				}
			}
			
			// add to node list
			// K V K T T
			CLuaFile2::CNode* pNode = new CLuaFile2::CNode(L, -3, -4);
			pNode->L = L;
			pNode->m_sName = strdup(sName);
			pNode->m_iNodeHash = iHash;
			pNode->m_iType = lua_type(L, -2);
			m_vNodes.push_back(pNode);
			lua_pop(L, 1); // V K T T

			try_next: lua_pop(L, 1); // K T T
		} // T T

		// move to $PARENT
		lua_pushstring(L, "$PARENT"); // "$PARENT" T T
		lua_gettable(L, -2); // $PARENT T T
		lua_replace(L, -2); // $PARENT T
	} while(lua_type(L, -1) == LUA_TTABLE);

	std::sort(m_vNodes.begin(), m_vNodes.end(), _SortNodes);

	lua_pop(L, 1); // T
}

CLuaFile2::CTable::CTable(lua_State *pL, bool bG)
:m_oTablePtr(pL, -1), m_sRef(0), m_bGlobals(bG), L(pL)
{
	_DoLoad();
}

bool CLuaFile2::_SaveKey::_Sort(_SaveKey* p1, _SaveKey* p2)
{
	if(p1->iWhat != p2->iWhat) return (p1->iWhat < p2->iWhat);
	if(p1->iWhat == 1) return p1->f < p2->f;
	else if(p1->iWhat == 3) return p1->b < p2->b;

	if(p1->s == 0 || p2->s == 0)
	{
		if(p1->s == 0 && p2->s != 0) return true;
		return false;
	}

	const char* sNumBegin1, *sNumBegin2;
	size_t iLen1 = strlen(p1->s), iLen2 = strlen(p2->s);

	while(iLen1 > 0 && p1->s[iLen1 - 1] >= '0' && p1->s[iLen1 - 1] <= '9') --iLen1;
	sNumBegin1 = p1->s + iLen1;
	while(iLen2 > 0 && p2->s[iLen2 - 1] >= '0' && p2->s[iLen2 - 1] <= '9') --iLen2;
	sNumBegin2 = p2->s + iLen2;

	if(iLen1 != iLen2) return (stricmp(p1->s, p2->s) < 0);
	int iCmp = strnicmp(p1->s, p2->s, iLen1);
	if(iCmp != 0) return iCmp;
	return atol(sNumBegin1) < atol(sNumBegin2);
}

bool CLuaFile2::CTable::_SortNodes(CLuaFile2::CNode* p1, CLuaFile2::CNode* p2)
{
	if(p1->m_sName == 0 || p2->m_sName == 0)
	{
		if(p1->m_sName == 0 && p2->m_sName != 0) return true;
		return false;
	}

	char* sNumBegin1, *sNumBegin2;
	size_t iLen1 = strlen(p1->m_sName), iLen2 = strlen(p2->m_sName);

	while(iLen1 > 0 && p1->m_sName[iLen1 - 1] >= '0' && p1->m_sName[iLen1 - 1] <= '9') --iLen1;
	sNumBegin1 = p1->m_sName + iLen1;
	while(iLen2 > 0 && p2->m_sName[iLen2 - 1] >= '0' && p2->m_sName[iLen2 - 1] <= '9') --iLen2;
	sNumBegin2 = p2->m_sName + iLen2;

	if(iLen1 != iLen2) return (stricmp(p1->m_sName, p2->m_sName) < 0);
	int iCmp = strnicmp(p1->m_sName, p2->m_sName, iLen1);
	if(iCmp != 0) return iCmp;
	return atol(sNumBegin1) < atol(sNumBegin2);
}

CLuaFile2::CTable::~CTable()
{
	m_oTablePtr.kill(L);
	if(m_sRef) free(m_sRef);
	for(std::vector<CLuaFile2::CNode*>::iterator itr = m_vNodes.begin(); itr != m_vNodes.end(); ++itr)
	{
		delete *itr;
	}
}

unsigned long CLuaFile2::CTable::VGetChildCount()
{
	return m_vNodes.size();
}

IMetaNode* CLuaFile2::CTable::VGetChild(unsigned long iIndex)
{
	CLuaFile2::CNode* pNode = m_vNodes[iIndex];
	CLuaFile2::CNode* p = new CLuaFile2::CNode(L, &pNode->m_oTablePtr, &pNode->m_oKeyPtr);
	p->L = L;
	p->m_sName = strdup(pNode->m_sName);
	p->m_iNodeHash = pNode->m_iNodeHash;
	p->m_iType = pNode->m_iType;

	return p;
}

IMetaNode::eDataTypes CLuaFile2::CTable::VGetReferenceType()
{
	return m_sRef ? IMetaNode::DT_String : IMetaNode::DT_NoData;
}

const char* CLuaFile2::CTable::VGetReferenceString()
{
	return m_sRef;
}

const wchar_t* CLuaFile2::CTable::VGetReferenceWString() {QUICK_THROW("Invalid type")}
void CLuaFile2::CTable::VSetReferenceType(IMetaNode::eDataTypes eType) {QUICK_THROW("TODO")}
void CLuaFile2::CTable::VSetReferenceString(const char* sValue) {QUICK_THROW("TODO")}
void CLuaFile2::CTable::VSetReferenceWString(const wchar_t* wsValue) {QUICK_THROW("TODO")}

IMetaNode* CLuaFile2::CTable::VAddChild(const char* sName)
{
	m_oTablePtr.push(L);
	lua_pushstring(L, sName);
	lua_gettable(L, -2);
	if(lua_type(L, -1) != LUA_TNIL)
	{
		lua_pop(L, 2);
		QUICK_THROW("Child with that name already present")
	}
	lua_pop(L, 1);
	lua_pushstring(L, sName);
	lua_pushstring(L, "");
	lua_settable(L, -3);

	lua_pushstring(L, sName);
	CLuaFile2::_LuaLocator oL(L, -1);
	lua_pop(L, 2);

	CLuaFile2::CNode *pNode = new CLuaFile2::CNode(L, &m_oTablePtr, &oL);
	pNode->L = L;
	pNode->m_iType = LUA_TSTRING;
	pNode->m_sName = strdup(sName);
	pNode->m_iNodeHash = (unsigned long) hash((ub1*) sName, (ub4)strlen(sName), 0);

	m_vNodes.push_back(pNode);
	IMetaNode* pINode = VGetChild(m_vNodes.size() - 1);
	std::sort(m_vNodes.begin(), m_vNodes.end(), _SortNodes);
	return pINode;
}

void CLuaFile2::CTable::VDeleteChild(unsigned long iIndex)
{
	CLuaFile2::CNode *pNode = m_vNodes[iIndex];

	pNode->_pushTable(); // T
	lua_pushstring(L, "$PARENT"); // "$PARENT" T
	lua_gettable(L, -2); // $PARENT T
	pNode->_pushKey(); // K $PARENT T
	lua_gettable(L, -2); // V $PARENT T
	if(lua_type(L, -1) != LUA_TNIL)
	{
		lua_pop(L, 3);
		QUICK_THROW("Cannot delete as is set in Inherit()ed or Reference()d file")
	}
	lua_pop(L, 2); // T
	pNode->_pushKey(); // K T
	lua_pushnil(L); // nil K T
	lua_settable(L, -3); // T
	lua_pop(L, 1);

	m_vNodes.erase(m_vNodes.begin() + iIndex);
}

CLuaFile2::CNode::~CNode()
{
	m_oTablePtr.kill(L);
	m_oKeyPtr.kill(L);
	free(m_sName);
}

CLuaFile2::CNode::eDataTypes CLuaFile2::CNode::VGetType()
{
	eDataTypes eRet = DT_NoData;
	switch(m_iType)
	{
	case LUA_TBOOLEAN:
		eRet = DT_Bool;
		break;

	case LUA_TNUMBER:
		eRet = DT_Float;
		break;

	case LUA_TSTRING:
		eRet = DT_String;
		break;

	case LUA_TTABLE:
		eRet = DT_Table;
		break;
	};
	return eRet;
}

const char* CLuaFile2::CNode::VGetName()
{
	return m_sName;
}
unsigned long CLuaFile2::CNode::VGetNameHash()
{
	return m_iNodeHash;
}

void CLuaFile2::CNode::_pushVal()
{
	_pushTable();
	_pushKey();
	lua_gettable(L, -2);
	lua_replace(L, -2);
}

float CLuaFile2::CNode::VGetValueFloat()
{
	if(m_iType != LUA_TNUMBER) QUICK_THROW("Wrong type")
	_pushVal();
	float V = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return V;
}

unsigned long CLuaFile2::CNode::VGetValueInteger()
{
	QUICK_THROW("Wrong type")
}

bool CLuaFile2::CNode::VGetValueBool()
{
	if(m_iType != LUA_TBOOLEAN) QUICK_THROW("Wrong type")
	_pushVal();
	bool V = lua_toboolean(L, -1);
	lua_pop(L, 1);
	return V;
}

const char* CLuaFile2::CNode::VGetValueString()
{
	if(m_iType != LUA_TSTRING) QUICK_THROW("Wrong type")
	_pushVal();
	const char* V = lua_tostring(L, -1);
	lua_pop(L, 1);
	return V;
}

const wchar_t* CLuaFile2::CNode::VGetValueWString()
{
	QUICK_THROW("Wrong type")
}

IMetaNode::IMetaTable* CLuaFile2::CNode::VGetValueMetatable()
{
	if(m_iType != LUA_TTABLE) QUICK_THROW("Wrong type")
	_pushVal();
	IMetaNode::IMetaTable* V = new CLuaFile2::CTable(L);
	lua_pop(L, 1);
	return V;
}

void CLuaFile2::CNode::_pushTable()
{
	m_oTablePtr.push(L);
}

void CLuaFile2::CNode::_pushKey()
{
	m_oKeyPtr.push(L);
	/*
	// check if number
	char* sTmp = m_sName;
	do
	{
		if( (sTmp < '0' || sTmp > '9') && sTmp != '.' ) goto not_a_number;
		++sTmp;
	}while(*sTmp);
	lua_Number f = (lua_Number)atof(m_sName);
	lua_pushnumber(m_oNodePtr.getL(), f);
	return;
not_a_number:
	lua_pushstring(m_oNodePtr.getL(), m_sName);
	return;
	*/
}

void CLuaFile2::CNode::VSetType(eDataTypes eType)
{
	int iTargetLuaType = LUA_TNONE;
	switch(eType)
	{
	case IMetaNode::DT_Bool: iTargetLuaType = LUA_TBOOLEAN; break;
	case IMetaNode::DT_Float: iTargetLuaType = LUA_TNUMBER; break;
	case IMetaNode::DT_String: iTargetLuaType = LUA_TSTRING; break;
	case IMetaNode::DT_Table: iTargetLuaType = LUA_TTABLE; break;
	default: QUICK_THROW("Data type not compatible with LUA")
	};
	if(iTargetLuaType == m_iType) return;
	if(iTargetLuaType != LUA_TTABLE && m_iType != LUA_TTABLE)
	{
		// simple data conversion
		_pushTable();
		_pushKey();
		lua_pushvalue(L, -1);
		lua_gettable(L, -3);
		if(iTargetLuaType == LUA_TBOOLEAN) lua_toboolean(L, -1);
		else if(iTargetLuaType == LUA_TNUMBER) lua_tonumber(L, -1);
		else lua_tostring(L, -1);
		if(lua_type(L, -1) != iTargetLuaType)
		{
			lua_pop(L, 1);
			if(iTargetLuaType == LUA_TBOOLEAN) lua_pushboolean(L, 1);
			else if(iTargetLuaType == LUA_TNUMBER) lua_pushnumber(L, 0.0);
			else lua_pushstring(L, "");
		}
		m_iType = lua_type(L, -1);
		lua_settable(L, -3);
		lua_pop(L, 1);
	}
	else if(iTargetLuaType == LUA_TTABLE)
	{
		// from simple to table
		_pushTable();
		_pushKey();

		if(m_iNodeHash == 0xF693C657) lua_pushstring(L, "Inherit");
		else if(m_iNodeHash == 0xCB4DD18B)  lua_pushstring(L, "InheritMeta");
		else lua_pushstring(L, "Reference");
		lua_gettable(L, LUA_GLOBALSINDEX);
		lua_pushstring(L, "");
		lua_call(L, 1, 1);

		lua_settable(L, -3);
		lua_pop(L, 1);

		m_iType = LUA_TTABLE;
	}
	else
	{
		// from table to simple
		QUICK_THROW("TODO")
	}
}

void CLuaFile2::CNode::VSetName(const char* sName)
{
	_pushTable(); // T
	lua_pushstring(L, sName); // NK T
	lua_gettable(L, -2); // V T
	if(lua_type(L, -1) != LUA_TNIL)
	{
		lua_pop(L, 2);
		QUICK_THROW("Something with that name already exists")
	}
	lua_pop(L, 1);

	_pushKey(); // K T
	lua_pushvalue(L, -1); // K K T
	lua_gettable(L, -3); // V K T
	lua_insert(L, -3); // K T V
	lua_pushnil(L); // nil K T V
	lua_settable(L, -3); // T V
	lua_pushstring(L, sName); // K T V
	lua_pushvalue(L, -3); // V K T V
	lua_settable(L, -3); // T V
	lua_pop(L, 2);

	free(m_sName);
	m_sName = strdup(sName);
	m_iNodeHash = (unsigned long) hash((ub1*) sName, (ub4)strlen(sName), 0);
}
void CLuaFile2::CNode::VSetNameHash(unsigned long iHash){QUICK_THROW("Invalid operation for LUA")}

void CLuaFile2::CNode::VSetValueFloat(float fValue)
{
	if(m_iType != LUA_TNUMBER) QUICK_THROW("Invalid type")
	_pushTable();
	_pushKey();
	lua_pushnumber(L, (lua_Number)fValue);
	lua_settable(L, -3);
	lua_pop(L, 1);
}

void CLuaFile2::CNode::VSetValueInteger(unsigned long iValue){QUICK_THROW("Invalid operation for LUA")}
void CLuaFile2::CNode::VSetValueBool(bool bValue)
{
	if(m_iType != LUA_TBOOLEAN) QUICK_THROW("Invalid type")
	_pushTable();
	_pushKey();
	lua_pushboolean(L, bValue ? 1 : 0);
	lua_settable(L, -3);
	lua_pop(L, 1);
}
void CLuaFile2::CNode::VSetValueString(const char* sValue)
{
	if(m_iType != LUA_TSTRING) QUICK_THROW("Invalid type")
	_pushTable();
	_pushKey();
	lua_pushstring(L, sValue);
	lua_settable(L, -3);
	lua_pop(L, 1);
}
void CLuaFile2::CNode::VSetValueWString(const wchar_t* wsValue){QUICK_THROW("Invalid operation for LUA")}

CMemoryStore::COutStream* CLuaFile2::CNode::VGetNodeAsRainmanRgd(){QUICK_THROW("TODO")}
void CLuaFile2::CNode::SGetNodeFromRainmanRgd(IFileStore::IStream* pInput, bool bSetName){QUICK_THROW("TODO")}

void CLuaFile2::saveFile(IFileStore::IOutputStream *pStream, const char* sFileName)
{
	pStream->VWriteString("----------------------------------------\r\n");
	pStream->VWriteString("-- File: \'");
	pStream->VWriteString(sFileName);
	pStream->VWriteString("\'\r\n");
	pStream->VWriteString("-- Created by: Corsix\'s Mod Studio\r\n");
	pStream->VWriteString("-- Note: DO edit by hand!\r\n");
	pStream->VWriteString("-- (c) 2001 Relic Entertainment Inc.\r\n\r\n");

	lua_pushstring(L, "GameData");
	lua_gettable(L, LUA_GLOBALSINDEX);
	_saveTable("GameData", pStream, 0);
	lua_pop(L, 1);
	lua_pushstring(L, "MetaData");
	lua_gettable(L, LUA_GLOBALSINDEX);
	_saveTable("MetaData", pStream, 0);
	lua_pop(L, 1);
	pStream->VWriteString("\r\n");

	lua_pushstring(L, "GameData");
	lua_gettable(L, LUA_GLOBALSINDEX);
	_saveTable("GameData", pStream, 1);
	lua_pop(L, 1);
	pStream->VWriteString("\r\n\r\n");

	lua_pushstring(L, "MetaData");
	lua_gettable(L, LUA_GLOBALSINDEX);
	_saveTable("MetaData", pStream, 3);
	lua_pop(L, 1);
}

void CLuaFile2::_appendEscapedString(char* sDest, const char* sSrc)
{
	sDest += strlen(sDest);

	while(*sSrc)
	{
		if(*sSrc == '\a' || *sSrc == '\b' || *sSrc == '\f' || *sSrc == '\n' || *sSrc == '\r' ||
			*sSrc == '\t' || *sSrc == '\v' || *sSrc == '\\' || *sSrc == '\"' || *sSrc == '\'')
		{
			*sDest = '\\';
			++sDest;
			switch(*sSrc)
			{
				case '\a': *sDest = 'a'; break;
				case '\b': *sDest = 'b'; break;
				case '\f': *sDest = 'f'; break;
				case '\n': *sDest = 'n'; break;
				case '\r': *sDest = 'r'; break;
				case '\t': *sDest = 't'; break;
				case '\v': *sDest = 'v'; break;
				default: *sDest = *sSrc;
			};
		}
		else
		{
			*sDest = *sSrc;
		}

		++sDest;
		++sSrc;
	}

	*sDest = 0;
}


void CLuaFile2::_saveTable(const char* sPrefix, IFileStore::IOutputStream* pStream, int iMode)
{
	/*
		Modes:
		0 = write inherit(meta) line only
		1 = write normally, but no inherit(meta)
		2 = write normally
		3 = write metadata table
		4 = write metadata inline table
	*/

	lua_checkstack(L, 5); // T
	if(iMode == 2 || iMode == 0)
	{
		lua_pushstring(L, "$FUNC"); // "$FUNC" T
		lua_rawget(L, -2); // $FUNC T
		if(lua_isstring(L, -1) && lua_strlen(L, -1))
		{
			pStream->VWriteString(sPrefix);
			pStream->VWriteString(" = ");
			pStream->VWriteString(lua_tostring(L, -1));
			pStream->VWriteString("([[");
			lua_pop(L, 1); // T
			lua_pushstring(L, "$REF"); // "$REF" T
			lua_rawget(L, -2); // $REF T
			pStream->VWriteString(lua_tostring(L, -1));
			pStream->VWriteString("]])\r\n");
		}
		lua_pop(L, 1); // T
	}

	if(iMode == 0) return;
	if(iMode < 2) iMode = 2;

	if(iMode == 4)
	{
		pStream->VWriteString(sPrefix);
		pStream->VWriteString(" = {");
	}

	std::vector<_SaveKey*> vKeys;
	lua_pushnil(L); // nil T
	while(lua_next(L, -2) != 0) // V K T
	{
		lua_pop(L, 1); // K T
		if(lua_isstring(L, -1))
		{
			const char* sTmp = lua_tostring(L, -1);
			if(sTmp[0] == '$')
			{
				if( (strcmp(sTmp, "$FUNC") == 0) || (strcmp(sTmp, "$PARENT") == 0) || (strcmp(sTmp, "$REF") == 0) )
				{
					continue;
				}
			}
		}
		_SaveKey* pKey = new _SaveKey;
		if(lua_isstring(L, -1))
		{
			pKey->iWhat = 2;
			pKey->s = lua_tostring(L, -1);
		}
		else if(lua_isnumber(L, -2))
		{
			pKey->iWhat = 1;
			pKey->f = lua_tonumber(L, -1);
		}
		else if(lua_isboolean(L, -2))
		{
			pKey->iWhat = 3;
			pKey->b = lua_toboolean(L, -1);
		}
		else
		{
			QUICK_THROW("Table keys must be strings, numbers or booleans")
		}

		vKeys.push_back(pKey);
	} // T

	std::sort(vKeys.begin(), vKeys.end(), _SaveKey::_Sort);

	for(std::vector<_SaveKey*>::iterator itr = vKeys.begin(); itr != vKeys.end(); ++itr)
	{
		switch((**itr).iWhat)
		{
		case 1: lua_pushnumber(L, (**itr).f); break;
		case 2: lua_pushstring(L, (**itr).s); break;
		case 3: lua_pushboolean(L, (**itr).b); break;
		}
		lua_pushvalue(L, -1); // K K T
		lua_gettable(L, -3); // V K T

		char* sNewPrefix = 0;
		if(lua_isstring(L, -2))
		{
			sNewPrefix = new char[strlen(sPrefix) + (lua_strlen(L, -2) * 2) + 1];
			if(iMode == 4) sNewPrefix[0] = 0;
			else sprintf(sNewPrefix, "%s[\"", sPrefix);
			_appendEscapedString(sNewPrefix, lua_tostring(L, -2));
			if(iMode != 4)strcat(sNewPrefix, "\"]");
		}
		else if(lua_isnumber(L, -2))
		{
			sNewPrefix = new char[strlen(sPrefix) + 24];
			if(iMode == 4) sprintf(sNewPrefix,"%.5f", (double)lua_tonumber(L, -2));
			else sprintf(sNewPrefix,"%s[%.5f]", sPrefix, (double)lua_tonumber(L, -2));
		}
		else if(lua_isboolean(L, -2))
		{
			sNewPrefix = new char[strlen(sPrefix) + 8];
			if(iMode == 4) sprintf(sNewPrefix, "%s", (lua_toboolean(L, -2) == 1) ? "true" : "false");
			else sprintf(sNewPrefix,"%s[%s]", sPrefix, (lua_toboolean(L, -2) == 1) ? "true" : "false");
		}
		else
		{
			QUICK_THROW("Table keys must be strings, numbers or booleans")
		}

		char sSprintSpace[24];
		switch(lua_type(L, -1))
		{
		case LUA_TNUMBER:
			sprintf(sSprintSpace, (iMode == 4) ? "%.0f" : "%.5f", (double)lua_tonumber(L, -1));
			pStream->VWriteString(sNewPrefix);
			pStream->VWriteString(" = ");
			pStream->VWriteString(sSprintSpace);
			if(iMode != 4) pStream->VWriteString("\r\n");
			else pStream->VWriteString(", ");
			break;

		case LUA_TBOOLEAN:
			pStream->VWriteString(sNewPrefix);
			pStream->VWriteString(" = ");
			pStream->VWriteString((lua_toboolean(L, -1) == 1) ? "true" : "false");
			if(iMode != 4) pStream->VWriteString("\r\n");
			else pStream->VWriteString(", ");
			break;

		case LUA_TSTRING: {
			if(iMode == 4)
			{
				pStream->VWriteString(sNewPrefix);
				pStream->VWriteString(" = [[");
				pStream->VWriteString(lua_tostring(L, -1));
				pStream->VWriteString("]], ");
			}
			else
			{
				char* sEscaped = new char[(lua_strlen(L, -1) * 2) + 1];
				sEscaped[0] = 0;
				_appendEscapedString(sEscaped, lua_tostring(L, -1));
				pStream->VWriteString(sNewPrefix);
				pStream->VWriteString(" = \"");
				pStream->VWriteString(sEscaped);
				pStream->VWriteString("\"\r\n");
				delete[] sEscaped;
			}
			break; }

		case LUA_TTABLE:
			_saveTable(sNewPrefix, pStream, (iMode == 3 ? 4 : iMode) );
			if(iMode >= 3) pStream->VWriteString("}");
			if(iMode == 3) pStream->VWriteString("\r\n");
			break;

		default: {
			if(iMode != 4)
			{
				const char* sTypeNames[] = {"LUA_TNIL" , 0 , "LUA_TLIGHTUSERDATA", 0, 0, 0, "LUA_TFUNCTION" , "LUA_TUSERDATA" , "LUA_TTHREAD"};
				pStream->VWriteString("-- ");
				pStream->VWriteString(sNewPrefix);
				pStream->VWriteString(" = (");
				pStream->VWriteString(sTypeNames[lua_type(L, -1)]);
				pStream->VWriteString(")\r\n");
			}
			break; }
		};
		delete[] sNewPrefix;

		lua_pop(L, 2); // T
	}
}