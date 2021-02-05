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

#include "CFileSystemStore.h"
#ifndef RAINMAN_GNUC
#include <direct.h>
#endif
#include <string.h>
#include <errno.h>
#include "Exception.h"
#include "Internal_Util.h"
#include "memdebug.h"

CFileSystemStore::CFileSystemStore(void)
{
#ifdef EXTEND_FILESTORE_WITH_TRAVERSE
	for(int i = 0; i < 26; ++i) m_bDrivePresent[i] = false;
#endif
}

CFileSystemStore::~CFileSystemStore(void)
{
}

CFileSystemStore::CStream::CStream(void)
{
	m_fFile = 0;
	#ifdef _DEBUG
	m_sFile = 0;
	#endif
}

CFileSystemStore::CStream::~CStream(void)
{
	if(m_fFile) fclose(m_fFile);
	#ifdef _DEBUG
	if(m_sFile) delete[] m_sFile;
	#endif
}

void CFileSystemStore::VInit(void* pUnused)
{
	m_bInited = true;
}

IFileStore::IStream* CFileSystemStore::VOpenStream(const char* sFile)
{
	FILE* fFile = fopen(sFile, "rb");
	if(fFile == 0)
	{
		throw new CRainmanException(0, __FILE__, __LINE__, "Could not open \'%s\'", sFile);
	}

	CStream *pStream = new CStream();
	if(pStream == 0)
	{
		fclose(fFile);
		throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
	}

	pStream->m_fFile = fFile;
	#ifdef _DEBUG
	pStream->m_sFile = new char[strlen(sFile) + 1];
	strcpy(pStream->m_sFile, sFile);
	#endif

	return pStream;
}

IFileStore::IStream* CFileSystemStore::OpenStreamW(const wchar_t* sFile)
{
	FILE* fFile = _wfopen(sFile, L"rb");
	if(fFile == 0)
	{
		throw new CRainmanException(0, __FILE__, __LINE__, "Could not open \'%S\'", sFile);
	}

	CStream *pStream = new CStream();
	if(pStream == 0)
	{
		fclose(fFile);
		throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
	}

	pStream->m_fFile = fFile;

	return pStream;
}

#ifdef RAINMAN_MSVC
bool TryMakeDirectory(char* sPath)
{
	bool bReturn = false;
	char* sSep = strrchr(sPath, '\\');
	if(sSep && strlen(sPath) > 3)
	{
		*sSep = 0;
		DWORD ret = GetFileAttributesA(sPath);
		bool bIsDir = (ret != (DWORD)-1) && (ret & FILE_ATTRIBUTE_DIRECTORY);
		if(!bIsDir)
		{
			TryMakeDirectory(sPath);
			if(_mkdir(sPath) == 0)
				bReturn = true;
		}
		*sSep = '\\';
	}
	return bReturn;
}

bool TryMakeDirectoryW(wchar_t* sPath)
{
	bool bReturn = false;
	wchar_t* sSep = wcsrchr(sPath, '\\');
	if(sSep && wcslen(sPath) > 3)
	{
		*sSep = 0;
		DWORD ret = GetFileAttributesW(sPath);
		bool bIsDir = (ret != (DWORD)-1) && (ret & FILE_ATTRIBUTE_DIRECTORY);
		if(!bIsDir)
		{
			TryMakeDirectoryW(sPath);
			if(_wmkdir(sPath) == 0)
				bReturn = true;
		}
		*sSep = '\\';
	}
	return bReturn;
}
#endif

