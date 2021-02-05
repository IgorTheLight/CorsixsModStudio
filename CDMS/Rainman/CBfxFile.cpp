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

#include "CBfxFile.h"
#include <algorithm>

CBfxFile::CBfxFile()
{
	m_bConvertTableIntToTable = false;
}

void CBfxFile::SaveAsBfxLua(IFileStore::IOutputStream *pStream, lua_State *Lmap)
{
	size_t iKeyCount = m_pDataChunk->RootEntry.Data.t->size();
	for(size_t i = 0; i < iKeyCount; ++i)
	{
		_SaveRaw(pStream, m_pDataChunk->RootEntry.Data.t->at(i), Lmap, false, 0);
	}
}

void CBfxFile::_SaveRaw(IFileStore::IOutputStream *pStream, _RgdEntry *pSource, lua_State *Lmap, bool bNumericID, size_t iIndentLvl)
{
	for(size_t i = 0; i < iIndentLvl; ++i) pStream->VWriteString("\t");
	//if(iIndentLvl == 1) bNumericID = true;
	if(!bNumericID && pSource->sName)
	{
		pStream->VWriteString(pSource->sName);
		pStream->VWriteString(" = ");
	}

	char sFpBuffer[35];

	switch(pSource->Type)
	{
	case DT_Float:
		sprintf((char*)sFpBuffer, "%.5g,\r\n", pSource->Data.f);
		if(strchr(sFpBuffer,'.'))
		{
			int iDec,iSign;
			char* sDigits = _fcvt(pSource->Data.f, 5, &iDec, &iSign);
			char* sMyBuf = new char[strlen(sDigits) + 8 + (iDec < 0 ? -iDec : 0) ];
			char* sPtr = sMyBuf;
			if(iSign)
			{
				*sPtr = '-';
				++sPtr;
			}
			if(iDec <= 0)
			{
				*sPtr = '0';
				++sPtr;
				*sPtr = '.';
				++sPtr;
				for(int i = 0; i < -iDec; ++i)
				{
					*sPtr = '0';
					++sPtr;
				}
				iDec = -1;
			}
			for(int i = 0; sDigits[i]; ++i)
			{
				if(i == iDec)
				{
					*sPtr = '.';
					++sPtr;
				}
				*sPtr = sDigits[i];
				++sPtr;
			}
			*sPtr = ',';
			++sPtr;
			*sPtr = '\r';
			++sPtr;
			*sPtr = '\n';
			++sPtr;
			*sPtr = '\0';
			++sPtr;
			pStream->VWriteString(sMyBuf);
			delete[] sMyBuf;
			break;
		}
		pStream->VWriteString(sFpBuffer);
		break;

	case DT_Integer:
		sprintf((char*)sFpBuffer, "%lu,\r\n", pSource->Data.i);
		pStream->VWriteString(sFpBuffer);
		break;

	case DT_Bool:
		sprintf((char*)sFpBuffer, "%s,\r\n", pSource->Data.b ? "true" : "false");
		pStream->VWriteString(sFpBuffer);
		break;

	case DT_String:
		{
			const char *sToPrint = pSource->Data.s;
			if(Lmap)
			{
				lua_pushstring(Lmap, pSource->Data.s);
				lua_gettable(Lmap, -2);
				sToPrint = lua_tostring(Lmap, -1);
			}

			size_t iL = strlen(sToPrint);
			for(size_t i = 0; i < iL; ++i)
			{
				if(sToPrint[i] == '\\' || sToPrint[i] == '\'' || sToPrint[i] == '\"') goto use_autoescape;
			}
			pStream->VWriteString("\"");
			pStream->VWriteString(sToPrint);
			pStream->VWriteString("\",\r\n");
			if(Lmap) lua_pop(Lmap, 1);
			break;
use_autoescape:
			//pStream->VWriteString("[[");
			//pStream->VWriteString(sToPrint);
			//pStream->VWriteString("]],\r\n");
			pStream->VWriteString("\"");
			for(size_t i = 0; i < iL; ++i)
			{
				if(sToPrint[i] == '\\' || sToPrint[i] == '\'' || sToPrint[i] == '\"')
				{
					pStream->VWriteString("\\");
				}
				pStream->VWrite(1,1,sToPrint+i);
			}
			pStream->VWriteString("\",\r\n");
			if(Lmap) lua_pop(Lmap, 1);
			break;
		}

	case DT_WString:
		pStream->VWriteString("nil -- unicode string\r\n");
		break;

	case DT_Table:
	case sk_TableInt:
		{

			pStream->VWriteString(" \r\n");
			for(size_t i = 0; i < iIndentLvl; ++i) pStream->VWriteString("\t");
			pStream->VWriteString("{\r\n");

			std::vector<_RgdEntry*> vEntries;
			for(std::vector<_RgdEntry*>::iterator itr = pSource->Data.t->begin(); itr != pSource->Data.t->end(); ++itr)
			{
				vEntries.push_back(*itr);
			}

			bool bIsNumericT = true;
			for(size_t i = 0; i < vEntries.size(); ++i)
			{
				_RgdEntry* p = vEntries[i];
				const char* sT = p->sName;
				if(p->sName == 0)
				{
					bIsNumericT = false;
					break;
				}
				do
				{
					if(*sT < '0' || *sT > '9')
					{
						bIsNumericT = false;
						break;
					}
					++sT;
				}while(*sT);
			}
			sort(vEntries.begin(), vEntries.end(), bIsNumericT ? _SortOutEntriesNum : _SortCMetaTableChildren);

			for(size_t i = 0; i < vEntries.size(); ++i)
			{
				_SaveRaw(pStream, vEntries.at(i), Lmap, bIsNumericT, iIndentLvl + 1);
			}
			for(size_t i = 0; i < iIndentLvl; ++i) pStream->VWriteString("\t");
			if(iIndentLvl == 0)
				pStream->VWriteString("}\r\n\r\n");
			else
				pStream->VWriteString("},\r\n");
			break;
		}

	case DT_NoData:
		pStream->VWriteString("nil -- no data\r\n");
		break;
	};
}