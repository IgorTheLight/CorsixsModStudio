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

#include "CRgdFile.h"
#include "CMemoryStore.h"
#include "..\zLib/zlib.h"
#include <algorithm>
#include "memdebug.h"
extern "C" {
#include <lauxlib.h>
#include <lualib.h>
};
#include "luax.h"
#include "Exception.h"

struct tLuaTableProtector
{
	void Init(lua_State* L, int iT, bool bOwn, tLuaTableProtector* parent = 0);

	// o[k]
	static int OnIndex(lua_State* L);

	// t
	static void SimpleTableDuplicate(lua_State* L);

	// o[k] = v
	static int OnNewIndex(lua_State* L);

	unsigned long iMagic;
	tLuaTableProtector* pParent;
	unsigned char iOwnIt;
};

static wchar_t* AsciiToUnicode(const char* sAscii)
{
	size_t iLen = strlen(sAscii) + 1;
	wchar_t *pUnicode = CHECK_MEM(new wchar_t[iLen]);
	for(size_t i = 0; i < iLen; ++i)
	{
		pUnicode[i] = (wchar_t)sAscii[i];
	}
	return pUnicode;
}

static char* UnicodeToAscii(const wchar_t* pUnicode)
{
	size_t iLen = wcslen(pUnicode) + 1;
	char *sAscii = CHECK_MEM(new char[iLen]);
	for(size_t i = 0; i < iLen; ++i)
	{
		sAscii[i] = (char)pUnicode[i];
	}
	return sAscii;
}

static char* mystrdup(const char* sStr)
{
	char* s = CHECK_MEM(new char[strlen(sStr) + 1]);
	strcpy(s, sStr);
	return s;
}

static wchar_t* mywcsdup(const wchar_t* sStr)
{
	wchar_t* s = CHECK_MEM(new wchar_t[wcslen(sStr) + 1]);
	wcscpy(s, sStr);
	return s;
}

CRgdFile::CRgdFile(void)
{
	m_RgdHeader.sHeader = 0;
	m_RgdHeader.iVersion = 0;
	m_RgdHeader.iUnknown3 = 0;
	m_RgdHeader.iUnknown4 = 0;
	m_RgdHeader.iUnknown5 = 0;
	m_RgdHeader.iUnknown6 = 0;
	m_pDataChunk = 0;
	m_pHashTable = 0;
	m_bConvertTableIntToTable = true;
}

CRgdFile::~CRgdFile(void)
{
	_Clean();
}

CRgdHashTable* CRgdFile::GetHashTable()
{
	return m_pHashTable;
}

void CRgdFile::SetHashTable(CRgdHashTable *pTable)
{
	m_pHashTable = pTable;
};

void CRgdFile::New(long iVersion)
{
	_Clean();
	m_RgdHeader.sHeader = CHECK_MEM(new char[16]);
	strcpy(m_RgdHeader.sHeader, "Relic Chunky\x0D\x0A\x1A");
	m_RgdHeader.iVersion = iVersion;
	m_RgdHeader.iUnknown3 = 1;
	m_RgdHeader.iUnknown4 = 0x24;
	m_RgdHeader.iUnknown5 = 0x1C;
	m_RgdHeader.iUnknown6 = 0x01;

	m_pDataChunk = new _RgdChunk;
	if(m_pDataChunk == 0)
	{
		_Clean();
		throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
	}
	m_vRgdChunks.push_back(m_pDataChunk);
	m_pDataChunk->RootEntry.Type = DT_Bool;
	m_pDataChunk->RootEntry.pParentFile = (CRgdFile*)this;
	m_pDataChunk->sChunkyType = 0;
	m_pDataChunk->iVersion = 1; 
	m_pDataChunk->iChunkLength = sizeof(unsigned long) + sizeof(long);
	m_pDataChunk->iStringLength = 0;
	m_pDataChunk->sString = 0;
	m_pDataChunk->iUnknown1 = 0xffffffff;
	m_pDataChunk->iUnknown2 = 0;
	m_pDataChunk->iCRC = crc32(0L, Z_NULL, 0);
	m_pDataChunk->iDataLength = 0;
	m_pDataChunk->pData = 0;

	m_pDataChunk->sChunkyType = new char[9];
	if(m_pDataChunk->sChunkyType == 0)
	{
		_Clean();
		throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
	}
	strcpy(m_pDataChunk->sChunkyType, "DATAAEGD");

	m_pDataChunk->sString = new char[1];
	if(m_pDataChunk->sString == 0)
	{
		_Clean();
		throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
	}
	strcpy(m_pDataChunk->sString, "");

	m_pDataChunk->RootEntry.iHash = 0;
	m_pDataChunk->RootEntry.sName = "";
	m_pDataChunk->RootEntry.Type = DT_Table;
	m_pDataChunk->RootEntry.pExt = 0;
	m_pDataChunk->RootEntry.Data.t = new std::vector<_RgdEntry*>;
	if(m_pDataChunk->RootEntry.Data.t == 0)
	{
		_Clean();
		throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
	}
}

bool CRgdFile::_SortOutEntries(CRgdFile::_RgdEntry* a, CRgdFile::_RgdEntry* b)
{
	return a->iHash < b->iHash;
}

void CRgdFile::Save(IFileStore::IOutputStream *pStream)
{
	// Write RGD Header
	if(pStream == 0) throw new CRainmanException(__FILE__, __LINE__, "No stream present");

	try
	{
		pStream->VWrite(16, 1, (void*)m_RgdHeader.sHeader);
		pStream->VWrite(1, 4, (void*)&m_RgdHeader.iVersion);
		pStream->VWrite(1, 4, (void*)&m_RgdHeader.iUnknown3);
		if(m_RgdHeader.iVersion == 3)
		{
			pStream->VWrite(1, 4, (void*)&m_RgdHeader.iUnknown4);
			pStream->VWrite(1, 4, (void*)&m_RgdHeader.iUnknown5);
			pStream->VWrite(1, 4, (void*)&m_RgdHeader.iUnknown6);
		}
	}
	catch(CRainmanException* pE)
	{
		throw new CRainmanException(__FILE__, __LINE__, "Write error", pE);
	}

	// Write chunks
	for(std::vector<_RgdChunk*>::iterator itr = m_vRgdChunks.begin(); itr != m_vRgdChunks.end(); ++itr)
	{
		if(*itr == m_pDataChunk)
		{
			// We need to update the data chunk
			CMemoryStore CMem;
			CMemoryStore::COutStream* pDataStr;
			try
			{
				pDataStr = (CMemoryStore::COutStream*)CMem.VOpenOutputStream(0, false); // I _know_ that it WILL be a CMemoryStore::COutStream
			}
			catch(CRainmanException* pE)
			{
				throw new CRainmanException(__FILE__, __LINE__, "Cannot open memory stream", pE);
			}
			try
			{
				_WriteRawRgdData(pDataStr, &(**itr).RootEntry);
			}
			catch(CRainmanException* pE)
			{
				delete pDataStr;
				throw new CRainmanException(__FILE__, __LINE__, "Cannot write raw data", pE);
			}
			try
			{
				(**itr).iDataLength	= (long)pDataStr->GetDataLength();
				delete[] (**itr).pData;
				(**itr).pData = new char[pDataStr->GetDataLength()];
			}
			catch(CRainmanException* pE)
			{
				delete pDataStr;
				throw new CRainmanException(__FILE__, __LINE__, "Cannot get data length", pE);
			}
			if((**itr).pData == 0)
			{
				delete pDataStr;
				throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
			}
			try
			{
				memcpy((**itr).pData, pDataStr->GetData(), pDataStr->GetDataLength());
			}
			catch(CRainmanException* pE)
			{
				delete pDataStr;
				throw new CRainmanException(__FILE__, __LINE__, "Cannot copy data", pE);
			}
			delete pDataStr;
			(**itr).iChunkLength = (**itr).iStringLength  + sizeof(unsigned long) + sizeof(long) + (**itr).iDataLength;
			(**itr).iCRC = crc32(crc32(0L, Z_NULL, 0), (const Bytef*)(**itr).pData, (**itr).iDataLength);
		}
		try
		{
			pStream->VWrite(1, 8, (void*)(**itr).sChunkyType);
			pStream->VWrite(1, 4, (void*)&(**itr).iVersion);
			pStream->VWrite(1, 4, (void*)&(**itr).iChunkLength);
			pStream->VWrite(1, 4, (void*)&(**itr).iStringLength);
			pStream->VWrite((**itr).iStringLength, 1, (void*)(**itr).sString);
			if(m_RgdHeader.iVersion == 3)
			{
				pStream->VWrite(1, 4, (void*)&(**itr).iUnknown1);
				pStream->VWrite(1, 4, (void*)&(**itr).iUnknown2);
			}
			pStream->VWrite(1, 4, (void*)&(**itr).iCRC);
			pStream->VWrite(1, 4, (void*)&(**itr).iDataLength);
			pStream->VWrite((**itr).iDataLength, 1, (void*)(**itr).pData);
		}
		catch(CRainmanException* pE)
		{
			throw new CRainmanException(__FILE__, __LINE__, "Cannot write values", pE);
		}

	}
}