IFileStore::IOutputStream* CFileSystemStore::OpenOutputStreamW(const wchar_t* sFile, bool bEraseIfPresent)
{
	FILE* fFile = 0;
	if(!bEraseIfPresent)
		fFile = _wfopen(sFile, L"r+b");
	if(fFile == 0)
		fFile = _wfopen(sFile, L"w+b");
	if(fFile == 0)
	{
		// Maybe folder doesn't exist?
#ifdef RAINMAN_MSVC
		wchar_t* sFile2 = wcsdup(sFile);
		if(TryMakeDirectoryW(sFile2))
		{
			if(!bEraseIfPresent)
				fFile = _wfopen(sFile, L"r+b");
			if(fFile == 0)
				fFile = _wfopen(sFile, L"w+b");
		}
		free(sFile2);
#endif
	}
	if(fFile == 0)
		throw new CRainmanException(0, __FILE__, __LINE__, "Could not open \'%S\'", sFile);
	COutputStream *pStream = new COutputStream();
	if(pStream == 0)
	{
		fclose(fFile);
		throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
	}

	pStream->m_fFile = fFile;
	return pStream;
}

IFileStore::IOutputStream* CFileSystemStore::VOpenOutputStream(const char* sFile, bool bEraseIfPresent)
{
	FILE* fFile = 0;
	if(!bEraseIfPresent)
		fFile = fopen(sFile, "r+b");
	if(fFile == 0)
		fFile = fopen(sFile, "w+b");
	if(fFile == 0)
	{
		// Maybe folder doesn't exist?
#ifdef RAINMAN_MSVC
		char* sFile2 = strdup(sFile);
		if(TryMakeDirectory(sFile2))
		{
			if(!bEraseIfPresent)
				fFile = fopen(sFile, "r+b");
			if(fFile == 0)
				fFile = fopen(sFile, "w+b");
		}
		free(sFile2);
#endif
	}
	if(fFile == 0)
		throw new CRainmanException(0, __FILE__, __LINE__, "Could not open \'%s\' for writing", sFile);
	COutputStream *pStream = new COutputStream();
	if(pStream == 0)
	{
		fclose(fFile);
		throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
	}

	pStream->m_fFile = fFile;
	return pStream;
}

CFileSystemStore::COutputStream::COutputStream(void)
{
	m_fFile = 0;
}

CFileSystemStore::COutputStream::~COutputStream(void)
{
	if(m_fFile) fclose(m_fFile);
}

void CFileSystemStore::COutputStream::VWrite(unsigned long iItemCount, unsigned long iItemSize, const void* pSource)
{
	if(m_fFile == 0) throw new CRainmanException(__FILE__, __LINE__, "No file associated with stream");

	long iOldPos = ftell(m_fFile);
	size_t iItemsWritten = fwrite(pSource, iItemSize, iItemCount, m_fFile);

	if(iItemsWritten == iItemCount)
	{
		//return true;
	}
	else
	{
		fseek(m_fFile, iOldPos, SEEK_SET);
		throw new CRainmanException(__FILE__, __LINE__, "Failed to write data");
	}
}

void CFileSystemStore::COutputStream::VRead(unsigned long iItemCount, unsigned long iItemSize, void* pDestination)
{
	if(m_fFile == 0) throw new CRainmanException(__FILE__, __LINE__, "No file associated with stream");
	unsigned long iByteCount = iItemCount * iItemSize;

	unsigned char* pTempDest = new unsigned char[iByteCount];
	if(pTempDest == 0) throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");

	long iOldPos = ftell(m_fFile);
	size_t iItemsRead = fread(pTempDest, iItemSize, iItemCount, m_fFile);

	if(iItemsRead == iItemCount)
	{
		memcpy(pDestination, pTempDest, iByteCount);
		delete[] pTempDest;
	}
	else
	{
		fseek(m_fFile, iOldPos, SEEK_SET);
		delete[] pTempDest;
		throw new CRainmanException(__FILE__, __LINE__, "Failed to read data");
	}
}

void CFileSystemStore::COutputStream::VSeek(long iPosition, IFileStore::IOutputStream::SeekLocation SeekFrom)
{
	if(m_fFile == 0) throw new CRainmanException(__FILE__, __LINE__, "No file associated with stream");

	switch(SeekFrom)
	{
	case IFileStore::IStream::SL_Current:
		if(fseek(m_fFile, iPosition, SEEK_CUR) != 0) throw new CRainmanException(__FILE__, __LINE__, "Seek failed");
		break;

	case IFileStore::IStream::SL_End:
		if(fseek(m_fFile, iPosition, SEEK_END) != 0) throw new CRainmanException(__FILE__, __LINE__, "Seek failed");
		break;

	case IFileStore::IStream::SL_Root:
		if(fseek(m_fFile, iPosition, SEEK_SET) != 0) throw new CRainmanException(__FILE__, __LINE__, "Seek failed");
		break;

	default:
		throw new CRainmanException(__FILE__, __LINE__, "Invalid SeekFrom argument");
	};
}

