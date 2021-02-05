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

#include "CRgdHashTable.h"
#include "memdebug.h"
#include "Exception.h"

extern "C" {
typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned       char ub1;
ub4 hash(ub1 * k,ub4 length,ub4 initval);
ub4 hash3(ub1 * k,ub4 length,ub4 initval);
}

static char* mystrdup(const char* sStr)
{
	char* s = new char[strlen(sStr) + 1];
	if(s == 0) return 0;
	strcpy(s, sStr);
	return s;
}

CRgdHashTable::CRgdHashTable(void)
{
	for(int i = 1; i < 10000; ++i)
	{
		char sBuf[24];
		_Value Val;
		Val.bCustom = false;
		Val.sString = CHECK_MEM(mystrdup(itoa(i,sBuf,10)));
		m_mHashTable[hash((ub1*)sBuf, (ub4) strlen(sBuf), 0)] = Val;
	}
}

CRgdHashTable::~CRgdHashTable(void)
{
	_Clean();
}

void CRgdHashTable::New()
{
	_Clean();
}

void CRgdHashTable::FillUnknownList(std::vector<unsigned long>& oList)
{
	for(std::map<unsigned long, CRgdHashTable::_Value>::iterator itr = m_mHashTable.begin(); itr != m_mHashTable.end(); ++itr)
	{
		if(itr->second.sString == 0) oList.push_back(itr->first);
	}
}

static char* fgetline(FILE *f, unsigned int iInitSize = 32)
{
	unsigned int iTotalLen;
	if(f == 0) throw new CRainmanException(__FILE__, __LINE__, "No file");
	if(iInitSize < 4) iInitSize = 4;
	iTotalLen = iInitSize;
	char *sBuffer = new char[iInitSize];
	char *sReadTo = sBuffer;
	if(sBuffer == 0) throw new CRainmanException(0, __FILE__, __LINE__, "Failed to allocate %u", iInitSize);

	do
	{
		if(fgets(sReadTo, iInitSize, f) == 0)
		{
			if(feof(f))
			{
				if(sReadTo[strlen(sReadTo) - 1] == '\n') sReadTo[strlen(sReadTo) - 1] = 0;
				return sBuffer;
			}
			delete[] sBuffer;
			throw new CRainmanException(__FILE__, __LINE__, "Failed to read string");
		}
		if(sReadTo[strlen(sReadTo) - 1] == '\n')
		{
			size_t n = strlen(sReadTo) - 1;
			sReadTo[n] = 0;
			while(n > 0)
			{
				--n;
				if(sReadTo[n] == '\n' || sReadTo[n] == '\r') sReadTo[n] = 0;
				else break;
			}
			return sBuffer;
		}
		iTotalLen += iInitSize;
		char *sTmp = new char[iTotalLen];
		if(sTmp == 0)
		{
			delete[] sBuffer;
			throw new CRainmanException(0, __FILE__, __LINE__, "Failed to allocate %u", iTotalLen);
		}
		strcpy(sTmp, sBuffer);
		delete[] sBuffer;
		sBuffer = sTmp;
		sReadTo = sBuffer + strlen(sBuffer);
	}while(1);
}