void CRgdFile::Load(CLuaFile *pLuaFile, long iVersion)
{
	try
	{
		New(iVersion);
	}
	catch(CRainmanException *pE)
	{
		throw new CRainmanException(__FILE__, __LINE__, "Cannot create new", pE);
	}

	lua_State* L = pLuaFile->m_pLua;
	lua_pushstring(L, "GameData");
	lua_gettable(L, LUA_GLOBALSINDEX);
	if(lua_type(L, -1) == LUA_TNIL)
	{
		// FX files have fxtypes as a root
		lua_pop(L, 1);
		lua_pushstring(L, "fxtypes");
		lua_gettable(L, LUA_GLOBALSINDEX);
		if(lua_type(L, -1) == LUA_TNIL)
		{
			_Clean();
			lua_pop(L, 1);
			throw new CRainmanException(__FILE__, __LINE__, "No GameData in lua file");
		}
	}
	delete m_pDataChunk->RootEntry.Data.t;
	m_pDataChunk->RootEntry.Data.t = 0;

	try
	{
		_LoadLua(L, &m_pDataChunk->RootEntry, true);
	}
	catch(CRainmanException *pE)
	{
		_Clean();
		lua_pop(L, 1);
		throw new CRainmanException(__FILE__, __LINE__, "Cannot create load raw lua", pE);
	}
	lua_pop(L, 1);
}

lua_State* CRgdFile::CreateLuaState()
{
	lua_State *L = lua_open();

	luaopen_base(L);
    luaopen_string(L);
    luaopen_math(L);
	luaopen_table(L);

	luax_DeleteGlobal(L, "coroutine");
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

	lua_pushstring(L, "GameData");
	lua_newtable(L);

	_RgdTableToLuaState(L, &m_pDataChunk->RootEntry);

	lua_settable(L, LUA_GLOBALSINDEX);

	return L;
}

void CRgdFile::_RgdTableToLuaState(lua_State* L, CRgdFile::_RgdEntry *pTable)
{
	// Reference

	// Children
	std::vector<_RgdEntry*>* pChildren = pTable->Data.t;
	for(std::vector<_RgdEntry*>::iterator itr = pChildren->begin(); itr != pChildren->end(); ++itr)
	{
		// Add name to lua stack
		if( (**itr).sName )
		{
			lua_pushstring(L, (**itr).sName);
		}
		else
		{
			char sName[11];
			sName[0] = '0';
			sName[1] = 'x';
			unsigned long iHash = (**itr).iHash;
			int i = 1;
			for(int iNibble = 7; iNibble >= 0; --iNibble)
			{
				sName[++i] = "0123456789ABCDEF"[(iHash >> (iNibble << 2)) & 15];
			}
			sName[10] = 0;

			lua_pushstring(L, sName);
		}

		// Add value to lua stack
		switch( (**itr).Type )
		{
		case DT_Float:
			lua_pushnumber(L, (**itr).Data.f);
			break;
		case DT_Integer:
			break;
		case DT_Bool:
			lua_pushboolean(L, (**itr).Data.b ? 1 : 0);
			break;
		case DT_String:
			lua_pushstring(L, (**itr).Data.s);
			break;
		case DT_WString:
			break;
		case DT_Table:
		case sk_TableInt:
			lua_newtable(L);
			_RgdTableToLuaState(L, *itr);
			break;
		default:
			lua_pushnil(L);
			break;
		};

		// Push name & value from the stack and into the table
		lua_settable(L, -3);
	}
}

void CRgdFile::_LoadLua(lua_State* L, _RgdEntry* pDest, bool bSkipThisLevelRef)
{
	switch(lua_type(L, -1))
	{
	case LUA_TNONE:
	case LUA_TNIL:
		throw new CRainmanException(0, __FILE__, __LINE__, "Data type %i not supported", lua_type(L, -1));

	case LUA_TBOOLEAN:
		pDest->Type = IMetaNode::DT_Bool;
		pDest->Data.b = lua_toboolean(L, -1) ? true : false;
		break;

	case LUA_TLIGHTUSERDATA:
		throw new CRainmanException(0, __FILE__, __LINE__, "Data type %i not supported", lua_type(L, -1));

	case LUA_TNUMBER:
		pDest->Type = IMetaNode::DT_Float;
		pDest->Data.f = lua_tonumber(L, -1);
		break;

	case LUA_TSTRING:
		{
		size_t iL = lua_strlen(L, -1);
		const char *sTmp = lua_tostring(L, -1);
		bool bIsUcsRef = false;
		if(*sTmp == '$') // LUA files will have UCS references as strings, not WStrings
		{
			++sTmp;
			bIsUcsRef = true;
			while(*sTmp && bIsUcsRef)
			{
				if(*sTmp < '0' || *sTmp > '9') bIsUcsRef = false;
				++sTmp;
			}
		}
		if(bIsUcsRef)
		{
			if(m_RgdHeader.iVersion == 1)
			{
				pDest->Type = IMetaNode::DT_WString;
				pDest->Data.ws = new wchar_t[iL + 1];
				sTmp = lua_tostring(L, -1);
				for(size_t i = 0; i <= iL; ++i) pDest->Data.ws[i] = sTmp[i];
			}
			else
			{
				pDest->Type = IMetaNode::DT_Integer;
				pDest->Data.i = strtoul(lua_tostring(L, -1) + 1, 0, 10);
			}
		}
		else
		{
			pDest->Type = IMetaNode::DT_String;
			pDest->Data.s = new char[iL + 1];
			strcpy(pDest->Data.s, lua_tostring(L, -1));
		}
		break;
		}

	case LUA_TUSERDATA:
		{
			tLuaTableProtector *pTmp = (tLuaTableProtector*)lua_touserdata(L, -1);
			if(pTmp->iMagic != 0x7291BEEF) throw new CRainmanException(__FILE__, __LINE__, "Data type not supported");
			lua_pushlightuserdata(L, (void*)pTmp);
			lua_remove(L, -2);
			lua_gettable(L, LUA_REGISTRYINDEX);
			if(lua_type(L, -1) != LUA_TTABLE) throw new CRainmanException(__FILE__, __LINE__, "Data type not supported");
			// flow onward / no break
		}
	case LUA_TTABLE:
		{
			pDest->Type = IMetaNode::DT_Table;
			std::vector<_RgdEntry*>* pTab = pDest->Data.t = new std::vector<_RgdEntry*>;
			_RgdEntry* pNew;
			lua_checkstack(L, 2);
			lua_pushnil(L);
			while(lua_next(L, -2) != 0)
			{
				pNew = new _RgdEntry;
				pNew->pParentFile = (CRgdFile*)this;
				pNew->pExt = 0;
				if(lua_type(L, -2) == LUA_TSTRING && strcmp(lua_tostring(L, -2), "$dow_mod_studio") == 0)
				{
					if(bSkipThisLevelRef)
					{
						delete pNew;
						lua_pop(L, 1);
						continue;
					}
					lua_checkstack(L, 1);
					lua_pushstring(L, "ref_name");
					lua_gettable(L, -2);
					pNew->sName = 0;
					pNew->iHash = 0x49D60FAE;
				}
				else
				{
					const char *sNameTmp;
					char sFlBuf[24];
					switch(lua_type(L, -2))
					{
					case LUA_TBOOLEAN:
						sNameTmp = lua_toboolean(L, -2) ? "true" : "false";
						break;

					case LUA_TNUMBER:
						sprintf(sFlBuf, "%.16g", lua_tonumber(L, -2));
						sNameTmp = &sFlBuf[0];
						break;

					case LUA_TSTRING:
						sNameTmp = lua_tostring(L, -2);
						break;

					default:
						delete pNew;
						lua_pop(L, 2);
						throw new CRainmanException(__FILE__, __LINE__, "Data type not supported");
					};
					try
					{
						pNew->sName = m_pHashTable->HashToValue(pNew->iHash = m_pHashTable->ValueToHash(sNameTmp));
					}
					catch(CRainmanException* pE)
					{
						delete pNew;
						lua_pop(L, 2);
						throw new CRainmanException(__FILE__, __LINE__, "Hash table problem", pE);
					}
				}
				try
				{
					_LoadLua(L, pNew);
				}
				catch(CRainmanException* pE)
				{
					delete pNew;
					lua_pop(L, 2);
					throw new CRainmanException(__FILE__, __LINE__, "Cannot load child", pE);
				}
				pTab->push_back(pNew);
				if(pNew->iHash == 0x49D60FAE)
				{
					pDest->pExt = pNew;
					lua_pop(L, 2);
				}
				else
				{
					lua_pop(L, 1);
				}
			}
			break;
		}

	case LUA_TFUNCTION:
	case LUA_TTHREAD:
		throw new CRainmanException(0, __FILE__, __LINE__, "Data type %i not supported", lua_type(L, -1));
	};
}