long CFileSystemStore::COutputStream::VTell()
{
	if(m_fFile == 0) throw new CRainmanException(__FILE__, __LINE__, "No file associated with stream");
	return ftell(m_fFile);
}

void CFileSystemStore::CStream::VRead(unsigned long iItemCount, unsigned long iItemSize, void* pDestination)
{
	if(m_fFile == 0) throw new CRainmanException(__FILE__, __LINE__, "No file associated with stream");
	unsigned long iByteCount = iItemCount * iItemSize;

	unsigned char* pTempDest = new unsigned char[iByteCount];
	if(pTempDest == 0) throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");

	long iOldPos = ftell(m_fFile);
	size_t iItemsRead = fread(pTempDest, iItemSize, iItemCount, m_fFile);

	if(iItemsRead == iItemCount)
	{
		memcpy(pDestination, pTempDest, iByteCount);
		delete[] pTempDest;
		// return true;
	}
	else
	{
		fseek(m_fFile, iOldPos, SEEK_SET);
		delete[] pTempDest;
		throw new CRainmanException(__FILE__, __LINE__, "Failed to read items");
	}
}

void CFileSystemStore::CStream::VSeek(long iPosition, IFileStore::IStream::SeekLocation SeekFrom)
{
	if(m_fFile == 0) throw new CRainmanException(__FILE__, __LINE__, "No file associated with stream");

	switch(SeekFrom)
	{
	case IFileStore::IStream::SL_Current:
		if(fseek(m_fFile, iPosition, SEEK_CUR) != 0) throw new CRainmanException(__FILE__, __LINE__, "Seek failed");
		break;

	case IFileStore::IStream::SL_End:
		if(fseek(m_fFile, iPosition, SEEK_END) != 0) throw new CRainmanException(__FILE__, __LINE__, "Seek failed");
		break;

	case IFileStore::IStream::SL_Root:
		if(fseek(m_fFile, iPosition, SEEK_SET) != 0) throw new CRainmanException(__FILE__, __LINE__, "Seek failed");
		break;

	default:
		throw new CRainmanException(__FILE__, __LINE__, "Invalid SeekFrom argument");
	};
}

long CFileSystemStore::CStream::VTell()
{
	if(m_fFile == 0) throw new CRainmanException(__FILE__, __LINE__, "No file associated with stream");
	return ftell(m_fFile);
}

static char* mystrdup(const char* sStr)
{
	char* s = new char[strlen(sStr) + 1];
	if(s == 0) return 0;
	strcpy(s, sStr);
	return s;
}

static wchar_t* mystrdup(const wchar_t* sStr)
{
	wchar_t* s = new wchar_t[wcslen(sStr) + 1];
	if(s == 0) return 0;
	wcscpy(s, sStr);
	return s;
}

#ifdef EXTEND_FILESTORE_WITH_TRAVERSE
tLastWriteTime CFileSystemStore::VGetLastWriteTime(const char* sPath)
{
	return GetLastWriteTime(sPath);
}

void CFileSystemStore::VCreateFolderIn(const char* sPath, const char* sNewFolderName)
{
	char* sNewPath = new char[strlen(sPath) + strlen(sNewFolderName) + 2];
	strcpy(sNewPath, sPath);
	strcat(sNewPath, "\\");
	strcat(sNewPath, sNewFolderName);
	bool bRet = (_mkdir(sNewPath) == 0);
	if(!bRet)
	{
#ifdef WIN32
		if(errno == EEXIST)
		{
			if(PathIsDirectory(sNewPath))
			{
				delete[] sNewPath;
				return;
			}
		}
#endif
		delete[] sNewPath;
		throw new CRainmanException(0, __FILE__, __LINE__, "Could not create \'%s\'", sPath);
	}
	delete[] sNewPath;
}