void CRgdHashTable::ExtendWithDictionary(const char* sFile, bool bCustom)
{
	FILE *fFile = fopen(sFile, "rb");
	if(fFile == 0) throw new CRainmanException(0, __FILE__, __LINE__, "Failed to open file \'%s\'", sFile);
	while(!feof(fFile))
	{
		char* sLine;
		try
		{
			sLine = fgetline(fFile);
		}
		catch(CRainmanException* pE)
		{
			fclose(fFile);
			throw new CRainmanException(__FILE__, __LINE__, "Failed to read line", pE);
		}
		char *sBaseLine = sLine;
		bool bUnk = false;
		if( (strncmp(sLine, "# 0x", 4) == 0) && (strncmp(sLine + 12, " is an unknown value!!", 22) == 0))
		{
			bUnk = true;
			sLine[0] = ' ';
			sLine[12] = '=';
		}
		if(sLine[0] != '#')
		{
			unsigned long iKey = 0;
			int iBitsRead = -8;
			// Read in key
			while((*sLine != '=') && *sLine)
			{
				if(iBitsRead == -8)
				{
					if(*sLine == '0') iBitsRead = -4;
				}
				else if(iBitsRead == -4)
				{
					if(*sLine == 'x' || *sLine == 'X') iBitsRead = 0;
				}
				else if(iBitsRead < 32)
				{
					if(*sLine >= '0' && *sLine <= '9')
					{
						iKey <<= 4;
						iKey |= (*sLine - '0');
						iBitsRead += 4;
					}
					else if(*sLine >= 'a' && *sLine <= 'f')
					{
						iKey <<= 4;
						iKey |= (*sLine - 'a' + 10);
						iBitsRead += 4;
					}
					else if(*sLine >= 'A' && *sLine <= 'F')
					{
						iKey <<= 4;
						iKey |= (*sLine - 'A' + 10);
						iBitsRead += 4;
					}
				}

				++sLine;
			}
			if(*sLine && *sLine == '=')
			{
				++sLine;
				
				// Read in value
				char *sValue = 0;
				if(!bUnk)
				{
					sValue = new char[strlen(sLine) + 1];
					if(sValue == 0)
					{
						delete[] sBaseLine;
						fclose(fFile);
						throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
					}
					char *sTmp = sValue;
					memset(sValue, 0, strlen(sLine) + 1);
					while(*sLine)
					{
						if( (*sLine != ' ') && (*sLine != '\t') && (*sLine != '\r') && (*sLine != '\n') )
						{
							*sTmp = *sLine;
							++sTmp;
						}
						++sLine;
					}
				}
				_Value Val;

				Val = m_mHashTable[iKey];
				if(Val.sString)
				{
					Val.bCustom = Val.bCustom && bCustom;
					delete[] sValue;
				}
				else
				{
					Val.sString = sValue;
					Val.bCustom = bCustom;
				}
				m_mHashTable[iKey] = Val;
			}

			delete[] sBaseLine;
		}
		else
		{
			delete[] sLine;
		}
	}

	fclose(fFile);
}

void CRgdHashTable::XRefWithStringList(const char* sFile)
{
	FILE *fFile = fopen(sFile, "rb");
	if(fFile == 0) throw new CRainmanException(0, __FILE__, __LINE__, "Failed to open file \'%s\'", sFile);
	while(!feof(fFile))
	{
		char* sLine;
		try
		{
			sLine = fgetline(fFile);
		}
		catch(CRainmanException* pE)
		{
			fclose(fFile);
			throw new CRainmanException(__FILE__, __LINE__, "Failed to read line", pE);
		}

		unsigned long iKey = hash((ub1*)sLine, (ub4) strlen(sLine), 0);

		if(m_mHashTable.find(iKey) != m_mHashTable.end() && m_mHashTable[iKey].sString == 0)
		{
			_Value Val;
			Val.bCustom = true;
			Val.sString = CHECK_MEM(mystrdup(sLine));
			m_mHashTable[iKey] = Val;
		}

		delete[] sLine;
	}
}

void CRgdHashTable::SaveCustomKeys(const char* sFile)
{
	FILE *fFile = fopen(sFile, "wb");
	if(fFile == 0) throw new CRainmanException(0, __FILE__, __LINE__, "Failed to open file \'%s\'", sFile);

	fputs("#RGD_DIC\n# This dictionary is generated from any keys that are \'discovered\'\n", fFile);

	for(std::map<unsigned long, _Value>::iterator itr = m_mHashTable.begin(); itr != m_mHashTable.end(); ++itr)
	{
		if(itr->second.bCustom)
		{
			if(itr->second.sString == 0) fputs("# ", fFile);
			// Output key
			fputs("0x", fFile);
			unsigned long iMask = 0xF0000000;
			for(int i = 7; i >= 0; --i)
			{
				fputc("0123456789ABCDEF"[(itr->first & iMask) >> (i << 2)], fFile);
				iMask >>= 4;
			}

			// Output value
			if(itr->second.sString == 0)
			{
				fputs(" is an unknown value!!\n", fFile);
			}
			else
			{
				fputc('=', fFile);
				fputs(itr->second.sString, fFile);
				fputc('\n', fFile);
			}
		}
	}

	fclose(fFile);
}

const char* CRgdHashTable::HashToValue(unsigned long iHash)
{
	const char* s = m_mHashTable[iHash].sString;
	if(s == 0)
	{
		m_mHashTable[iHash].bCustom = true;
		return 0;
	}
	return s;
}

