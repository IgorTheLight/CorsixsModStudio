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

#include "CMemoryStore.h"
#include "memdebug.h"
#include "Exception.h"

CMemoryStore::CMemoryStore(void)
{
}

CMemoryStore::~CMemoryStore(void)
{
}

CMemoryStore::CStream::CStream(void)
{
	m_bDeleteWhenDone = false;
}

CMemoryStore::CStream::~CStream(void)
{
	if(m_bDeleteWhenDone) delete[] m_pBegin;
}

void CMemoryStore::VInit(void* pUnused)
{
	m_bInited = true;
}

char* CMemoryStore::MemoryRange(void* pBegin, unsigned long iLength)
{
	_MemRange* Range = new _MemRange;
	if(Range == 0) throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
	Range->i = iLength;
	Range->p = (char*)pBegin;

	return (char*)Range;
}

IFileStore::IStream* CMemoryStore::VOpenStream(const char* sFile)
{
	if(sFile == 0) throw new CRainmanException(__FILE__, __LINE__, "No stream descriptor");
	_MemRange* Range = (_MemRange*)sFile;
	CStream* pStream = new CStream();
	if(pStream == 0) throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
	pStream->m_pBegin = Range->p;
	pStream->m_pCurrent = Range->p;
	pStream->m_iLength = Range->i;
	pStream->m_iLengthLeft = Range->i;

	delete Range;
	return pStream;
}

CMemoryStore::CStream* CMemoryStore::OpenStreamExt(char* pBegin, unsigned long iLength, bool bDeleteWhenDone)
{
	if(pBegin == 0) throw new CRainmanException(__FILE__, __LINE__, "No memory input");
	CStream* pStream = new CStream();
	if(pStream == 0) throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
	pStream->m_pBegin = pBegin;
	pStream->m_pCurrent = pBegin;
	pStream->m_iLength = iLength;
	pStream->m_iLengthLeft = iLength;
	pStream->m_bDeleteWhenDone = bDeleteWhenDone;

	return pStream;
}

IFileStore::IOutputStream* CMemoryStore::VOpenOutputStream(const char* sIdentifier, bool bEraseIfPresent)
{
	return new COutStream;
}

CMemoryStore::COutStream* CMemoryStore::OpenOutputStreamExt()
{
	return new COutStream;
}

void CMemoryStore::CStream::VRead(unsigned long iItemCount, unsigned long iItemSize, void* _pDestination)
{
	union
	{
		void* pDestination;
		long lDestination;
	};
	pDestination = _pDestination;

	if((iItemCount * iItemSize) > m_iLengthLeft) throw new CRainmanException(__FILE__, __LINE__, "Trying to read beyond EOF");
	while(iItemCount)
	{
		memcpy(pDestination, m_pCurrent, iItemSize);
		m_pCurrent += iItemSize;
		lDestination += iItemSize;
		m_iLengthLeft -= iItemSize;
		--iItemCount;
	}
}

CMemoryStore::COutStream::COutStream()
{
	m_pBegin = m_pCurrent = new char[m_iBufferLength = 131072]; // 128kb
	if(m_pBegin == 0) throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
	m_iLength = m_iLengthLeft = 0;
}

CMemoryStore::COutStream::~COutStream(void)
{
	delete[] m_pBegin;
}

void CMemoryStore::COutStream::VWrite(unsigned long iItemCount, unsigned long iItemSize, const void* _pSource)
{
	union
	{
		const void* pSource;
		long lSource;
	};
	pSource = _pSource;

	unsigned long iDataToWrite = iItemCount * iItemSize;
	while((m_iLength + iDataToWrite) > m_iBufferLength)
	{
		// Expand buffer
		char* pNewBuffer = new char[m_iBufferLength <<= 1];
		memcpy(pNewBuffer, m_pBegin, m_iLength);
		delete[] m_pBegin;
		m_pCurrent = (m_pCurrent - m_pBegin) + pNewBuffer;
		m_pBegin = pNewBuffer;
	}
	// Do write
	while(iItemCount)
	{
		memcpy(m_pCurrent, pSource, iItemSize);
		m_pCurrent += iItemSize;
		lSource += iItemSize;
		if(m_iLengthLeft == 0)
		{
			m_iLength += iItemSize;
		}
		else
		{
			unsigned long iAmtOverwritten = m_iLengthLeft >= iItemSize ? iItemSize : m_iLengthLeft;
			m_iLengthLeft -= iAmtOverwritten;
			m_iLength += (iItemSize - iAmtOverwritten);
		}
		--iItemCount;
	}
}

