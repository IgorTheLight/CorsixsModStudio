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

#include "CChunkyFile.h"
#include "memdebug.h"

CChunkyFile::CChunkyFile()
{
	m_sHeader[0] = 0;
	m_iVersion = 0;
	m_iUnknown1 = 0;
	m_iUnknown2 = 0;
	m_iUnknown3 = 0;
	m_iUnknown4 = 0;
}

CChunkyFile::~CChunkyFile()
{
	for(std::vector<CChunk*>::iterator itr = m_vChunks.begin(); itr != m_vChunks.end(); ++itr)
	{
		delete *itr;
	}
}

void CChunkyFile::New(long iVersion)
{
	for(std::vector<CChunk*>::iterator itr = m_vChunks.begin(); itr != m_vChunks.end(); ++itr)
	{
		delete *itr;
	}

	strcpy( (char*)m_sHeader, "Relic Chunky\x0D\x0A\x1A");

	m_iVersion = iVersion;
	m_iUnknown1 = 1;
	m_iUnknown2 = 36;
	m_iUnknown3 = 28;
	m_iUnknown4 = 1;
}

CChunkyFile::CChunk* CChunkyFile::CChunk::InsertBefore(size_t iBefore, const char* sName, CChunk::eTypes eType)
{
	CChunk *pChunk = CHECK_MEM(new CChunk);

	pChunk->m_eType = eType;
	strncpy(pChunk->m_sName, sName, 4);
	pChunk->m_iVersion = 0;
	pChunk->m_iDataLength = 0;
	pChunk->m_sDescriptor = new char[1];
	pChunk->m_sDescriptor[0] = 0;
	pChunk->m_iUnknown1 = (eType == CChunk::T_Folder ? 0 : -1);
	pChunk->m_iUnknown2 = 0;
	pChunk->m_pData = new char[1];

	m_vChildren.insert(m_vChildren.begin() + iBefore, pChunk);

	return pChunk;
}

CChunkyFile::CChunk* CChunkyFile::CChunk::AppendNew(const char* sName, CChunk::eTypes eType)
{
	CChunk *pChunk = CHECK_MEM(new CChunk);

	pChunk->m_eType = eType;
	strncpy(pChunk->m_sName, sName, 4);
	pChunk->m_iVersion = 0;
	pChunk->m_iDataLength = 0;
	pChunk->m_sDescriptor = new char[1];
	pChunk->m_sDescriptor[0] = 0;
	pChunk->m_iUnknown1 = (eType == CChunk::T_Folder ? 0 : -1);
	pChunk->m_iUnknown2 = 0;
	pChunk->m_pData = new char[1];

	m_vChildren.push_back(pChunk);

	return pChunk;
}

CChunkyFile::CChunk* CChunkyFile::AppendNew(const char* sName, CChunk::eTypes eType)
{
	CChunk *pChunk = CHECK_MEM(new CChunk);

	pChunk->m_eType = eType;
	strncpy(pChunk->m_sName, sName, 4);
	pChunk->m_iVersion = 0;
	pChunk->m_iDataLength = 0;
	pChunk->m_sDescriptor = new char[1];
	pChunk->m_sDescriptor[0] = 0;
	pChunk->m_iUnknown1 = (eType == CChunk::T_Folder ? 0 : -1);
	pChunk->m_iUnknown2 = 0;
	pChunk->m_pData = new char[1];

	m_vChunks.push_back(pChunk);

	return pChunk;
}