unsigned long CRgdHashTable::ValueToHash(const char* sValue)
{
	if(sValue[0] == '0' && sValue[1] == 'x')
	{
		for(int i = 2; i < 10; ++i)
		{
			if(!(
				(sValue[i] >= '0' && sValue[i] <= '9') ||
				(sValue[i] >= 'a' && sValue[i] <= 'f') ||
				(sValue[i] >= 'A' && sValue[i] <= 'F')
				))
				goto nothex;
		}
		if(sValue[10] == 0)
		{
			unsigned long iKey = 0;
			for(int i = 2; i < 10; ++i)
			{
				iKey <<= 4;
				if(sValue[i] >= '0' && sValue[i] <= '9') iKey |= (sValue[i] - '0');
				if(sValue[i] >= 'a' && sValue[i] <= 'f') iKey |= (sValue[i] - 'a' + 10);
				if(sValue[i] >= 'A' && sValue[i] <= 'F') iKey |= (sValue[i] - 'A' + 10);
			}
			return iKey;
		}
	}
nothex:
	unsigned long iKey = hash((ub1*)sValue, (ub4) strlen(sValue), 0);
	if(m_mHashTable[iKey].sString == 0)
	{
		_Value Val;
		Val.bCustom = false;
		const char* sTmp = sValue;
		do
		{
			if( ((*sTmp) < '0') || ((*sTmp) > '9') )
			{
				Val.bCustom = true;
				break;
			}
			++sTmp;
		}while(*sTmp);

		Val.sString = CHECK_MEM(mystrdup(sValue));
		m_mHashTable[iKey] = Val;
	}
	return iKey;
}

unsigned long CRgdHashTable::ValueToHashStatic(const char* sValue, size_t iValueLen)
{
	if(sValue[0] == '0' && sValue[1] == 'x' && iValueLen == 10)
	{
		for(int i = 2; i < 10; ++i)
		{
			if(!(
				(sValue[i] >= '0' && sValue[i] <= '9') ||
				(sValue[i] >= 'a' && sValue[i] <= 'f') ||
				(sValue[i] >= 'A' && sValue[i] <= 'F')
				))
				goto nothex;
		}
		unsigned long iKey = 0;
		for(int i = 2; i < 10; ++i)
		{
			iKey <<= 4;
			if(sValue[i] >= '0' && sValue[i] <= '9') iKey |= (sValue[i] - '0');
			if(sValue[i] >= 'a' && sValue[i] <= 'f') iKey |= (sValue[i] - 'a' + 10);
			if(sValue[i] >= 'A' && sValue[i] <= 'F') iKey |= (sValue[i] - 'A' + 10);
		}
		return iKey;
	}
nothex:
	unsigned long iKey = hash((ub1*)sValue, (ub4) iValueLen, 0);
	return iKey;
}

unsigned long CRgdHashTable::ValueToHashStatic(const char* sValue)
{
		if(sValue[0] == '0' && sValue[1] == 'x')
	{
		for(int i = 2; i < 10; ++i)
		{
			if(!(
				(sValue[i] >= '0' && sValue[i] <= '9') ||
				(sValue[i] >= 'a' && sValue[i] <= 'f') ||
				(sValue[i] >= 'A' && sValue[i] <= 'F')
				))
				goto nothex;
		}
		if(sValue[10] == 0)
		{
			unsigned long iKey = 0;
			for(int i = 2; i < 10; ++i)
			{
				iKey <<= 4;
				if(sValue[i] >= '0' && sValue[i] <= '9') iKey |= (sValue[i] - '0');
				if(sValue[i] >= 'a' && sValue[i] <= 'f') iKey |= (sValue[i] - 'a' + 10);
				if(sValue[i] >= 'A' && sValue[i] <= 'F') iKey |= (sValue[i] - 'A' + 10);
			}
			return iKey;
		}
	}
nothex:
	unsigned long iKey = hash((ub1*)sValue, (ub4) strlen(sValue), 0);
	return iKey;
}

CRgdHashTable::_Value::_Value()
{
	sString = 0;
	bCustom = false;
}

void CRgdHashTable::_Clean()
{
	for(std::map<unsigned long, _Value>::iterator itr = m_mHashTable.begin(); itr != m_mHashTable.end(); ++itr)
	{
		if(itr->second.sString) delete[] itr->second.sString;
	}
}