CFileSystemStore::CIteratorW::CIteratorW(const wchar_t* sFolder,const CFileSystemStore *pStore)
{
	m_HandFD = 0;
	m_wParentPath = mystrdup(sFolder);
	m_wFullPath = 0;
	m_pStore = pStore;
	m_sParentPath = 0;
	m_sFullPath = 0;
	m_sFileName = 0;

	wchar_t* sTmp;
	sTmp = new wchar_t[wcslen(sFolder) + 3];
	if(sTmp == 0)
	{
		delete[] m_wParentPath;
		throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
	}
	wcscpy(sTmp, sFolder);
	wcscat(sTmp, L"\\*");

	m_HandFD = FindFirstFileW(sTmp, &m_W32FD);
	delete[] sTmp;
	if(m_HandFD == INVALID_HANDLE_VALUE)
	{
		m_HandFD = 0;
		delete[] m_wParentPath;
		throw new CRainmanException(0, __FILE__, __LINE__, "FindFirstFile failed (%S)", sFolder);
	}
	while(wcscmp(m_W32FD.cFileName,L".") == 0 || wcscmp(m_W32FD.cFileName,L"..") == 0)
	{
		if(!FindNextFileW(m_HandFD, &m_W32FD))
		{
			FindClose(m_HandFD);
			m_HandFD = 0;
			return; // no exception; an empty folder is not an error
		}
	}

	m_wFullPath = new wchar_t[wcslen(m_wParentPath) + wcslen(m_W32FD.cFileName) + 2];
	if(m_wFullPath == 0)
	{
		delete[] m_wParentPath;
		FindClose(m_HandFD);
		m_HandFD = 0;
		throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
	}
	wcscpy(m_wFullPath, m_wParentPath);
	wcscat(m_wFullPath, L"\\");
	wcscat(m_wFullPath, m_W32FD.cFileName);
}

CFileSystemStore::CIterator::CIterator(const char* sFolder,const CFileSystemStore *pStore)
{
	#ifdef RAINMAN_GNUC

	m_pDirectory = 0;
	m_pDirEnt = 0;
	m_sParentPath = new char[strlen(sFolder) + 2];
	strcpy(m_sParentPath, sFolder);
	Util_EnsureEndsWith(m_sParentPath, '/');
	m_sFullPath = 0;
	m_pStore = pStore;

	if( (m_pDirectory = opendir(m_sParentPath)) == 0)
	{
		delete[] m_sParentPath;
		throw new CRainmanException(0, __FILE__, __LINE__, "opendir failed (%s)", sFolder);
	}

	while ((m_pDirEnt = readdir(m_pDirectory)) != 0)
	{
		if( (!strcmp(m_pDirEnt->d_name, ".")) ||
			(!strcmp(m_pDirEnt->d_name, "..")) )
		{
			continue;
        }
		break;
	}

	if(m_pDirEnt != 0)
	{
		m_sFullPath = new char[strlen(m_sParentPath) + strlen(m_pDirEnt->d_name) + 1];
		sprintf(m_sFullPath, "%s%s", m_sParentPath, m_pDirEnt->d_name);
	}

	#else
	m_HandFD = 0;
	m_sParentPath = mystrdup(sFolder);
	m_sFullPath = 0;
	m_pStore = pStore;

	char* sTmp;
	sTmp = new char[strlen(sFolder) + 3];
	if(sTmp == 0)
	{
		delete[] m_sParentPath;
		throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
	}
	strcpy(sTmp, sFolder);
	strcat(sTmp, "\\*");

	m_HandFD = FindFirstFileA(sTmp, &m_W32FD);
	delete[] sTmp;
	if(m_HandFD == INVALID_HANDLE_VALUE)
	{
		m_HandFD = 0;
		delete[] m_sParentPath;
		throw new CRainmanException(0, __FILE__, __LINE__, "FindFirstFile failed (%s)", sFolder);
	}
	while(strcmp(m_W32FD.cFileName,".") == 0 || strcmp(m_W32FD.cFileName,"..") == 0)
	{
		if(!FindNextFileA(m_HandFD, &m_W32FD))
		{
			FindClose(m_HandFD);
			m_HandFD = 0;
			return; // no exception; an empty folder is not an error
		}
	}

	m_sFullPath = new char[strlen(m_sParentPath) + strlen(m_W32FD.cFileName) + 2];
	if(m_sFullPath == 0)
	{
		delete[] m_sParentPath;
		FindClose(m_HandFD);
		m_HandFD = 0;
		throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
	}
	strcpy(m_sFullPath, m_sParentPath);
	strcat(m_sFullPath, "\\");
	strcat(m_sFullPath, m_W32FD.cFileName);
	#endif
}