void CRgdFile::Load(IFileStore::IStream *pStream)
{
	// Load RGD Header
	_Clean();
	if(pStream == 0) throw new CRainmanException(__FILE__, __LINE__, "No stream present");

	m_RgdHeader.sHeader = new char[16];
	if(m_RgdHeader.sHeader == 0) throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
	memset((void*)m_RgdHeader.sHeader, 0, 16);

	try
	{
		pStream->VRead(16, 1, (void*)m_RgdHeader.sHeader);
		pStream->VRead(1, 4, (void*)&m_RgdHeader.iVersion);
		pStream->VRead(1, 4, (void*)&m_RgdHeader.iUnknown3);
		if(m_RgdHeader.iVersion == 3)
		{
			pStream->VRead(1, 4, (void*)&m_RgdHeader.iUnknown4);
			pStream->VRead(1, 4, (void*)&m_RgdHeader.iUnknown5);
			pStream->VRead(1, 4, (void*)&m_RgdHeader.iUnknown6);
		}
	}
	catch(CRainmanException *pE)
	{
		_Clean();
		throw new CRainmanException(__FILE__, __LINE__, "Input error", pE);
	}

	if((strcmp(m_RgdHeader.sHeader, "Relic Chunky\x0D\x0A\x1A") != 0) || !(m_RgdHeader.iVersion == 1 || m_RgdHeader.iVersion == 3) || m_RgdHeader.iUnknown3 != 1)
	{
		_Clean();
		throw new CRainmanException(0, __FILE__, __LINE__, "Unrecognised header (%s,%li,%li)", m_RgdHeader.sHeader, m_RgdHeader.iVersion, m_RgdHeader.iUnknown3);
	}

	// Attempt to read chunks
	char *sChunkHeadTempBuffer = new char[9];
	if(sChunkHeadTempBuffer == 0)
	{
		_Clean();
		throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
	}
	memset(sChunkHeadTempBuffer, 0, 9);

	while(1)
	{
		try
		{
			pStream->VRead(8, 1, sChunkHeadTempBuffer);
		}
		catch(CRainmanException* pE)
		{
			pE->destroy();
			break;
		}
		_RgdChunk *pChunk = new _RgdChunk;
		if(pChunk == 0)
		{
			delete[] sChunkHeadTempBuffer;
			_Clean();
			throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
		}
		pChunk->sChunkyType = 0;
		pChunk->sString = 0;
		pChunk->pData = 0;
		pChunk->RootEntry.Type = DT_Bool;
		pChunk->RootEntry.pParentFile = (CRgdFile*)this;
		m_vRgdChunks.push_back(pChunk);
		if(m_pDataChunk == 0 && (strcmp(sChunkHeadTempBuffer, "DATAAEGD") == 0))
		{
			m_pDataChunk = pChunk;
		}

		pChunk->sChunkyType = new char[9]; // 8 bytes + 1
		if(pChunk->sChunkyType == 0)
		{
			delete[] sChunkHeadTempBuffer;
			_Clean();
			throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
		}
		strcpy(pChunk->sChunkyType, sChunkHeadTempBuffer);

		try
		{
			pStream->VRead(1, 4, (void*)&pChunk->iVersion);
			pStream->VRead(1, 4, (void*)&pChunk->iChunkLength);
			pStream->VRead(1, 4, (void*)&pChunk->iStringLength);
		}
		catch(CRainmanException* pE)
		{
			delete[] sChunkHeadTempBuffer;
			_Clean();
			throw new CRainmanException(__FILE__, __LINE__, "Input error", pE);
		}

		pChunk->sString = new char[pChunk->iStringLength + 1]; // n bytes + 1
		if(pChunk->sString == 0)
		{
			delete[] sChunkHeadTempBuffer;
			_Clean();
			throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
		}
		memset((void*)pChunk->sString, 0, pChunk->iStringLength + 1);

		try
		{
			pStream->VRead(pChunk->iStringLength, 1, (void*)pChunk->sString);
			if(m_RgdHeader.iVersion == 3)
			{
				pStream->VRead(1, 4, (void*)&pChunk->iUnknown1);
				pStream->VRead(1, 4, (void*)&pChunk->iUnknown2);
			}
			pStream->VRead(1, 4, (void*)&pChunk->iCRC);
			pStream->VRead(1, 4, (void*)&pChunk->iDataLength);
		}
		catch(CRainmanException* pE)
		{
			delete[] sChunkHeadTempBuffer;
			_Clean();
			throw new CRainmanException(__FILE__, __LINE__, "Input error", pE);
		}

		pChunk->pData = new char[pChunk->iDataLength]; // n bytes
		if(pChunk->pData == 0)
		{
			delete[] sChunkHeadTempBuffer;
			_Clean();
			throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
		}

		try
		{
			long iPos = pStream->VTell();
			pStream->VRead(pChunk->iDataLength, 1, (void*)pChunk->pData);
		}
		catch(CRainmanException* pE)
		{
			delete[] sChunkHeadTempBuffer;
			_Clean();
			throw new CRainmanException(__FILE__, __LINE__, "Input error", pE);
		}
	}

	delete[] sChunkHeadTempBuffer;

	if(m_pDataChunk == 0)
	{
		_Clean();
		throw new CRainmanException(__FILE__, __LINE__, "No DATAAEGD found");
	}

	// Process binary data into table
	unsigned long iCRC = crc32(0L, Z_NULL, 0);
	iCRC = crc32(iCRC, (const Bytef*)m_pDataChunk->pData, m_pDataChunk->iDataLength);

	if(iCRC != m_pDataChunk->iCRC)
	{
		// Some really retarded tools don't put in valid CRCs. As much as I would like to throw an error here,
		// loading these RGDs is something we want, otherwise fixing the CRC is a pain in the ass to do, and
		// we want to be user friendly. :/

		/*
		PAUSE_THROW(0, __FILE__, __LINE__, "CRCs do not match (%lu, %lu)", iCRC, m_pDataChunk->iCRC);
		_Clean();
		UNPAUSE_THROW;
		*/
	}

	m_pDataChunk->RootEntry.iHash = 0;
	m_pDataChunk->RootEntry.sName = "";
	m_pDataChunk->RootEntry.Type = DT_Table;
	m_pDataChunk->RootEntry.pExt = 0;
	m_pDataChunk->RootEntry.Data.t = new std::vector<_RgdEntry*>;
	if(m_pDataChunk->RootEntry.Data.t == 0)
	{
		_Clean();
		throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
	}

	CMemoryStore MemStore;
	IFileStore::IStream *pMemStream;
	try
	{
		MemStore.VInit();
		pMemStream = MemStore.VOpenStream(MemStore.MemoryRange(m_pDataChunk->pData, m_pDataChunk->iDataLength));
	}
	catch(CRainmanException* pE)
	{
		_Clean();
		throw new CRainmanException(__FILE__, __LINE__, "Memory buffer error", pE);
	}

	try
	{
		_ProcessRawRgdData(pMemStream, &m_pDataChunk->RootEntry);
	}
	catch(CRainmanException* pE)
	{
		delete pMemStream;
		_Clean();
		throw new CRainmanException(__FILE__, __LINE__, "Could not process DATAAEGD data", pE);
	}
	delete pMemStream;
}

const char* CRgdFile::GetDescriptorString()
{
	if(m_pDataChunk == 0) throw new CRainmanException(__FILE__, __LINE__, "No main chunk present for string");
	return m_pDataChunk->sString;
}

void CRgdFile::SetDescriptorString(const char* sString)
{
	if(m_pDataChunk == 0) throw new CRainmanException(__FILE__, __LINE__, "No main chunk present for string");
	if(sString == 0) sString = ""; // The blank string is a const char* to the res section and thus can be used

	if(((long)strlen(sString)) > m_pDataChunk->iStringLength) // Only allocate / deallocate if needed
	{
		char* sTmp = CHECK_MEM(new char[strlen(sString) + 1]);
		delete[] m_pDataChunk->sString;
		m_pDataChunk->sString = sTmp;
	}

	strcpy(m_pDataChunk->sString, sString);
	m_pDataChunk->iStringLength = (long)strlen(sString);
	m_pDataChunk->iChunkLength = m_pDataChunk->iStringLength + sizeof(unsigned long) + sizeof(long) + m_pDataChunk->iDataLength;
}

