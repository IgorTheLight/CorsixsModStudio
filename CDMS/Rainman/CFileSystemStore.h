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

// #define EXTEND_FILESTORE_WITH_TRAVERSE

#ifndef _C_FILESYSTEM_STORE_H_
#define _C_FILESYSTEM_STORE_H_

#include "gnuc_defines.h"

#define EXTEND_FILESTORE_WITH_TRAVERSE

#include "IFileStore.h"
#ifdef EXTEND_FILESTORE_WITH_TRAVERSE
#include "IDirectoryTraverser.h"
#ifdef RAINMAN_GNUC
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#else
#include <windows.h>
#include <shlwapi.h>
#endif
#endif
#include "Api.h"
#include <stdio.h>
#include <memory.h>

class RAINMAN_API CFileSystemStore : public IFileStore
#ifdef EXTEND_FILESTORE_WITH_TRAVERSE
	, public IDirectoryTraverser
#endif
{
public:
	CFileSystemStore(void);
	virtual ~CFileSystemStore(void);

	class RAINMAN_API CStream : public IFileStore::IStream
	{
	protected:
		friend class CFileSystemStore;
		CStream(void);
		FILE *m_fFile;
#ifdef _DEBUG
		char* m_sFile;
#endif
	public:
		virtual ~CStream(void);

		virtual void VRead(unsigned long iItemCount, unsigned long iItemSize, void* pDestination);
		virtual void VSeek(long iPosition, IFileStore::IStream::SeekLocation SeekFrom = SL_Current);
		virtual long VTell();
	};

	class RAINMAN_API COutputStream : public IFileStore::IOutputStream
	{
	protected:
		friend class CFileSystemStore;
		COutputStream(void);
		FILE *m_fFile;

	public:
		virtual ~COutputStream(void);
		virtual void VWrite(unsigned long iItemCount, unsigned long iItemSize, const void* pSource);
		virtual void VRead(unsigned long iItemCount, unsigned long iItemSize, void* pDestination) ;
		virtual void VSeek(long iPosition, SeekLocation SeekFrom = SL_Current);
		virtual long VTell();
	};

	virtual void VInit(void* pUnused = 0);
	virtual IStream* VOpenStream(const char* sFile);
	IStream* OpenStreamW(const wchar_t* sFile);
	virtual IOutputStream* VOpenOutputStream(const char* sFile, bool bEraseIfPresent);
	IOutputStream* OpenOutputStreamW(const wchar_t* sFile, bool bEraseIfPresent);

#ifdef EXTEND_FILESTORE_WITH_TRAVERSE
	class RAINMAN_API CIterator : public IDirectoryTraverser::IIterator
	{
	protected:
		friend class CFileSystemStore;
		CIterator(const char* sFolder,const CFileSystemStore *pStore);

		#ifdef RAINMAN_GNUC
		DIR* m_pDirectory;
		dirent* m_pDirEnt;
		#else
		WIN32_FIND_DATAA m_W32FD;
		HANDLE m_HandFD;
		#endif
		char* m_sParentPath;
		char* m_sFullPath;
		const CFileSystemStore* m_pStore;

	public:
		~CIterator();

		virtual IDirectoryTraverser::IIterator::eTypes VGetType();
		virtual IDirectoryTraverser::IIterator* VOpenSubDir();
		virtual IFileStore::IStream* VOpenFile();
		virtual const char* VGetName();
		virtual const char* VGetFullPath();
		virtual const char* VGetDirectoryPath();
		virtual tLastWriteTime VGetLastWriteTime();
		virtual IDirectoryTraverser::IIterator::eErrors VNextItem();
	};

	class RAINMAN_API CIteratorW : public IDirectoryTraverser::IIterator
	{
	protected:
		friend class CFileSystemStore;
		CIteratorW(const wchar_t* sFolder,const CFileSystemStore *pStore);

		WIN32_FIND_DATAW m_W32FD;
		HANDLE m_HandFD;
		wchar_t* m_wParentPath;
		char* m_sParentPath;
		wchar_t* m_wFullPath;
		char* m_sFullPath;
		char* m_sFileName;
		const CFileSystemStore* m_pStore;

	public:
		~CIteratorW();
		static void _ensureAsciiVersionOf(const wchar_t* wString, char* &sString);

		virtual IDirectoryTraverser::IIterator::eTypes VGetType();
		virtual IDirectoryTraverser::IIterator* VOpenSubDir();
		virtual IFileStore::IStream* VOpenFile();
		virtual const char* VGetName();
		virtual const char* VGetFullPath();
		virtual const char* VGetDirectoryPath();

		const wchar_t* GetNameW();
		const wchar_t* GetFullPathW();
		const wchar_t* GetDirectoryPathW();

		virtual tLastWriteTime VGetLastWriteTime();
		virtual IDirectoryTraverser::IIterator::eErrors VNextItem();
	};

	virtual IDirectoryTraverser::IIterator* VIterate(const char* sPath);
	virtual CIteratorW* IterateW(const wchar_t* sPath);

	virtual unsigned long VGetEntryPointCount();
	virtual const char* VGetEntryPoint(unsigned long iID);
	virtual void VCreateFolderIn(const char* sPath, const char* sNewFolderName);
	tLastWriteTime VGetLastWriteTime(const char* sPath);
#endif

protected:
	bool m_bInited;
#ifdef EXTEND_FILESTORE_WITH_TRAVERSE
	bool m_bDrivePresent[26];
	char m_sDriveStrings[26][3];
#endif
};

#endif