CFileSystemStore::CIteratorW::~CIteratorW()
{
	delete[] m_sParentPath;
	delete[] m_sFullPath;
	delete[] m_sFileName;
	delete[] m_wParentPath;
	delete[] m_wFullPath;
	if(m_HandFD) FindClose(m_HandFD);
}

CFileSystemStore::CIterator::~CIterator()
{
	delete[] m_sParentPath;
	delete[] m_sFullPath;
	#ifdef RAINMAN_GNUC
	if(m_pDirectory) closedir(m_pDirectory);
	#else
	if(m_HandFD) FindClose(m_HandFD);
	#endif
}

IDirectoryTraverser::IIterator::eTypes CFileSystemStore::CIteratorW::VGetType()
{
	if(m_HandFD == 0) return IDirectoryTraverser::IIterator::T_Nothing;
	return (m_W32FD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? IDirectoryTraverser::IIterator::T_Directory : IDirectoryTraverser::IIterator::T_File);
}

IDirectoryTraverser::IIterator::eTypes CFileSystemStore::CIterator::VGetType()
{
	#ifdef RAINMAN_GNUC
	if(m_pDirEnt == 0) return IDirectoryTraverser::IIterator::T_Nothing;
	return (m_pDirEnt->d_type == DT_DIR) ? IDirectoryTraverser::IIterator::T_Directory : IDirectoryTraverser::IIterator::T_File;
	#else
	if(m_HandFD == 0) return IDirectoryTraverser::IIterator::T_Nothing;
	return (m_W32FD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? IDirectoryTraverser::IIterator::T_Directory : IDirectoryTraverser::IIterator::T_File);
	#endif
}

IDirectoryTraverser::IIterator* CFileSystemStore::CIteratorW::VOpenSubDir()
{
	if(m_HandFD == 0) throw new CRainmanException(__FILE__, __LINE__, "Nothing to open");
	if(m_W32FD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) return new CIteratorW(m_wFullPath, m_pStore);
	throw new CRainmanException(__FILE__, __LINE__, "Cannot iterate something which is not a folder");
}

IDirectoryTraverser::IIterator* CFileSystemStore::CIterator::VOpenSubDir()
{
	#ifdef RAINMAN_GNUC
	if(m_pDirEnt == 0) throw new CRainmanException(__FILE__, __LINE__, "Nothing to open");
	if(m_pDirEnt->d_type == DT_DIR) return new CIterator(m_sFullPath, m_pStore);
	throw new CRainmanException(__FILE__, __LINE__, "Cannot iterate something which is not a folder");
	#else
	if(m_HandFD == 0) throw new CRainmanException(__FILE__, __LINE__, "Nothing to open");
	if(m_W32FD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) return new CIterator(m_sFullPath, m_pStore);
	throw new CRainmanException(__FILE__, __LINE__, "Cannot iterate something which is not a folder");
	#endif
}

IFileStore::IStream* CFileSystemStore::CIteratorW::VOpenFile()
{
	if(m_HandFD == 0 || m_pStore == 0) throw new CRainmanException(__FILE__, __LINE__, "Handle or store invalid");
	if(m_W32FD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) throw new CRainmanException(__FILE__, __LINE__, "Cannot open a folder");
	return ((CFileSystemStore*)m_pStore)->OpenStreamW(m_wFullPath);
}