long CRgdFile::GetChunkVersion()
{
	if(m_pDataChunk == 0) throw new CRainmanException(__FILE__, __LINE__, "No main chunk present for version");
	return m_pDataChunk->iVersion;
}

void CRgdFile::SetChunkVersion(long iVersion)
{
	if(m_pDataChunk == 0) throw new CRainmanException(__FILE__, __LINE__, "No main chunk present for version");
	m_pDataChunk->iVersion = iVersion;
	return;
}

void CRgdFile::_Clean()
{
	if(m_RgdHeader.sHeader != 0)
	{
		delete[] m_RgdHeader.sHeader;
		m_RgdHeader.sHeader = 0;
	}
	m_RgdHeader.iVersion = 0;
	m_RgdHeader.iUnknown3 = 0;
	m_RgdHeader.iUnknown4 = 0;
	m_RgdHeader.iUnknown5 = 0;
	m_RgdHeader.iUnknown6 = 0;
	m_pDataChunk = 0;

	for(std::vector<_RgdChunk*>::iterator itr = m_vRgdChunks.begin(); itr != m_vRgdChunks.end(); ++itr)
	{
		if((**itr).sString != 0)
		{
			delete[] (**itr).sString;
			(**itr).sString = 0;
		}

		if((**itr).sChunkyType != 0)
		{
			delete[] (**itr).sChunkyType;
			(**itr).sChunkyType = 0;
		}

		if((**itr).pData != 0)
		{
			delete[] (**itr).pData;
			(**itr).pData = 0;
		}

		(**itr).iVersion = 0; 
		(**itr).iChunkLength = sizeof(unsigned long) + sizeof(long);
		(**itr).iStringLength = 0;
		(**itr).iCRC = crc32(0L, Z_NULL, 0);
		(**itr).iDataLength = 0;

		if((**itr).RootEntry.Type == DT_Table || (**itr).RootEntry.Type == sk_TableInt)
		{
			_CleanRgdTable(&(**itr).RootEntry);
		}

		delete (*itr);
	}
}

void CRgdFile::_CleanRgdTable(CRgdFile::_RgdEntry *pTable)
{
	if(pTable->Type == DT_Table || pTable->Type == sk_TableInt)
	{
		if(pTable->Data.t != 0)
		{
			for(std::vector<_RgdEntry*>::iterator itr = pTable->Data.t->begin(); itr != pTable->Data.t->end(); ++itr)
			{
				switch((**itr).Type)
				{
				case DT_Table:
				case sk_TableInt:
					_CleanRgdTable(*itr);
					break;
				case DT_String:
					if((**itr).Data.s) delete[] (**itr).Data.s;
					break;
				case DT_WString:
					if((**itr).Data.ws) delete[] (**itr).Data.ws;
					break;
				};
				delete *itr;
			}

			delete pTable->Data.t;
		}
	}
}

bool CRgdFile::_SortOutEntriesNum(_RgdEntry* a, _RgdEntry* b)
{
	return (strtoul(a->sName, 0, 10) < strtoul(b->sName, 0, 10));
}

void CRgdFile::_WriteRawRgdData(IFileStore::IOutputStream *pStream, _RgdEntry *pSource, bool bTable101)
{
	if(pSource->Type != DT_Table && pSource->Type != sk_TableInt) throw new CRainmanException(0, __FILE__, __LINE__, "Source is not a table (%i)", (int)pSource->Type);
	std::vector<_RgdEntry*> vEntries;
	for(std::vector<_RgdEntry*>::iterator itr = pSource->Data.t->begin(); itr != pSource->Data.t->end(); ++itr)
	{
		if((**itr).Type != DT_NoData) vEntries.push_back(*itr);
	}
	sort(vEntries.begin(), vEntries.end(), bTable101 ? _SortOutEntriesNum : _SortOutEntries);
	size_t iKeyCount = vEntries.size();
	try
	{
		pStream->VWrite(1, sizeof(long), &iKeyCount);
	}
	CATCH_THROW("Output error")

	// Write 'dummy' data for keys (so that key data can be appended)
	long iOurBegin;
	try
	{
		iOurBegin = pStream->VTell();
	}
	CATCH_THROW("VTell error")

	long iDataLen = 0;
	long iDataOffset = (iKeyCount * (3 * sizeof(long))) + iOurBegin;
	for(long i = 0; i < iKeyCount; ++i)
	{
		long iTmp = 0;
		try
		{
			pStream->VWrite(1, sizeof(long), &iTmp);
			pStream->VWrite(1, sizeof(long), &iTmp);
			pStream->VWrite(1, sizeof(long), &iTmp);
		}
		CATCH_THROW("Output error")
	}
	try
	{
		pStream->VSeek(iOurBegin, IFileStore::IStream::SL_Root);
	}
	CATCH_THROW("Seek error")

	// Write actual keys
	for(std::vector<_RgdEntry*>::iterator itr = vEntries.begin(); itr != vEntries.end(); ++itr)
	{
		// We need to pad the data so that if it is N bytes big, it falls on an N byte boundry
		// Relic does this, and it's more efficient this way
		size_t iPad = 1;
		switch((**itr).Type)
		{
			case DT_Float:
				iPad = sizeof(float);
				break;
			case DT_Integer:
				iPad = sizeof(unsigned long);
				break;
			case DT_WString:
				iPad = sizeof(wchar_t);
				break;
			case DT_Table:
			case sk_TableInt:
				iPad = sizeof(unsigned long); // Tables are a collection of unsigned longs
				break;
		};
		if(iDataLen % iPad)
		{
			iPad = iPad - (iDataLen % iPad);
			iDataLen += iPad;
		}
		else
		{
			iPad = 0;
		}

		bool bNumericTable = false;
		try
		{
			unsigned long iType = (**itr).Type;
			if((**itr).Type == DT_Table || (**itr).Type == sk_TableInt)
			{
				for(std::vector<_RgdEntry*>::iterator itrT = (**itr).Data.t->begin(); itrT != (**itr).Data.t->end(); ++itrT)
				{
					if((**itrT).sName == 0) goto not_all_numeric;
					const char* s = (**itrT).sName;
					while(*s)
					{
						if(*s < '0' || *s > '9') goto not_all_numeric;
						++s;
					}
				}
				iType = sk_TableInt;
				bNumericTable = true;
not_all_numeric:;
			}
			pStream->VWrite(1, sizeof(long), &(**itr).iHash);
			pStream->VWrite(1, sizeof(long), &iType);
			pStream->VWrite(1, sizeof(long), &iDataLen);
		}
		CATCH_THROW("Output error")

		long iOldPos;
		try
		{
			iOldPos = pStream->VTell();
			pStream->VSeek(iDataOffset + iDataLen - iPad, IFileStore::IStream::SL_Root);
		}
		CATCH_THROW("Seek/Tell error")

		if(iPad)
		{
			unsigned char sTmp[7]; // iPad _shouldn't_ be bigger than seven
			sTmp[0] = 0;
			sTmp[1] = 0;
			sTmp[2] = 0;
			sTmp[3] = 0;
			sTmp[4] = 0;
			sTmp[5] = 0;
			sTmp[6] = 0;
			try
			{
				pStream->VWrite(iPad, 1, (void*)sTmp);
			}
			CATCH_THROW("Output error")
		}
		switch((**itr).Type)
		{
		case DT_Float:
			try
			{
				pStream->VWrite(1, sizeof(float), &(**itr).Data.f);
			}
			CATCH_THROW("Output error")
			iDataLen += sizeof(float);
			break;
		case DT_Integer:
			try
			{
				pStream->VWrite(1, sizeof(unsigned long), &(**itr).Data.i);
			}
			CATCH_THROW("Output error")
			iDataLen += sizeof(unsigned long);
			break;
		case DT_Bool:
			try
			{
				pStream->VWrite(1, 1, &(**itr).Data.b);
			}
			CATCH_THROW("Output error")
			iDataLen += 1;
			break;
		case DT_String:
			{
				size_t iL = strlen((**itr).Data.s) + 1;
				try
				{
					pStream->VWrite(iL, 1, (**itr).Data.s);
				}
				CATCH_THROW("Output error")
				iDataLen += iL;
				break;
			}
		case DT_WString:
			{
				size_t iL = wcslen((**itr).Data.ws) + 1;
				try
				{
					pStream->VWrite(iL, 2, (**itr).Data.ws);
				}
				CATCH_THROW("Output error")
				iDataLen += (iL  << 1);
				break;
			}
		case DT_Table:
		case sk_TableInt:
			try
			{
				_WriteRawRgdData(pStream, (*itr), bNumericTable);
			}
			CATCH_THROW("Could not write child")
			iDataLen += pStream->VTell() - (iDataOffset + iDataLen);
			break;
		};
		try
		{
			pStream->VSeek(iOldPos, IFileStore::IStream::SL_Root);
		}
		CATCH_THROW("Seek error")
	}
	// Seek to end of our output
	try
	{
		pStream->VSeek(iDataOffset + iDataLen, IFileStore::IStream::SL_Root);
	}
	CATCH_THROW("Seek error")
}

