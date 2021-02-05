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

#include "CUcsFile.h"
#include "memdebug.h"
#include "Exception.h"

CUcsFile::CUcsFile(void)
{
}

CUcsFile::~CUcsFile(void)
{
	_Clean();
}

bool CUcsFile::IsDollarString(const char* s)
{
	if(!s || *s != '$') return false;
	++s;
	if(*s == 0) return false;
	while(*s)
	{
		if(*s < '0' || *s > '9') return false;
		++s;
	}
	return true;
}

bool CUcsFile::IsDollarString(const wchar_t* s)
{
	if(!s || *s != '$') return false;
	++s;
	if(*s == 0) return false;
	while(*s)
	{
		if(*s < '0' || *s > '9') return false;
		++s;
	}
	return true;
}

void CUcsFile::New()
{
	_Clean();
	return;
}

void CUcsFile::Save(const char* sFile)
{
	FILE* f = fopen(sFile, "wb");
	if(!f) throw new CRainmanException(0, __FILE__, __LINE__, "Cannot open file \'%s\' in mode \'wb\'", sFile);
	unsigned short iHeader = 0xFEFF;
	fwrite(&iHeader, 2, 1, f);
	for(std::map<unsigned long, wchar_t*>::iterator itr = m_mapValues.begin(); itr != m_mapValues.end(); ++itr)
	{
		if(itr->second)
		{
			// get the string representation of the number and write it
			wchar_t sBuffer[35];
			_ltow((long)itr->first, sBuffer, 10); 
			fputws(sBuffer, f);
			wchar_t t;

			// write the tab delimeter and value
			t = 0x09;
			fwrite(&t,2,1,f);
			fputws(itr->second, f);

			// write newline
			t = 0x0D;
			fwrite(&t,2,1,f);
			t = 0x0A;
			fwrite(&t,2,1,f);
		}
	}
	fclose(f);
}

std::map<unsigned long, wchar_t*>* CUcsFile::GetRawMap()
{
	return &m_mapValues;
}

const std::map<unsigned long, wchar_t*>* CUcsFile::GetRawMap() const
{
	return &m_mapValues;
}

static wchar_t* mywcsdup(const char* sStr)
{
	wchar_t* s = new wchar_t[strlen(sStr) + 1];
	if(s == 0) return 0;
	for(size_t i = 0; i <= strlen(sStr); ++i) s[i] = sStr[i];
	return s;
}

static wchar_t* mywcsdup(const wchar_t* sStr)
{
	/*
		Equivalent to wcsdup(), except uses "new" instead of "malloc"
		(and thus "delete" instead of "free")
	*/
	wchar_t* s = new wchar_t[wcslen(sStr) + 1];
	if(s == 0) return 0;
	wcscpy(s, sStr);
	return s;
}

static wchar_t* readwideline(IFileStore::IStream *pStream, unsigned long iInitSize = 32)
{
	/*
		reads one unicode line from the stream
		does not throw CRainmanException

		Words == 2 bytes (eg. computing "word", not linguistic "word")
	*/
	wchar_t* sBuffer = CHECK_MEM(new wchar_t[iInitSize]);
	memset(sBuffer, 0, sizeof(wchar_t) * iInitSize);
	unsigned long iWordsRead = 0;

	try
	{
		pStream->VRead(1,sizeof(wchar_t), sBuffer + iWordsRead);
	}
	catch(CRainmanException *pE)
	{
		delete[] sBuffer;
		pE->destroy();
		return 0;
	}

	try
	{
		do
		{
			if(++iWordsRead >= (iInitSize - 1)) // _shouldn't_ be '>' but you never know...
			{
				wchar_t* sTmp = new wchar_t[iInitSize <<= 1];
				if(sTmp == 0)
				{
					delete[] sBuffer;
					return 0;
				}
				memset(sTmp, 0, sizeof(wchar_t) * iInitSize);
				wcscpy(sTmp, sBuffer);
				delete[] sBuffer;
				sBuffer = sTmp;
			}
			if((char)sBuffer[iWordsRead-1] == '\x0A') break;
			pStream->VRead(1,sizeof(wchar_t), sBuffer + iWordsRead);
		} while(1);
	}
	IGNORE_EXCEPTIONS

	return sBuffer;
}