IFileStore::IStream* CFileSystemStore::CIterator::VOpenFile()
{
	#ifdef RAINMAN_GNUC
	if(m_pDirEnt == 0 || m_pStore == 0) throw new CRainmanException(__FILE__, __LINE__, "Handle or store invalid");
	return ((CFileSystemStore*)m_pStore)->VOpenStream(m_sFullPath);
	#else
	if(m_HandFD == 0 || m_pStore == 0) throw new CRainmanException(__FILE__, __LINE__, "Handle or store invalid");
	if(m_W32FD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) throw new CRainmanException(__FILE__, __LINE__, "Cannot open a folder");
	return ((CFileSystemStore*)m_pStore)->VOpenStream(m_sFullPath);
	#endif
}

void CFileSystemStore::CIteratorW::_ensureAsciiVersionOf(const wchar_t* wString, char* &sString)
{
	if(sString == 0)
	{
		sString = new char[wcslen(wString) + 1];
		int i = -1;
		do
		{
			++i;
			sString[i] = (char)wString[i];
		} while(wString[i]);
	}
}

const char* CFileSystemStore::CIteratorW::VGetName()
{
	if(m_HandFD == 0) throw new CRainmanException(__FILE__, __LINE__, "Invalid handle");
	_ensureAsciiVersionOf(m_W32FD.cFileName, m_sFileName);
	return m_sFileName;
}

const char* CFileSystemStore::CIterator::VGetName()
{
	#ifdef RAINMAN_GNUC
	if(m_pDirEnt == 0) throw new CRainmanException(__FILE__, __LINE__, "Invalid handle");
	return m_pDirEnt->d_name;
	#else
	if(m_HandFD == 0) throw new CRainmanException(__FILE__, __LINE__, "Invalid handle");
	return m_W32FD.cFileName;
	#endif
}

const char* CFileSystemStore::CIteratorW::VGetFullPath()
{
	_ensureAsciiVersionOf(m_wFullPath, m_sFullPath);
	return m_sFullPath;
}

const char* CFileSystemStore::CIterator::VGetFullPath()
{
	return m_sFullPath;
}

const char* CFileSystemStore::CIteratorW::VGetDirectoryPath()
{
	_ensureAsciiVersionOf(m_wParentPath, m_sParentPath);
	return m_sParentPath;
}

const char* CFileSystemStore::CIterator::VGetDirectoryPath()
{
	return m_sParentPath;
}

tLastWriteTime CFileSystemStore::CIterator::VGetLastWriteTime()
{
	#ifdef RAINMAN_GNUC
	if(m_pDirEnt == 0 || m_pStore == 0) throw new CRainmanException(__FILE__, __LINE__, "Handle or store invalid");
	return ((CFileSystemStore*)m_pStore)->VGetLastWriteTime(m_sFullPath);
	#else
	if(m_HandFD == 0) throw new CRainmanException(__FILE__, __LINE__, "Invalid handle");
	tLastWriteTime oRet = m_W32FD.ftLastWriteTime.dwHighDateTime;
	oRet <<= 32;
	oRet += m_W32FD.ftLastWriteTime.dwLowDateTime;
	oRet /= 10;
	oRet -= 0x2B6109100;
	return oRet;
	#endif
}

tLastWriteTime CFileSystemStore::CIteratorW::VGetLastWriteTime()
{
	if(m_HandFD == 0) throw new CRainmanException(__FILE__, __LINE__, "Invalid handle");
	tLastWriteTime oRet = m_W32FD.ftLastWriteTime.dwHighDateTime;
	oRet <<= 32;
	oRet += m_W32FD.ftLastWriteTime.dwLowDateTime;
	oRet /= 10;
	oRet -= 0x2B6109100;
	return oRet;
}