void CRgdFile::_ProcessRawRgdData(IFileStore::IStream *pStream, _RgdEntry *pDestination)
{
	long iKeyCount, iDataOffset;
	try
	{
		pStream->VRead(1, sizeof(long), &iKeyCount);
		iDataOffset = (iKeyCount * (3 * sizeof(long))) + pStream->VTell();
	}
	CATCH_THROW("Stream problem")

	while(iKeyCount)
	{
		_RgdEntry *pEntry = CHECK_MEM(new _RgdEntry);
		long iOffset;
		pDestination->Data.t->push_back(pEntry);
		
		pEntry->iHash = 0;
		pEntry->sName = 0;
		pEntry->Type = DT_Bool; // So that if we abort before Type is set, it gets cleaned up
		pEntry->pParentFile = (CRgdFile*)this;

		try
		{
			pStream->VRead(1, sizeof(long), &pEntry->iHash);
		}
		CATCH_THROW("Input error")

		try
		{
			pStream->VRead(1, sizeof(long), &pEntry->Type);
		}
		CATCH_THROW("Input error")

		if(m_pHashTable)
		{
			pEntry->sName = m_pHashTable->HashToValue(pEntry->iHash);
			if(pEntry->sName == 0 && pDestination->Type == sk_TableInt)
			{
				char sBuffer[20];
				unsigned long i = 0, h;
				while(i <= 10000)
				{
					_ultoa(i, sBuffer, 10);
					h = m_pHashTable->ValueToHashStatic(sBuffer);
					if(h == pEntry->iHash)
					{
						h = m_pHashTable->ValueToHash(sBuffer);
						pEntry->sName = m_pHashTable->HashToValue(h);
						break;
					}
					++i;
				}
			}
		}

		if(pEntry->Type == DT_Table ||pEntry->Type == sk_TableInt )
		{
			pEntry->pExt = 0;
			pEntry->Data.t = 0; // in case of abort
		}
		if(pEntry->Type == DT_String) pEntry->Data.s = 0; // in case of abort
		if(pEntry->Type == DT_WString) pEntry->Data.ws = 0; // in case of abort

		try
		{
			pStream->VRead(1, sizeof(long), &iOffset);
		}
		CATCH_THROW("Input error")

		long iOldPos;
		try
		{
			iOldPos = pStream->VTell();
			pStream->VSeek(iDataOffset + iOffset, IFileStore::IStream::SL_Root);
		}
		CATCH_THROW("Seek/Tell error")

		switch(pEntry->Type)
		{
		case DT_Float:
			try
			{
				pStream->VRead(1, sizeof(float), &pEntry->Data.f);
			}
			CATCH_THROW("Input error")
			break;
		case DT_Integer:
			try
			{
				pStream->VRead(1, sizeof(unsigned long), &pEntry->Data.i);
			}
			CATCH_THROW("Input error")
			break;
		case DT_Bool:
			try
			{
				pStream->VRead(1, 1, &pEntry->Data.b);
			}
			CATCH_THROW("Input error")
			break;
		case DT_String:
			{
				unsigned long iStrLen = 8, iLen = 0;
				char *sTmp = new char[iStrLen];
				if(sTmp == 0) QUICK_THROW("Memory error")
				do
				{
					try
					{
						pStream->VRead(1, 1, sTmp + iLen);
					}
					catch(CRainmanException *pE)
					{
						delete[] sTmp;
						throw new CRainmanException(__FILE__, __LINE__, "Input error", pE);
					}
					++iLen;
					if(iLen == iStrLen)
					{
						iStrLen <<= 1;
						char *sNew = new char[iStrLen];
						if(sNew == 0)
						{
							delete[] sTmp;
							QUICK_THROW("Memory error")
						}
						memcpy(sNew, sTmp, iLen);
						delete[] sTmp;
						sTmp = sNew;
					}
				}
				while(sTmp[iLen - 1]);

				pEntry->Data.s = sTmp;
				if(pEntry->iHash == 0x49D60FAE) pDestination->pExt = pEntry;
			}
			break;
		case DT_WString:
			{
				unsigned long iStrLen = 8, iLen = 0;
				wchar_t *sTmp = new wchar_t[iStrLen];
				if(sTmp == 0) QUICK_THROW("Memory error")
				do
				{
					try
					{
						pStream->VRead(1, sizeof(wchar_t), sTmp + iLen);
					}
					catch(CRainmanException *pE)
					{
						delete[] sTmp;
						throw new CRainmanException(__FILE__, __LINE__, "Input error", pE);
					}
					++iLen;
					if(iLen == iStrLen)
					{
						iStrLen <<= 1;
						wchar_t *sNew = new wchar_t[iStrLen];
						if(sNew == 0)
						{
							delete[] sTmp;
							QUICK_THROW("Memory error")
						}
						memcpy(sNew, sTmp, iLen * sizeof(wchar_t));
						delete[] sTmp;
						sTmp = sNew;
					}
				}
				while(sTmp[iLen - 1]);

				pEntry->Data.ws = sTmp;
				if(pEntry->iHash == 0x49D60FAE) pDestination->pExt = pEntry;
			}
			break;
		case DT_Table:
		case sk_TableInt:
			pEntry->Data.t = new std::vector<_RgdEntry*>;
			if(pEntry->Data.t == 0) QUICK_THROW("Memory error")
			try
			{
				_ProcessRawRgdData(pStream, pEntry);
			}
			catch(CRainmanException *pE)
			{
				throw new CRainmanException(__FILE__, __LINE__, "Processing of child failed", pE);
			}
			if(m_bConvertTableIntToTable) pEntry->Type = DT_Table;
			break;
		default:
			QUICK_THROW("Should never execute this :/ (Unknown RGD data type)")
			break;
		};
		try
		{
			pStream->VSeek(iOldPos, IFileStore::IStream::SL_Root);
		}
		CATCH_THROW("Seek error")

		--iKeyCount;
	}
}

IMetaNode::eDataTypes CRgdFile::VGetType()
{
	if(m_pDataChunk)
	{
		return DT_Table;
	}
	return DT_NoData;
}

const char* CRgdFile::VGetName()
{
	return "GameData";
}

float CRgdFile::VGetValueFloat()
{
	QUICK_THROW("Is not a float")
}

unsigned long CRgdFile::VGetValueInteger()
{
	QUICK_THROW("Is not an integer")
}

bool CRgdFile::VGetValueBool()
{
	QUICK_THROW("Is not a bool")
}

const char* CRgdFile::VGetValueString()
{
	QUICK_THROW("Is not a string")
}

const wchar_t* CRgdFile::VGetValueWString()
{
	QUICK_THROW("Is not a unicode string")
}

void CRgdFile::VSetType(IMetaNode::eDataTypes eType)
{
	if(eType != DT_Table) QUICK_THROW("Can only be table")
}

void CRgdFile::VSetName(const char* sName)
{
	if(strcmp(sName, "GameData") != 0) QUICK_THROW("Must be called GameData")
}

void CRgdFile::VSetNameHash(long iHash)
{
	QUICK_THROW("Must be a table")
}

void CRgdFile::VSetValueFloat(float fValue)
{
	QUICK_THROW("Must be a table")
}

void CRgdFile::VSetValueInteger(unsigned long iValue)
{
	QUICK_THROW("Must be a table")
}

void CRgdFile::VSetValueBool(bool bValue)
{
	QUICK_THROW("Must be a table")
}

void CRgdFile::VSetValueString(const char* sValue)
{
	QUICK_THROW("Must be a table")
}

void CRgdFile::VSetValueWString(const wchar_t* wsValue)
{
	QUICK_THROW("Must be a table")
}

IMetaNode::IMetaTable* CRgdFile::VGetValueMetatable()
{
	if(m_pDataChunk)
	{
		return new CMetaTable(&m_pDataChunk->RootEntry);
	}
	QUICK_THROW("No table present")
}

CRgdFile::CMetaNode::CMetaNode(CRgdFile::_RgdEntry* pData)
{
	m_pData = pData;
}

CRgdFile::CMetaNode::~CMetaNode()
{
}

