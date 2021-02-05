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

#ifndef _CHUNKY_FILE_H_
#define _CHUNKY_FILE_H_

#include "gnuc_defines.h"
#include "CMemoryStore.h"
#include "Exception.h"
#include <vector>

class RAINMAN_API CChunkyFile
{
public:
	CChunkyFile();
	~CChunkyFile();

	void Load(IFileStore::IStream* pStream);
	void Save(IFileStore::IOutputStream* pStream);
	void New(long iVersion);

	class RAINMAN_API CChunk
	{
	public:
		CChunk();
		~CChunk();

		enum eTypes
		{
			T_Folder,
			T_Data
		};

		// Applicable to all
		eTypes GetType() const;
		const char* GetName() const;
		long GetVersion() const;
		const char* GetDescriptor() const;

		void SetVersion(long iValue);
		void SetDescriptor(const char* sValue);
		void SetUnknown1(long iValue);

		// Only applicable to T_Data
		CMemoryStore::CStream* GetData();
		char* GetDataRaw();
		unsigned long GetDataLength();

		void SetData(CMemoryStore::COutStream* pStream);

		// Only applicable to T_Folder
		size_t GetChildCount() const;
		CChunk* GetChild(size_t iN) const;
		CChunk* GetChildByName(const char* sName, eTypes eType) const;
		CChunk* AppendNew(const char* sName, CChunk::eTypes eType);
		CChunk* InsertBefore(size_t iBefore, const char* sName, CChunk::eTypes eType);
		void RemoveChild(size_t iN);

	protected:
		friend class CChunkyFile;

		//! Read one chunk from file
		/*!
			Reads one FOLDxxxx or DATAxxxx chunk from the stream.
			If it is a FOLDxxxx chunk then all the children will be read aswell,
			if it is a DATAxxxx chunk then the chunk data will be read into memory.

			\param[in] pStream Input stream
			\return Returns true if all went well, false if the very first read operation gave an error (eg. end of stream), throws an exception for all other errors.
		*/
		bool _Load(IFileStore::IStream* pStream, long iChunkyVersion);

		void _Save(IFileStore::IOutputStream* pStream);

		unsigned long _FoldUpdateSize();

		eTypes m_eType;
		char m_sName[5];
		char* m_sDescriptor;
		long m_iVersion;

		long m_iUnknown1, m_iUnknown2;

		char* m_pData;
		unsigned long m_iDataLength;

		std::vector<CChunk*> m_vChildren;
	};

	long GetVersion() const;

	size_t GetChildCount() const;
	CChunk* GetChild(size_t iN) const;
	CChunk* GetChildByName(const char* sName, CChunk::eTypes eType) const;
	void RemoveChild(size_t iN);

	CChunk* AppendNew(const char* sName, CChunk::eTypes eType);

protected:
	char m_sHeader[17];
	long m_iVersion;

	long m_iUnknown1, m_iUnknown2, m_iUnknown3, m_iUnknown4;

	std::vector<CChunk*> m_vChunks;
};

#endif // #ifndef _CHUNKY_FILE_H_