void CChunkyFile::Load(IFileStore::IStream* pStream)
{
	try
	{
		pStream->VRead(16, 1, (void*) m_sHeader);

		if(strcmp(m_sHeader, "Relic Chunky\x0D\x0A\x1A") != 0)
		{
			throw new CRainmanException(0, __FILE__, __LINE__, "Unrecognised header (%s)", m_sHeader);
		}

		pStream->Read(m_iVersion);
		pStream->VRead(1, sizeof(long), &m_iUnknown1);
		if(m_iVersion >= 3)
		{
			pStream->VRead(1, sizeof(long), &m_iUnknown2);
			pStream->VRead(1, sizeof(long), &m_iUnknown3);
			pStream->VRead(1, sizeof(long), &m_iUnknown4);
		}
	}
	catch(CRainmanException *pE)
	{
		throw new CRainmanException(__FILE__, __LINE__, "Error reading from stream", pE);
	}

	CChunk *pChunk = 0;

	try
	{
		pChunk = CHECK_MEM(new CChunk);
		while(pChunk->_Load(pStream, m_iVersion))
		{
			m_vChunks.push_back(pChunk);
			pChunk = CHECK_MEM(new CChunk);
		}
	}
	catch(CRainmanException *pE)
	{
		throw new CRainmanException(pE, __FILE__, __LINE__, "Error loading root chunk (#%lu)", (unsigned long)m_vChunks.size() );
	}

	delete pChunk;
}

void CChunkyFile::Save(IFileStore::IOutputStream* pStream)
{
	try
	{
		pStream->VWrite(16, 1, (void*) m_sHeader);
		pStream->VWrite(1, sizeof(long), &m_iVersion);
		pStream->VWrite(1, sizeof(long), &m_iUnknown1);
		pStream->VWrite(1, sizeof(long), &m_iUnknown2);
		pStream->VWrite(1, sizeof(long), &m_iUnknown3);
		pStream->VWrite(1, sizeof(long), &m_iUnknown4);
	}
	catch(CRainmanException *pE)
	{
		throw new CRainmanException(__FILE__, __LINE__, "Error writing to stream", pE);
	}

	for(std::vector<CChunk*>::iterator itr = m_vChunks.begin(); itr != m_vChunks.end(); ++itr)
	{
		(**itr)._Save(pStream);
	}
}

bool CChunkyFile::CChunk::_Load(IFileStore::IStream* pStream, long iChunkyVersion)
{
	char sType[5];
	sType[4] = 0;

	try
	{
		pStream->VRead(4, 1, (void*)sType);
	}
	catch(CRainmanException *pE)
	{
		pE->destroy();
		return false;
	}

	if(strcmp(sType, "DATA") == 0) m_eType = T_Data;
	else if(strcmp(sType, "FOLD") == 0) m_eType = T_Folder;
	else throw new CRainmanException(0, __FILE__, __LINE__, "Unrecognised chunk type \'%s\'", sType);

	unsigned long iDescriptorLength = 0;
	try
	{
		pStream->VRead(4, 1, (void*)m_sName);
		pStream->VRead(1, sizeof(long), &m_iVersion);
		pStream->VRead(1, sizeof(unsigned long), &m_iDataLength);
		pStream->VRead(1, sizeof(unsigned long), &iDescriptorLength);
		if(iChunkyVersion >= 3)
		{
			pStream->VRead(1, sizeof(unsigned long), &m_iUnknown1);
			pStream->VRead(1, sizeof(unsigned long), &m_iUnknown2);
		}
	}
	catch(CRainmanException *pE)
	{
		throw new CRainmanException(__FILE__, __LINE__, "Error reading from stream", pE);
	}

	if(m_sDescriptor) delete[] m_sDescriptor;
	m_sDescriptor = 0;
	m_sDescriptor = CHECK_MEM(new char[iDescriptorLength + 1]);
	m_sDescriptor[iDescriptorLength] = 0;

	try
	{
		pStream->VRead(iDescriptorLength, 1, (void*)m_sDescriptor);
	}
	catch(CRainmanException *pE)
	{
		throw new CRainmanException(__FILE__, __LINE__, "Error reading from stream", pE);
	}

	if(m_eType == T_Folder)
	{
		long iDataEnd = 0;
		try
		{
			iDataEnd = pStream->VTell() + (long) m_iDataLength;
		}
		catch(CRainmanException *pE)
		{
			throw new CRainmanException(__FILE__, __LINE__, "Cannot get position from stream", pE);
		}
		unsigned long iChildN = 0;
		while(1)
		{
			try
			{
				if(pStream->VTell() >= iDataEnd) break;
			}
			catch(CRainmanException *pE)
			{
				throw new CRainmanException(__FILE__, __LINE__, "Cannot get position from stream", pE);
			}

			CChunk *pChunk = CHECK_MEM(new CChunk);

			try
			{
				if(!pChunk->_Load(pStream, iChunkyVersion)) throw new CRainmanException(__FILE__, __LINE__, "End of stream reached");
			}
			catch(CRainmanException *pE)
			{
				delete pChunk;
				throw new CRainmanException(pE, __FILE__, __LINE__, "Error reading child #%lu of FOLD%s", iChildN, m_sName);
			}

			m_vChildren.push_back(pChunk);
			++iChildN;
		}
	}
	else
	{
		if(m_pData) delete[] m_pData;
		m_pData = 0;
		m_pData = CHECK_MEM(new char[m_iDataLength]);
		try
		{
			pStream->VRead(m_iDataLength, 1, (void*)m_pData);
		}
		catch(CRainmanException *pE)
		{
			throw new CRainmanException(__FILE__, __LINE__, "Error reading from stream", pE);
		}
	}

	return true;
}