IMetaNode::eDataTypes CRgdFile::CMetaNode::VGetType()
{
	return m_pData->Type;
}

const char* CRgdFile::CMetaNode::VGetName()
{
	return m_pData->sName;
}

unsigned long CRgdFile::CMetaNode::VGetNameHash()
{
	return m_pData->iHash;
}

float CRgdFile::CMetaNode::VGetValueFloat()
{
	if(m_pData->Type == DT_Float) return m_pData->Data.f;
	QUICK_THROW("Is not a float")
}

unsigned long CRgdFile::CMetaNode::VGetValueInteger()
{
	if(m_pData->Type == DT_Integer) return m_pData->Data.i;
	QUICK_THROW("Is not an integer")
}

bool CRgdFile::CMetaNode::VGetValueBool()
{
	if(m_pData->Type == DT_Bool) return m_pData->Data.b;
	QUICK_THROW("Is not a bool")
}

const char* CRgdFile::CMetaNode::VGetValueString()
{
	if(m_pData->Type == DT_String) return m_pData->Data.s;
	QUICK_THROW("Is not a string")
}

const wchar_t* CRgdFile::CMetaNode::VGetValueWString()
{
	if(m_pData->Type == DT_WString) return m_pData->Data.ws;
	QUICK_THROW("Is not a unicode string")
}

IMetaNode::IMetaTable* CRgdFile::CMetaNode::VGetValueMetatable()
{
	if(m_pData->Type == DT_Table || m_pData->Type == sk_TableInt) return new CMetaTable(m_pData);
	QUICK_THROW("Is not a metatable")
}

void CRgdFile::CMetaNode::VSetType(IMetaNode::eDataTypes eType)
{
	if(eType == m_pData->Type) return;
	if(eType == DT_NoData) QUICK_THROW("Type cannot be set to DT_NoData")
	_RgdEntryData oOldData = m_pData->Data;

	// Make new data
	switch(eType)
	{
	case DT_Float:
		switch(m_pData->Type)
		{
		case DT_Bool:
			m_pData->Data.f = oOldData.b ? 1.0f : 0.0f;
			break;
		case DT_Integer:
			m_pData->Data.f = (float)oOldData.i;
			break;
		case DT_String:
			m_pData->Data.f = atof(oOldData.s);
			break;
		case DT_WString:
			{
				char *sAscii = UnicodeToAscii(oOldData.ws);
				m_pData->Data.f = atof(sAscii);
				delete[] sAscii;
				break;
			}
		case DT_Table:
		case sk_TableInt:
			m_pData->Data.f = 0.0f;
			break;
		};
		break;
	case DT_Integer:
		switch(m_pData->Type)
		{
		case DT_Bool:
			m_pData->Data.i = oOldData.b ? 1 : 0;
			break;
		case DT_Float:
			m_pData->Data.i = (unsigned long)oOldData.f;
			break;
		case DT_String:
			m_pData->Data.i = strtoul(oOldData.s, 0, 10);
			break;
		case DT_WString:
			{
				m_pData->Data.i = wcstoul(oOldData.ws, 0, 10);
				break;
			}
		case DT_Table:
		case sk_TableInt:
			m_pData->Data.i = 0;
			break;
		};
		break;
	case DT_Bool:
		switch(m_pData->Type)
		{
		case DT_Float:
			m_pData->Data.b = ((oOldData.f < 0.0f) || (oOldData.f > 0.0f));
			break;
		case DT_Integer:
			m_pData->Data.b = (oOldData.i != 0);
			break;
		case DT_String:
			m_pData->Data.b = (oOldData.s && *oOldData.s);
			break;
		case DT_WString:
			m_pData->Data.b = (oOldData.ws && *oOldData.ws);
			break;
		case DT_Table:
		case sk_TableInt:
			m_pData->Data.b = false;
			break;
		};
		break;
	case DT_String:
		switch(m_pData->Type)
		{
		case DT_Bool:
			m_pData->Data.s = mystrdup(oOldData.b ? "true" : "false");
			break;
		case DT_Float:
			m_pData->Data.s = new char[32];
			_snprintf(m_pData->Data.s, 31, "%f", oOldData.f);
			break;
		case DT_Integer:
			m_pData->Data.s = new char[32];
			_snprintf(m_pData->Data.s, 31, "%lu", oOldData.i);
			break;
		case DT_WString:
			m_pData->Data.s = UnicodeToAscii(oOldData.ws);
			break;
		case DT_Table:
		case sk_TableInt:
			switch(m_pData->pExt->Type)
			{
			case DT_String:
				m_pData->Data.s	= mystrdup(m_pData->pExt->Data.s);
				break;
			case DT_WString:
				m_pData->Data.s	= UnicodeToAscii(m_pData->pExt->Data.ws);
				break;
			default:
				m_pData->Data.s = mystrdup("");
			};
			break;
		};
		break;
	case DT_WString:
		switch(m_pData->Type)
		{
		case DT_Bool:
			m_pData->Data.ws = mywcsdup(oOldData.b ? L"true" : L"false");
			break;
		case DT_Float:
			m_pData->Data.ws = new wchar_t[32];
			_snwprintf(m_pData->Data.ws, 31, L"%f", oOldData.f);
			break;
		case DT_Integer:
			m_pData->Data.ws = new wchar_t[32];
			_snwprintf(m_pData->Data.ws, 31, L"%lu", oOldData.i);
			break;
		case DT_String:
			m_pData->Data.ws = AsciiToUnicode(oOldData.s);
			break;
		case DT_Table:
		case sk_TableInt:
			switch(m_pData->pExt->Type)
			{
			case DT_String:
				m_pData->Data.ws = AsciiToUnicode(m_pData->pExt->Data.s);
				break;
			case DT_WString:
				m_pData->Data.ws = mywcsdup(m_pData->pExt->Data.ws);
				break;
			default:
				m_pData->Data.ws = mywcsdup(L"");
			};
			break;
		};
		break;
	case DT_Table:
	case sk_TableInt:
		m_pData->Data.t = new std::vector<_RgdEntry*>;
		m_pData->Data.t->push_back(m_pData->pExt = new _RgdEntry);
		m_pData->pExt->iHash = 0x49D60FAE;
		m_pData->pExt->sName = 0;
		m_pData->pExt->pExt = 0;
		m_pData->pExt->Data.b = false;
		m_pData->pExt->Type = DT_NoData;
		m_pData->pExt->pParentFile = m_pData->pParentFile;
		switch(m_pData->Type)
		{
			case DT_String:
				m_pData->pExt->Type = DT_String;
				m_pData->pExt->Data.s = mystrdup(oOldData.s);
				break;
			case DT_WString:
				m_pData->pExt->Type = DT_WString;
				m_pData->pExt->Data.ws = mywcsdup(oOldData.ws);
				break;
		};
		break;
	};
	// Free old data
	switch(m_pData->Type)
	{
	case DT_String:
		delete[] oOldData.s;
		break;
	case DT_WString:
		delete[] oOldData.ws;
		break;
	case DT_Table:
	case sk_TableInt:
		_RgdEntry oOldEntry;
		oOldEntry.Type = DT_Table;
		oOldEntry.Data = oOldData;
		_CleanRgdTable(&oOldEntry);
		break;
	};
	// Finish up
	m_pData->Type = eType;
}

void CRgdFile::CMetaNode::VSetName(const char* sName)
{
	if(m_pData->pParentFile && m_pData->pParentFile->m_pHashTable)
	{
		try
		{
			m_pData->iHash = m_pData->pParentFile->m_pHashTable->ValueToHash(sName);
			m_pData->sName = m_pData->pParentFile->m_pHashTable->HashToValue(m_pData->iHash);
		}
		catch(CRainmanException *pE)
		{
			throw new CRainmanException(pE, __FILE__, __LINE__, "Error setting name to \'%s\' (dictinary problem)", sName);
		}
		return;
	}
	throw new CRainmanException(0, __FILE__, __LINE__, "Error setting name to \'%s\' (no dictionary)", sName);
}

void CRgdFile::CMetaNode::VSetNameHash(unsigned long iHash)
{
	m_pData->sName = 0;
	m_pData->iHash = iHash;
	if(m_pData->pParentFile && m_pData->pParentFile->m_pHashTable) m_pData->sName = m_pData->pParentFile->m_pHashTable->HashToValue(iHash);
}

void CRgdFile::CMetaNode::VSetValueFloat(float fValue)
{
	if(m_pData->Type != DT_Float) QUICK_THROW("Is not a float")
	m_pData->Data.f = fValue;
}

void CRgdFile::CMetaNode::VSetValueInteger(unsigned long iValue)
{
	if(m_pData->Type != DT_Integer) QUICK_THROW("Is not an integer")
	m_pData->Data.i = iValue;
}

