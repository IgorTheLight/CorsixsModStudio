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

#include "CUcsTransaction.h"
#include "memdebug.h"
#include "Exception.h"

CUcsTransaction::CUcsTransaction(CUcsFile* pUcsObject)
{
	m_pRawFile = pUcsObject;
}

CUcsTransaction::~CUcsTransaction(void)
{
	_Clean();
}

void CUcsTransaction::Save(const char* sFile)
{
	for(std::map<unsigned long, wchar_t*>::iterator itr = m_mapValues.begin(); itr != m_mapValues.end(); ++itr)
	{
		m_pRawFile->ReplaceString(itr->first, itr->second);
	}
	m_mapValues.clear();

	try
	{
		m_pRawFile->Save(sFile);
	}
	CATCH_THROW("Raw object failed to write")
}

std::map<unsigned long, wchar_t*>* CUcsTransaction::GetRawMap()
{
	if(m_mapValues.size() == 0) return m_pRawFile->GetRawMap();

	// Need to make a faux map of a combination of our edits and the original
	m_mapCombinationValues.clear();
	m_mapCombinationValues = *m_pRawFile->GetRawMap();

	for(std::map<unsigned long, wchar_t*>::iterator itr = m_mapValues.begin(); itr != m_mapValues.end(); ++itr)
	{
		m_mapCombinationValues[itr->first] = itr->second;
	}

	return &m_mapCombinationValues;
}

void CUcsTransaction::Load(IFileStore::IStream *pStream)
{
	_Clean();

	try
	{
		m_pRawFile->Load(pStream);
	}
	CATCH_THROW("Raw object failed to load")
}

const wchar_t* CUcsTransaction::ResolveStringID(unsigned long iID) 
{
	const wchar_t* pS = m_mapValues[iID];
	if(!pS) return m_pRawFile->ResolveStringID(iID);
	return pS;
}

CUcsFile* CUcsTransaction::GetRawObject()
{
	return m_pRawFile;
}