unsigned long CChunkyFile::CChunk::_FoldUpdateSize()
{
	unsigned long iDataLength;
	iDataLength = 0;
	if(m_eType == T_Folder)
	{
		iDataLength = 0;
		for(std::vector<CChunk*>::iterator itr = m_vChildren.begin(); itr != m_vChildren.end(); ++itr)
		{
			iDataLength += (**itr)._FoldUpdateSize();
		}
	}
	else
	{
		iDataLength += m_iDataLength;
	}
	m_iDataLength = iDataLength;

	iDataLength += 8 + (5 * sizeof(long));
	if(m_sDescriptor != 0 && strlen(m_sDescriptor)) iDataLength += strlen(m_sDescriptor) + 1;
	return iDataLength;
}

void CChunkyFile::CChunk::_Save(IFileStore::IOutputStream* pStream)
{
	switch(m_eType)
	{
	case T_Data:
		pStream->VWrite(4, 1, "DATA");
		break;
	case T_Folder:
		pStream->VWrite(4, 1, "FOLD");
		break;
	default:
		pStream->VWrite(4, 1, "????");
		break;
	};

	unsigned long iDescriptorLength = m_sDescriptor ? (strlen(m_sDescriptor) + 1) : 0;
	if(iDescriptorLength == 1) iDescriptorLength = 0;

	_FoldUpdateSize();

	try
	{
		pStream->VWrite(4, 1, (void*)m_sName);
		pStream->VWrite(1, sizeof(long), &m_iVersion);
		pStream->VWrite(1, sizeof(unsigned long), &m_iDataLength);
		pStream->VWrite(1, sizeof(unsigned long), &iDescriptorLength);
		pStream->VWrite(1, sizeof(unsigned long), &m_iUnknown1);
		pStream->VWrite(1, sizeof(unsigned long), &m_iUnknown2);
		pStream->VWrite(iDescriptorLength, 1, (void*)m_sDescriptor);
	}
	catch(CRainmanException *pE)
	{
		throw new CRainmanException(__FILE__, __LINE__, "Error writing to stream", pE);
	}

	if(m_eType == T_Folder)
	{
		for(std::vector<CChunk*>::iterator itr = m_vChildren.begin(); itr != m_vChildren.end(); ++itr)
		{
			(**itr)._Save(pStream);
		}
	}
	else
	{
		pStream->VWrite(m_iDataLength, 1, (void*)m_pData);
	}
}

CChunkyFile::CChunk::CChunk()
{
	m_eType = T_Folder;
	m_sName[0] = 'N';
	m_sName[1] = 'U';
	m_sName[2] = 'L';
	m_sName[3] = 'L';
	m_sName[4] = 0;
	m_sDescriptor = 0;
	m_iVersion = 0;
	m_iUnknown1 = 0;
	m_iUnknown2 = 0;
	m_pData = 0;
	m_iDataLength = 0;
}