IDirectoryTraverser::IIterator::eErrors CFileSystemStore::CIterator::VNextItem()
{
	#ifdef RAINMAN_GNUC
	if(m_pDirEnt == 0) throw new CRainmanException(__FILE__, __LINE__, "Invalid handle");

	if ((m_pDirEnt = readdir(m_pDirectory)) != 0)
	{
		m_sFullPath = new char[strlen(m_sParentPath) + strlen(m_pDirEnt->d_name) + 1];
		sprintf(m_sFullPath, "%s%s", m_sParentPath, m_pDirEnt->d_name);
	}
	else
	{
		closedir(m_pDirectory);
		m_pDirectory = 0;
		return E_AtEnd;
	}

	#else
	if(m_HandFD == 0) throw new CRainmanException(__FILE__, __LINE__, "Invalid handle");
	if(FindNextFileA(m_HandFD, &m_W32FD) == TRUE)
	{
		delete[] m_sFullPath;
		m_sFullPath = new char[strlen(m_sParentPath) + strlen(m_W32FD.cFileName) + 2];
		if(m_sFullPath == 0)
		{
			FindClose(m_HandFD);
			m_HandFD = 0;
			throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
		}
		strcpy(m_sFullPath, m_sParentPath);
		strcat(m_sFullPath, "\\");
		strcat(m_sFullPath, m_W32FD.cFileName);

		return E_OK;
	}
	else
	{
		if(GetLastError() == ERROR_NO_MORE_FILES)
		{
			return E_AtEnd;
		}
		else
		{
			FindClose(m_HandFD);
			m_HandFD = 0;
			throw new CRainmanException(__FILE__, __LINE__, "Unknown error");
		}
	}
	#endif
}

IDirectoryTraverser::IIterator::eErrors CFileSystemStore::CIteratorW::VNextItem()
{
	if(m_HandFD == 0) throw new CRainmanException(__FILE__, __LINE__, "Invalid handle");
	if(FindNextFileW(m_HandFD, &m_W32FD) == TRUE)
	{
		delete[] m_wFullPath;
		m_wFullPath = new wchar_t[wcslen(m_wParentPath) + wcslen(m_W32FD.cFileName) + 2];
		if(m_sFullPath == 0)
		{
			FindClose(m_HandFD);
			m_HandFD = 0;
			throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
		}
		wcscpy(m_wFullPath, m_wParentPath);
		wcscat(m_wFullPath, L"\\");
		wcscat(m_wFullPath, m_W32FD.cFileName);

		delete[] m_sFullPath; m_sFullPath = 0;
		delete[] m_sFileName; m_sFileName = 0;

		return E_OK;
	}
	else
	{
		if(GetLastError() == ERROR_NO_MORE_FILES)
		{
			return E_AtEnd;
		}
		else
		{
			FindClose(m_HandFD);
			m_HandFD = 0;
			throw new CRainmanException(__FILE__, __LINE__, "Unknown error");
		}
	}
}

IDirectoryTraverser::IIterator* CFileSystemStore::VIterate(const char* sPath)
{
	return new CIterator(sPath, this);
}

CFileSystemStore::CIteratorW* CFileSystemStore::IterateW(const wchar_t* sPath)
{
	return new CIteratorW(sPath, this);
}

unsigned long CFileSystemStore::VGetEntryPointCount()
{
	#ifdef RAINMAN_GNUC
	m_sDriveStrings[0][0] = '/';
	m_sDriveStrings[0][1] = 0;
	return 1;
	#else
	unsigned long iCount = 0;
	int curdrive = _getdrive();
	for(int i = 1; i <= 26; ++i)
	{
		if(_chdrive(i) == 0)
		{
			m_sDriveStrings[iCount][0] = i + 'A' - 1;
			m_sDriveStrings[iCount][1] = ':';
			m_sDriveStrings[iCount][2] = 0;
			++iCount;
		}
	}
	_chdrive(curdrive);
	return iCount;
	#endif
}

const char* CFileSystemStore::VGetEntryPoint(unsigned long iID)
{
	return m_sDriveStrings[iID];
}
#endif