const char* CMemoryStore::COutStream::GetData()
{
	return m_pBegin;
}

unsigned long CMemoryStore::COutStream::GetDataLength()
{
	return m_iLength;
}

void CMemoryStore::COutStream::VRead(unsigned long iItemCount, unsigned long iItemSize, void* _pDestination)
{
	union
	{
		void* pDestination;
		long lDestination;
	};
	pDestination = _pDestination;

	if((iItemCount * iItemSize) > m_iLengthLeft) throw new CRainmanException(__FILE__, __LINE__, "Trying to read beyond EOF");
	while(iItemCount)
	{
		memcpy(pDestination, m_pCurrent, iItemSize);
		m_pCurrent += iItemSize;
		lDestination += iItemSize;
		m_iLengthLeft -= iItemSize;
		--iItemCount;
	}
}

void CMemoryStore::COutStream::VSeek(long iPosition, IFileStore::IStream::SeekLocation SeekFrom)
{
	switch(SeekFrom)
	{
	case IFileStore::IStream::SL_Current:
		if(iPosition == 0) return;
		if((m_pCurrent + iPosition) < m_pBegin) throw new CRainmanException(__FILE__, __LINE__, "Cannot seek to location");
		if((m_iLengthLeft - iPosition) < 0) throw new CRainmanException(__FILE__, __LINE__, "Cannot seek to location");
		m_iLengthLeft -= iPosition;
		m_pCurrent += iPosition;
		return;

	case IFileStore::IStream::SL_Root:
		if(iPosition < 0 || iPosition > ((long)m_iLength)) throw new CRainmanException(__FILE__, __LINE__, "Cannot seek to location");
		m_pCurrent = m_pBegin + iPosition;
		m_iLengthLeft = m_iLength - iPosition;
		return;

	case IFileStore::IStream::SL_End:
		if(iPosition > 0 || (-iPosition) > ((long)m_iLength)) throw new CRainmanException(__FILE__, __LINE__, "Cannot seek to location");
		m_pCurrent = m_pBegin + m_iLength + iPosition;
		m_iLengthLeft = (-iPosition);
		return;
	};
	throw new CRainmanException(__FILE__, __LINE__, "Unknown SeekFrom");
}

long CMemoryStore::COutStream::VTell()
{
	return m_iLength - m_iLengthLeft;
}

void CMemoryStore::CStream::VSeek(long iPosition, IFileStore::IStream::SeekLocation SeekFrom)
{
	switch(SeekFrom)
	{
	case IFileStore::IStream::SL_Current:
		if(iPosition == 0) return;
		if((m_pCurrent + iPosition) < m_pBegin) throw new CRainmanException(__FILE__, __LINE__, "Cannot seek to location");
		if((m_iLengthLeft - iPosition) < 0) throw new CRainmanException(__FILE__, __LINE__, "Cannot seek to location");
		m_iLengthLeft -= iPosition;
		m_pCurrent += iPosition;
		return;

	case IFileStore::IStream::SL_Root:
		if(iPosition < 0 || iPosition > ((long)m_iLength)) throw new CRainmanException(__FILE__, __LINE__, "Cannot seek to location");
		m_pCurrent = m_pBegin + iPosition;
		m_iLengthLeft = m_iLength - iPosition;
		return;

	case IFileStore::IStream::SL_End:
		if(iPosition > 0 || (-iPosition) > ((long)m_iLength)) throw new CRainmanException(__FILE__, __LINE__, "Cannot seek to location");
		m_pCurrent = m_pBegin + m_iLength + iPosition;
		m_iLengthLeft = (-iPosition);
		return;
	};
	throw new CRainmanException(__FILE__, __LINE__, "Unknown SeekFrom");
}

long CMemoryStore::CStream::VTell()
{
	return m_iLength - m_iLengthLeft;
}