void CRgdFile::CMetaNode::VSetValueBool(bool bValue)
{
	if(m_pData->Type != DT_Bool) QUICK_THROW("Is not a bool")
	m_pData->Data.b = bValue;
}

void CRgdFile::CMetaNode::VSetValueString(const char* sValue)
{
	if(m_pData->Type != DT_String) QUICK_THROW("Is not a string")
	delete[] m_pData->Data.s;
	m_pData->Data.s = mystrdup(sValue);
}

void CRgdFile::CMetaNode::VSetValueWString(const wchar_t* wsValue)
{
	if(m_pData->Type != DT_WString) QUICK_THROW("Is not a unicode string")
	delete[] m_pData->Data.ws;
	m_pData->Data.ws = mywcsdup(wsValue);
}

CMemoryStore::COutStream* CRgdFile::CMetaNode::VGetNodeAsRainmanRgd()
{
	CMemoryStore::COutStream* pOut = CMemoryStore::OpenOutputStreamExt();
	_WriteRainmanRgdData(pOut, m_pData);
	return pOut;
}

CMemoryStore::COutStream* CRgdFile::VGetNodeAsRainmanRgd()
{
	CMemoryStore::COutStream* pOut = CMemoryStore::OpenOutputStreamExt();
	_WriteRainmanRgdData(pOut, &m_pDataChunk->RootEntry);
	return pOut;
}

void CRgdFile::CMetaNode::SGetNodeFromRainmanRgd(IFileStore::IStream* pInput, bool bSetName)
{
	_ReadRainmanRgdData(pInput, m_pData, bSetName);
}

void CRgdFile::SGetNodeFromRainmanRgd(IFileStore::IStream* pInput, bool bSetName)
{
	_ReadRainmanRgdData(pInput, &m_pDataChunk->RootEntry, bSetName);
}

static bool IsALLNumerical(const char* s)
{
	if(!s) return false;
	++s;
	if(*s == 0) return false;
	while(*s)
	{
		if(*s < '0' || *s > '9') return false;
		++s;
	}
	return true;
}

bool _SortCMetaTableChildren(CRgdFile::_RgdEntry* p1, CRgdFile::_RgdEntry* p2)
{
	if(p1->sName == 0 || p2->sName == 0)
	{
		if(p1->sName == 0 && p2->sName != 0) return true;
		if(p2->sName == 0 && p1->sName != 0) return false;
		return (p1->iHash < p2->iHash);
	}

	char *sUnderPos1, *sUnderPos2;
	sUnderPos1 = strrchr((char*) p1->sName, '_');
	sUnderPos2 = strrchr((char*) p2->sName, '_');
	if(IsALLNumerical(sUnderPos1) && IsALLNumerical(sUnderPos2))
	{
		*sUnderPos1 = *sUnderPos2 = 0;
		if(stricmp(p1->sName, p2->sName) == 0)
		{
			*sUnderPos1 = *sUnderPos2 = '_';
			long l1 = atol(sUnderPos1 + 1);
			long l2 = atol(sUnderPos2 + 1);
			return (l1 < l2);
		}
		else
		{
			*sUnderPos1 = *sUnderPos2 = '_';
			return (stricmp(p1->sName, p2->sName) < 0);
		}
	}
	return (stricmp(p1->sName, p2->sName) < 0);
}

CRgdFile::CMetaTable::CMetaTable(_RgdEntry* pData)
{
	m_pData = pData;
	for(std::vector<_RgdEntry*>::iterator itr = m_pData->Data.t->begin(); itr != m_pData->Data.t->end(); ++itr)
	{
		if((*itr) != m_pData->pExt)
			m_vecChildren.push_back(*itr);
	}
	sort(m_vecChildren.begin(), m_vecChildren.end(), _SortCMetaTableChildren);
}

CRgdFile::CMetaTable::~CMetaTable()
{
}

unsigned long CRgdFile::CMetaTable::VGetChildCount()
{
	return (unsigned long)m_vecChildren.size();
}

IMetaNode* CRgdFile::CMetaTable::VGetChild(unsigned long iIndex)
{
	if(iIndex >= VGetChildCount()) throw new CRainmanException(0, __FILE__, __LINE__, "Index %lu beyond %lu", iIndex, VGetChildCount());
	return new CRgdFile::CMetaNode(m_vecChildren[iIndex]);
}

IMetaNode::eDataTypes CRgdFile::CMetaTable::VGetReferenceType()
{
	if(m_pData->pExt) return m_pData->pExt->Type;
	return DT_NoData;
}

const char* CRgdFile::CMetaTable::VGetReferenceString()
{
	if(VGetReferenceType() == DT_String) return m_pData->pExt->Data.s;
	QUICK_THROW("No reference string")
}

const wchar_t* CRgdFile::CMetaTable::VGetReferenceWString()
{
	if(VGetReferenceType() == DT_WString) return m_pData->pExt->Data.ws;
	QUICK_THROW("No reference unicode string")
}

void CRgdFile::CMetaTable::VSetReferenceType(IMetaNode::eDataTypes eType)
{
	if(eType == DT_NoData)
	{
		if(m_pData->pExt)
		{
			for(std::vector<_RgdEntry*>::iterator itr = m_pData->Data.t->begin(); itr != m_pData->Data.t->end(); ++itr)
			{
				if((*itr) == m_pData->pExt)
				{
					m_pData->Data.t->erase(itr);
					break;
				}
			}
			if(m_pData->pExt->Type == DT_String) delete[] m_pData->pExt->Data.s;
			else if(m_pData->pExt->Type == DT_WString) delete[] m_pData->pExt->Data.ws;
			delete m_pData->pExt;
			m_pData->pExt = 0;
		}
		return;
	}
	else if(eType == DT_String || eType == DT_WString)
	{
		if(!m_pData->pExt)
		{
			m_pData->pExt = new _RgdEntry;
			m_pData->pExt->iHash = 0x49D60FAE;
			m_pData->pExt->pExt = 0;
			m_pData->pExt->sName = 0;
			m_pData->pExt->Type = eType;
			m_pData->Data.t->push_back(m_pData->pExt);
			if(eType == DT_String) m_pData->pExt->Data.s = mystrdup("");
			else m_pData->pExt->Data.ws = mywcsdup(L"");
		}
		else
		{
			if(m_pData->pExt->Type == DT_String) delete[] m_pData->pExt->Data.s;
			else if(m_pData->pExt->Type == DT_WString) delete[] m_pData->pExt->Data.ws;
			if(eType == DT_String)
			{
				m_pData->pExt->Data.s = mystrdup("");
				m_pData->pExt->Type = DT_String;
			}
			else
			{
				m_pData->pExt->Type = DT_WString;
				m_pData->pExt->Data.ws = mywcsdup(L"");
			}
		}
		return;
	}
	QUICK_THROW("Invalid type")
}

void CRgdFile::CMetaTable::VSetReferenceString(const char* sValue)
{
	if(!m_pData->pExt || m_pData->pExt->Type != DT_String) QUICK_THROW("Reference type is not string")
	delete[] m_pData->pExt->Data.s;
	m_pData->pExt->Data.s = mystrdup(sValue);
}

void CRgdFile::CMetaTable::VSetReferenceWString(const wchar_t* wsValue)
{
	if(!m_pData->pExt || m_pData->pExt->Type != DT_WString) QUICK_THROW("Reference type is not unicode string")
	delete[] m_pData->pExt->Data.ws;
	m_pData->pExt->Data.ws = mywcsdup(wsValue);
}

IMetaNode* CRgdFile::CMetaTable::VAddChild(const char* sName)
{
	_RgdEntry* pNew = new _RgdEntry;
	if(!pNew) QUICK_THROW("Memory allocate error")
	pNew->Data.b = false;
	pNew->Type = IMetaNode::DT_Bool;
	pNew->pExt = 0;
	pNew->pParentFile = m_pData->pParentFile;
	if(!m_pData->pParentFile->m_pHashTable) m_pData->pParentFile->m_pHashTable = new CRgdHashTable;
	pNew->sName = m_pData->pParentFile->m_pHashTable->HashToValue(pNew->iHash = m_pData->pParentFile->m_pHashTable->ValueToHash(sName));
	for(std::vector<_RgdEntry*>::iterator itr = m_pData->Data.t->begin(); itr != m_pData->Data.t->end(); ++itr)
	{
		if(((**itr).sName && pNew->sName && (stricmp((**itr).sName, pNew->sName) == 0)) || (pNew->iHash == (**itr).iHash))
		{
			delete pNew;
			QUICK_THROW("Matching child found already")
		}
	}
	m_pData->Data.t->push_back(pNew);
	m_vecChildren.push_back(pNew);
	sort(m_vecChildren.begin(), m_vecChildren.end(), _SortCMetaTableChildren);
	return CHECK_MEM(new CRgdFile::CMetaNode(pNew));
}