static char* readasciiline(IFileStore::IStream *pStream, unsigned long iInitSize = 32)
{
	/*
		reads one unicode line from the stream
		does not throw CRainmanException

		Words == 2 bytes (eg. computing "word", not linguistic "word")
	*/
	char* sBuffer = CHECK_MEM(new char[iInitSize]);
	memset(sBuffer, 0, sizeof(char) * iInitSize);
	unsigned long iWordsRead = 0;

	try
	{
		pStream->VRead(1,sizeof(char), sBuffer + iWordsRead);
	}
	catch(CRainmanException *pE)
	{
		delete[] sBuffer;
		pE->destroy();
		return 0;
	}

	try
	{
		do
		{
			if(++iWordsRead >= (iInitSize - 1)) // _shouldn't_ be '>' but you never know...
			{
				char* sTmp = new char[iInitSize <<= 1];
				if(sTmp == 0)
				{
					delete[] sBuffer;
					return 0;
				}
				memset(sTmp, 0, sizeof(char) * iInitSize);
				strcpy(sTmp, sBuffer);
				delete[] sBuffer;
				sBuffer = sTmp;
			}
			if((char)sBuffer[iWordsRead-1] == '\x0A') break;
			pStream->VRead(1,sizeof(char), sBuffer + iWordsRead);
		} while(1);
	}
	IGNORE_EXCEPTIONS

	return sBuffer;
}

void CUcsFile::LoadDat(IFileStore::IStream *pStream)
{
	_Clean();

	char *sLine = 0;
	while(sLine = readasciiline(pStream))
	{
		unsigned long iNumber = 0;
		unsigned long i = 0;
		if(sLine[i] >= '0' && sLine[i] <= '9')
		{
			while(sLine[i] && sLine[i] >= '0' && sLine[i] <= '9')
			{
				iNumber *= 10;
				iNumber += (sLine[i] - '0');
				++i;
			}
			if(i != 0 && sLine[i] && sLine[++i] && m_mapValues[iNumber] == 0) // silenty ignore duplicate values
			{
				wchar_t *sString = mywcsdup(sLine + i);
				if(sString)
				{
					wchar_t *sTmp;
					if(sTmp = wcschr(sString, 0x0D)) *sTmp = 0;
					if(sTmp = wcschr(sString, 0x0A)) *sTmp = 0;

					m_mapValues[iNumber] = sString;
				}
			}
		}

		delete[] sLine;
	}
}

void CUcsFile::Load(IFileStore::IStream *pStream)
{
	_Clean();

	unsigned short iHeader;
	try
	{
		pStream->VRead(1, sizeof(short), &iHeader);
	}
	CATCH_THROW("Failed to read header")

	if(iHeader != 0xFEFF)
	{
		// UCS file has invalid header - what should I do?
		// I'll do what relic do; throw an error!
		throw new CRainmanException(0, __FILE__, __LINE__, "Invalid header (%u)", iHeader);
	}

	wchar_t *sLine = 0;
	while(sLine = readwideline(pStream))
	{
		unsigned long iNumber = 0;
		unsigned long i = 0;
		while(sLine[i] && sLine[i] >= '0' && sLine[i] <= '9')
		{
			iNumber *= 10;
			iNumber += (sLine[i] - '0');
			++i;
		}
		if(i != 0 && sLine[i] && sLine[++i] && m_mapValues[iNumber] == 0) // silenty ignore duplicate values
		{
			wchar_t *sString = mywcsdup(sLine + i);
			if(sString)
			{
				wchar_t *sTmp;
				if(sTmp = wcschr(sString, 0x0D)) *sTmp = 0;
				if(sTmp = wcschr(sString, 0x0A)) *sTmp = 0;

				m_mapValues[iNumber] = sString;
			}
		}

		delete[] sLine;
	}
}

const wchar_t* CUcsFile::ResolveStringID(unsigned long iID) 
{
	const wchar_t* pS = m_mapValues[iID];
	if(!pS) return 0; //throw new CRainmanException(0, __FILE__, __LINE__, "$%ul no key", iID);
	return pS;
}

void CUcsFile::SetString(unsigned long iID, const wchar_t* pString)
{
	delete[] m_mapValues[iID];
	if(pString == 0)
	{
		m_mapValues.erase(iID);
		return;
	}
	wchar_t* pTmp = mywcsdup(pString);
	if(pTmp == 0) 
		throw new CRainmanException(__FILE__, __LINE__, "Cannot duplicate string (out of memory?)");
	m_mapValues[iID] = pTmp;
}

void CUcsFile::ReplaceString(unsigned long iID, wchar_t* pString)
{
	delete[] m_mapValues[iID];
	if(pString == 0)
	{
		m_mapValues.erase(iID);
		return;
	}
	m_mapValues[iID] = pString;
}

void CUcsFile::_Clean()
{
	for(std::map<unsigned long, wchar_t*>::iterator itr = m_mapValues.begin(); itr != m_mapValues.end(); ++itr)
		delete[] itr->second;
	m_mapValues.erase(m_mapValues.begin(), m_mapValues.end());
}

