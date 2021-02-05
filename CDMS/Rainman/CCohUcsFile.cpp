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

/*
#include "CCohUcsFile.h"

CCohUcsFile::CCohUcsFile()
{
	m_sFileComment = 0;
	m_iBegin = 0;
	m_iEnd = 0;
}

CCohUcsFile::~CCohUcsFile()
{
	_Clean();
}

bool CCohUcsFile::New()
{
	_Clean();
	return true;
}

static char* readstreamline(IFileStore::IStream *pStream, unsigned long iInitSize = 32)
{
	char* sBuffer = new char[iInitSize];
	memset(sBuffer, 0, sizeof(char) * iInitSize);
	if(sBuffer == 0) return 0;
	unsigned long iCharsRead = 0;

	if(!pStream->VRead(1,sizeof(char), sBuffer + iCharsRead))
	{
		delete[] sBuffer;
		return 0;
	}

	do
	{
		if(++iCharsRead >= (iInitSize - 1)) // _shouldn't_ be '>' but you never know...
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
		if(sBuffer[iCharsRead-1] == '\x0A') break;
	} while(pStream->VRead(1,sizeof(char), sBuffer + iCharsRead));

	return sBuffer;
}

static char* readnum(char *sStr, unsigned long* pRet)
{
	unsigned long iRet = 0;
	while(*sStr >= '0' && *sStr <= '9')
	{
		iRet *= 10;
		iRet += (*sStr - '0');
		++sStr;
	}
	*pRet = iRet;
	return sStr;
}

bool CCohUcsFile::Load(IFileStore::IStream *pStream)
{
	_Clean();
	char *sLine = 0, *sLastComment = 0, *sTmp;
	_tRange* pActiveRange = 0;
	bool bFileRangeSet = false;
	while(sLine = readstreamline(pStream))
	{
		// Chop off the newline
		if(sTmp = strchr(sLine, 0x0D)) *sTmp = 0;
		if(sTmp = strchr(sLine, 0x0A)) *sTmp = 0;

		// Look for a comment
		sTmp = strstr(sLine, "//");
		if(sTmp)
		{
			*sTmp = 0;
			++sTmp;
			while(*sTmp == '/' || *sTmp == ' ') ++sTmp;
			if(strlen(sTmp))
			{
				if(sLastComment) free(sLastComment);
				sLastComment = strdup(sTmp);
			}
		}

		// Parse line
		if(strlen(sLine))
		{
			if(*sLine >= '0' && *sLine <= '9')
			{
				 // Number line
				unsigned long iNum;
				sTmp = readnum(sLine, &iNum);
				if(sTmp && *sTmp == '\t' && pActiveRange)
				{
					pActiveRange->mapEntries[iNum] = 0;//strdup(sTmp + 1);
				}
			}
			else
			{
				 // Command line
				if(strncmp(sLine, "filerange", 9) == 0)
				{
					sTmp = sLine + 9;
					while(*sTmp && (*sTmp < '0' || *sTmp > '9') ) ++sTmp;
					if(*sTmp)
					{
						sTmp = readnum(sTmp, &m_iBegin);
						if(sTmp && *sTmp)
						{
							sTmp = readnum(sTmp, &m_iEnd);
						}
					}
				}
			}
		}
	}

	//!!!!!! TODO
	return false;
}

void CCohUcsFile::_Clean()
{
	if(m_sFileComment) delete[] m_sFileComment;

	m_sFileComment = 0;
	m_iBegin = 0;
	m_iEnd = 0;

	for(std::vector<_tRange*>::iterator itr = m_vRanges.begin(); itr != m_vRanges.end(); ++itr)
	{
		delete *itr;
	}
	m_vRanges.clear();
}

CCohUcsFile::_tRange::_tRange()
{
	sComment = 0;
	iBegin = 0;
	iEnd = 0;
}

CCohUcsFile::_tRange::~_tRange()
{
	if(sComment) delete[] sComment;
	for(std::map<unsigned long, wchar_t*>::iterator itr = mapEntries.begin(); itr != mapEntries.end(); ++itr)
	{
		free(itr->second);
	}
}
*/

