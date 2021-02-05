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

#ifndef _C_MEMORY_STORE_H_
#define _C_MEMORY_STORE_H_

#include "gnuc_defines.h"
#include "IFileStore.h"
#include <memory.h>

class RAINMAN_API CMemoryStore : public IFileStore
{
public:
	//! Constructor
	/*!
		\return Returns nothing and never throws an exception
	*/
	CMemoryStore(void);
	virtual ~CMemoryStore(void);

	//! A input/read stream implementation
	/*!
		\sa IFileStore::IStream
	*/
	class RAINMAN_API CStream : public IFileStore::IStream
	{
	protected:
		friend class CMemoryStore;
		CStream(void);
		char *m_pBegin, *m_pCurrent;
		unsigned long m_iLength, m_iLengthLeft;
		bool m_bDeleteWhenDone;

	public:
		virtual ~CStream(void);

		virtual void VRead(unsigned long iItemCount, unsigned long iItemSize, void* pDestination);
		virtual void VSeek(long iPosition, IFileStore::IStream::SeekLocation SeekFrom = SL_Current);
		virtual long VTell();
	};

	class RAINMAN_API COutStream : public IFileStore::IOutputStream
	{
	protected:
		friend class CMemoryStore;
		COutStream();
		char *m_pBegin, *m_pCurrent;
		unsigned long m_iLength, m_iLengthLeft, m_iBufferLength;
	public:
		virtual ~COutStream(void);
		virtual void VWrite(unsigned long iItemCount, unsigned long iItemSize, const void* pSource);
		const char* GetData();
		unsigned long GetDataLength();

		virtual void VRead(unsigned long iItemCount, unsigned long iItemSize, void* pDestination);
		virtual void VSeek(long iPosition, IFileStore::IStream::SeekLocation SeekFrom = SL_Current);
		virtual long VTell();
	};

	//! Get the memory store read to create memory stream
	/*!
		\param[in] pUnused Currently unused
		\return Returns nothing and never throws an exception
	*/
	virtual void VInit(void* pUnused = 0);

	//! Open an input/read stream
	/*!
		\param[in] sFile This must be a memory range defined by MemoryRange() - this value will also by "delete"d - so only pass the return value from MemoryRange() to VOpenStream() once.
		\return Returns a valid stream (a CMemoryStore::CStream* via runtime polymorphism) or throws an exception. Cannot return zero. Remember to delete this pointer when you are done using it.
	*/
	virtual IStream* VOpenStream(const char* sFile);

	static CStream* OpenStreamExt(char* pBegin, unsigned long iLength, bool bDeleteWhenDone);

	//! Open an output/write stream
	/*!
		Creates a block of automatically resizing memory that can be written to as a stream
		\param[in] sIdentifier Ignored
		\param[in] bEraseIfPresent Ignored
		\return Returns a valid stream (a CMemoryStore::COutStream* via runtime polymorphism) or throws an exception. Cannot return zero. You must delete this returned pointer when you are finished with it.
	*/
	virtual IOutputStream* VOpenOutputStream(const char* sIdentifier, bool bEraseIfPresent);

	static COutStream* OpenOutputStreamExt();

	//! Define a memory range to pass to VOpenStream()
	/*!
		\param[in] pBegin The position in memory at which the memory range begins
		\param[in] iLength The length of the memory range (in bytes)
		\return Returns a valid pointer or throws a CRainmanException
	*/
	char* MemoryRange(void* pBegin, unsigned long iLength);

protected:
	bool m_bInited;

	struct _MemRange
	{
		char* p;
		unsigned long i;
	};
};

#endif