void CRgdFile::CMetaTable::VDeleteChild(unsigned long iIndex)
{
	if(iIndex >= VGetChildCount()) throw new CRainmanException(0, __FILE__, __LINE__, "Index %lu beyond %lu", iIndex, VGetChildCount());
	_RgdEntry *pToDelete = m_vecChildren[iIndex];
	for(std::vector<_RgdEntry*>::iterator itr = m_pData->Data.t->begin(); itr != m_pData->Data.t->end(); ++itr)
	{
		if((*itr) == pToDelete)
		{
			switch((**itr).Type)
			{
			case DT_Table:
			case sk_TableInt:
				_CleanRgdTable(*itr);
				break;
			case DT_String:
				if((**itr).Data.s) delete[] (**itr).Data.s;
				break;
			case DT_WString:
				if((**itr).Data.ws) delete[] (**itr).Data.ws;
				break;
			};
			delete *itr;
			m_pData->Data.t->erase(itr);
			goto good; // evil but effecient
		}
	}
	QUICK_THROW("Internal error")
good:
	m_vecChildren.erase(m_vecChildren.begin() + iIndex);
}

void CRgdFile::_ReadRainmanRgdData(IFileStore::IStream* pInput, CRgdFile::_RgdEntry* pDestination, bool bSetName)
{
	// Name
	unsigned long iHash, iNameLen;
	pInput->VRead(1, sizeof(long), &iHash);
	pInput->VRead(1, sizeof(long), &iNameLen);
	if(bSetName)
	{
		if(iNameLen == 0)
		{
			pDestination->sName = 0;
			pDestination->iHash = iHash;
			if(pDestination->pParentFile && pDestination->pParentFile->m_pHashTable) pDestination->sName = pDestination->pParentFile->m_pHashTable->HashToValue(iHash);
		}
		else
		{
			char* sName = new char[iNameLen+1];
			pInput->VRead(iNameLen, 1, sName);
			sName[iNameLen] = 0;
			if(pDestination->pParentFile == 0 || pDestination->pParentFile->m_pHashTable == 0)
			{
				delete[] sName;
				throw new CRainmanException(__FILE__, __LINE__, "Cannot paste data as hash table missing");
			}
			if(iHash == 0)
			{
				pDestination->iHash = pDestination->pParentFile->m_pHashTable->ValueToHash(sName);
				pDestination->sName = pDestination->pParentFile->m_pHashTable->HashToValue(pDestination->iHash);
			}
			else
			{
				pDestination->iHash = iHash;
				pDestination->sName = pDestination->pParentFile->m_pHashTable->HashToValue(pDestination->iHash);
			}
			if(strcmp(pDestination->sName, sName) != 0)
			{
				pDestination->iHash = pDestination->pParentFile->m_pHashTable->ValueToHash(sName);
				pDestination->sName = pDestination->pParentFile->m_pHashTable->HashToValue(pDestination->iHash);
			}
			delete[] sName;
		}
	}
	else
	{
		pInput->VSeek(iNameLen);
	}

	// Value
	switch(pDestination->Type)
	{
	case DT_String:
		delete[] pDestination->Data.s;
		break;
	case DT_WString:
		delete[] pDestination->Data.ws;
		break;
	case DT_Table:
	case sk_TableInt:
		_CleanRgdTable(pDestination);
		break;
	};
	pDestination->pExt = 0;

	char cDataType;
	pInput->VRead(1, 1, &cDataType);
	pDestination->Type = (eDataTypes) cDataType;
	unsigned long iDataLen;
	pInput->VRead(1, sizeof(long), &iDataLen);

	switch(pDestination->Type)
	{
	case DT_Float:
		{
			if(iDataLen == sizeof(float))
			{
				pInput->VRead(1, sizeof(float), &pDestination->Data.f);
			}
			else if(iDataLen == sizeof(double))
			{
				double d;
				pInput->VRead(1, sizeof(double), &d);
				pDestination->Data.f = (float)d;
			}
			else
			{
				throw new CRainmanException(0, __FILE__, __LINE__, "Data length of %lu unsupported for float data", iDataLen);
			}
			break;
		}
	case DT_Integer:
		{
			if(iDataLen == sizeof(long))
			{
				pInput->VRead(1, sizeof(long), &pDestination->Data.i);
			}
			else if(iDataLen == sizeof(short))
			{
				short s;
				pInput->VRead(1, sizeof(short), &s);
				pDestination->Data.i = (long)s;
			}
			else
			{
				throw new CRainmanException(0, __FILE__, __LINE__, "Data length of %lu unsupported for integer data", iDataLen);
			}
			break;
		}
	case DT_Bool:
		{
			if(iDataLen == sizeof(bool))
			{
				pInput->VRead(1, sizeof(bool), &pDestination->Data.b);
			}
			else
			{
				throw new CRainmanException(0, __FILE__, __LINE__, "Data length of %lu unsupported for boolean data", iDataLen);
			}
			break;
		}
	case DT_String:
		{
			pDestination->Data.s = new char[iDataLen + 1];
			pInput->VRead(iDataLen, 1, pDestination->Data.s);
			pDestination->Data.s[iDataLen] = 0;
			break;
		}
	case DT_WString:
		{
			pDestination->Data.ws = new wchar_t[iDataLen + 1];
			pInput->VRead(iDataLen, 2, pDestination->Data.ws);
			pDestination->Data.ws[iDataLen] = 0;
			break;
		}
	case DT_Table:
	case sk_TableInt:
		{
			pDestination->Data.t = new std::vector<_RgdEntry*>;
			while(iDataLen)
			{
				--iDataLen;
				_RgdEntry *pNew = new _RgdEntry;
				pNew->Data.b = false;
				pNew->Type = IMetaNode::DT_Bool;
				pNew->pExt = 0;
				pNew->pParentFile = pDestination->pParentFile;
				if(!pDestination->pParentFile->m_pHashTable) pDestination->pParentFile->m_pHashTable = new CRgdHashTable;
				_ReadRainmanRgdData(pInput, pNew, true);
				pDestination->Data.t->push_back(pNew);
				if(pNew->iHash == 0x49D60FAE)
					pDestination->pExt = pNew;
			}
			break;
		}
	default:
		pDestination->Type = DT_NoData;
	};
}

void CRgdFile::_WriteRainmanRgdData(CMemoryStore::COutStream* pOutput, CRgdFile::_RgdEntry* pSource)
{
	pOutput->VWrite(1, sizeof(long), &pSource->iHash);
	unsigned long iL = pSource->sName ? strlen(pSource->sName) : 0;
	pOutput->VWrite(1, sizeof(long), &iL);
	if(iL) pOutput->VWrite(iL, 1, pSource->sName);
	unsigned char cType = pSource->Type;
	pOutput->VWrite(1, 1, &cType);
	switch(pSource->Type)
	{
	case DT_Float:
		iL = sizeof(float);
		pOutput->VWrite(1, sizeof(long), &iL);
		pOutput->VWrite(1, sizeof(float), &pSource->Data.f);
		break;

	case DT_Integer:
		iL = sizeof(long);
		pOutput->VWrite(1, sizeof(long), &iL);
		pOutput->VWrite(1, sizeof(long), &pSource->Data.i);
		break;

	case DT_Bool:
		iL = sizeof(bool);
		pOutput->VWrite(1, sizeof(long), &iL);
		pOutput->VWrite(1, sizeof(bool), &pSource->Data.b);
		break;

	case DT_String:
		iL = pSource->Data.s ? strlen(pSource->Data.s) : 0;
		pOutput->VWrite(1, sizeof(long), &iL);
		if(iL) pOutput->VWrite(iL, 1, pSource->Data.s);
		break;

	case DT_WString:
		iL = pSource->Data.ws ? wcslen(pSource->Data.ws) : 0;
		pOutput->VWrite(1, sizeof(long), &iL);
		if(iL) pOutput->VWrite(iL, 2, pSource->Data.ws);
		break;

	case DT_Table:
	case sk_TableInt:
		{
			std::vector<_RgdEntry*>* pT = pSource->Data.t;
			iL = pT->size();
			pOutput->VWrite(1, sizeof(long), &iL);
			for(std::vector<_RgdEntry*>::iterator itr = pT->begin(); itr != pT->end(); ++itr)
			{
				_WriteRainmanRgdData(pOutput, *itr);
			}
			break;
		}

	default:
		iL = 0;
		pOutput->VWrite(1, sizeof(long), &iL);
		break;
	};
}