CChunkyFile::CChunk::eTypes CChunkyFile::CChunk::GetType() const
{
	return m_eType;
}

long CChunkyFile::CChunk::GetVersion() const
{
	return m_iVersion;
}

const char* CChunkyFile::CChunk::GetName() const
{
	return (const char*) m_sName;
}

const char* CChunkyFile::CChunk::GetDescriptor() const
{
	return (const char*)(m_sDescriptor ? m_sDescriptor : "");
}

CChunkyFile::CChunk* CChunkyFile::GetChildByName(const char* sName, CChunkyFile::CChunk::eTypes eType) const
{
	for(std::vector<CChunk*>::const_iterator itr = m_vChunks.begin(); itr != m_vChunks.end(); ++itr)
	{
		CChunkyFile::CChunk::eTypes eT = (**itr).m_eType;
		const char* s = (**itr).m_sName;
		if( (**itr).m_eType == eType && strcmp(sName, (**itr).m_sName) == 0) return *itr;
	}
	return 0;
}

size_t CChunkyFile::CChunk::GetChildCount() const
{
	return m_vChildren.size();
}

CChunkyFile::CChunk* CChunkyFile::CChunk::GetChild(size_t iN) const
{
	return m_vChildren[iN];
}

void CChunkyFile::CChunk::RemoveChild(size_t iN)
{
	m_vChildren.erase(m_vChildren.begin() + iN);
}

CChunkyFile::CChunk* CChunkyFile::GetChild(size_t iN) const
{
	return m_vChunks[iN];
}

size_t CChunkyFile::GetChildCount() const
{
	return m_vChunks.size();
}

void CChunkyFile::RemoveChild(size_t iN)
{
	m_vChunks.erase(m_vChunks.begin() + iN);
}

CChunkyFile::CChunk* CChunkyFile::CChunk::GetChildByName(const char* sName, CChunkyFile::CChunk::eTypes eType) const
{
	for(std::vector<CChunk*>::const_iterator itr = m_vChildren.begin(); itr != m_vChildren.end(); ++itr)
	{
		if( (**itr).m_eType == eType && strcmp(sName, (**itr).m_sName) == 0) return *itr;
	}
	return 0;
}

CMemoryStore::CStream* CChunkyFile::CChunk::GetData()
{
	return CMemoryStore::OpenStreamExt(m_pData, m_iDataLength, false);
}

char* CChunkyFile::CChunk::GetDataRaw()
{
	return m_pData;
}

unsigned long CChunkyFile::CChunk::GetDataLength()
{
	return m_iDataLength;
}

void CChunkyFile::CChunk::SetVersion(long iValue)
{
	m_iVersion = iValue;
}

void CChunkyFile::CChunk::SetUnknown1(long iValue)
{
	m_iUnknown1 = iValue;
}

void CChunkyFile::CChunk::SetDescriptor(const char* sValue)
{
	if(m_sDescriptor) delete[] m_sDescriptor;
	size_t iL = strlen(sValue) + 1;
	m_sDescriptor = new char[iL];
	memcpy(m_sDescriptor, sValue, iL);
}

void CChunkyFile::CChunk::SetData(CMemoryStore::COutStream* pStream)
{
	if(m_pData) delete[] m_pData;
	m_pData = new char[m_iDataLength = pStream->GetDataLength()];
	memcpy(m_pData, pStream->GetData(), m_iDataLength);
}

CChunkyFile::CChunk::~CChunk()
{
	if(m_sDescriptor) delete[] m_sDescriptor;
	if(m_pData) delete[] m_pData;

	for(std::vector<CChunk*>::iterator itr = m_vChildren.begin(); itr != m_vChildren.end(); ++itr)
	{
		delete *itr;
	}
}

