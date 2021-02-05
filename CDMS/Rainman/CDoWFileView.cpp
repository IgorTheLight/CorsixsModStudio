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

#include "CDoWFileView.h"
#include <algorithm>
#include "memdebug.h"
#include "Exception.h"

CDoWFileView::CDoWFileView(void)
{
	_Clean();
	m_RootFolder.sFullName = ""; // dont worry - it wont try and delete it
	m_RootFolder.sName = "";
}

void CDoWFileView::Reset()
{
	_Clean();
}

CDoWFileView::~CDoWFileView(void)
{
	_Clean();
}

void CDoWFileView::VInit(void* pUnused)
{
	m_bInited = true;
}

CDoWFileView::_VirtFile* CDoWFileView::_FindFile(const char* sPath, CDoWFileView::_VirtFolder** ipFolder)
{
	_VirtFolder* pFolder = &m_RootFolder;
	unsigned long iPartLength;
	const char* sSlashLoc = strchr(sPath, '\\');
	if(ipFolder) *ipFolder = 0;

	size_t iBSL, iBSH, iBSM;
	// Identify Correct Folder
	iPartLength = (unsigned long)(sSlashLoc ? sSlashLoc - sPath : strlen(sPath));
	while(sSlashLoc && iPartLength)
	{
		bool bFound = false;
		iBSL = 0;
		iBSH = pFolder->vChildFolders.size();
		iBSM = (iBSL + iBSH) >> 1;
		while(!bFound && iBSH > iBSL)
		{
			int iNiRes = strnicmp(pFolder->vChildFolders[iBSM]->sName, sPath, iPartLength);
			if(iNiRes > 0)
			{
				iBSH = iBSM;
				iBSM = (iBSL + iBSH) >> 1;
			}
			else if(iNiRes < 0)
			{
				iBSL = iBSM + 1;
				iBSM = (iBSL + iBSH) >> 1;
			}
			else
			{
				if(strlen(pFolder->vChildFolders[iBSM]->sName) != iPartLength)
				{
					iBSH = iBSM;
					iBSM = (iBSL + iBSH) >> 1;
				}
				else
				{
					pFolder = pFolder->vChildFolders[iBSM];
					bFound = true;
				}
			}
		}
		if(!bFound) throw new CRainmanException(0, __FILE__, __LINE__, "Could not find \'%s\'", sPath);

		sPath += iPartLength + (sSlashLoc ? 1 : 0);
		sSlashLoc = strchr(sPath, '\\');
		iPartLength = (unsigned long)(sSlashLoc ? sSlashLoc - sPath : strlen(sPath));
	}
	if(iPartLength == 0) throw new CRainmanException(__FILE__, __LINE__, "No file name");

	// Find file in folder
	if(ipFolder) *ipFolder = pFolder;
	iBSL = 0;
	iBSH = pFolder->vChildFiles.size();
	iBSM = (iBSL + iBSH) >> 1;
	while(iBSH > iBSL)
	{
		int iNiRes = strnicmp(pFolder->vChildFiles[iBSM]->sName, sPath, iPartLength);
		if(iNiRes > 0)
		{
			iBSH = iBSM;
			iBSM = (iBSL + iBSH) >> 1;
		}
		else if(iNiRes < 0)
		{
			iBSL = iBSM + 1;
			iBSM = (iBSL + iBSH) >> 1;
		}
		else
		{
			if(strlen(pFolder->vChildFiles[iBSM]->sName) != iPartLength)
			{
				iBSH = iBSM;
				iBSM = (iBSL + iBSH) >> 1;
			}
			else
			{
				return pFolder->vChildFiles[iBSM];
			}
		}
	}
	/*
	for(std::vector<_VirtFile*>::iterator itr = pFolder->vChildFiles.begin(); itr != pFolder->vChildFiles.end(); ++itr)
	{
		if(strlen((**itr).sName) == iPartLength && strnicmp((**itr).sName, sPath, iPartLength) == 0)
		{
			return *itr;
		}
	}
	*/
	throw new CRainmanException(0, __FILE__, __LINE__, "Could not find \'%s\'", sPath);
}

void CDoWFileView::VCreateFolderIn(const char* sPath, const char* sNewFolderName)
{
	//! Todo
	throw new CRainmanException(__FILE__, __LINE__, "TODO: Need to implement this :D");
}

bool CDoWFileView::_SortFolds(CDoWFileView::_VirtFolder* a, CDoWFileView::_VirtFolder* b)
{
	return (stricmp(a->sName, b->sName) < 0);
}

bool CDoWFileView::_SortFiles(CDoWFileView::_VirtFile* a, CDoWFileView::_VirtFile* b)
{
	return (stricmp(a->sName, b->sName) < 0);
}

void CDoWFileView::_EnsureOutputFolder(_VirtFolder* pFolder, unsigned long *pSourceID)
{
	for(std::map<unsigned long, char*>::iterator itr = pFolder->mapSourceFolderNames.begin(); itr != pFolder->mapSourceFolderNames.end(); ++itr)
	{
		if(m_vSourceFlags[itr->first].second)
		{
			*pSourceID = itr->first;
			return;
		}
	}
	if(pFolder->pParent)
	{
		try
		{
			_EnsureOutputFolder(pFolder->pParent, pSourceID);
			m_vSourceDirItrs[*pSourceID]->VCreateFolderIn(pFolder->pParent->mapSourceFolderNames[*pSourceID],pFolder->sName);
		}
		catch(CRainmanException* pE)
		{
			throw new CRainmanException(pE, __FILE__, __LINE__, "Cannot ensure output folder (%s)", pFolder->sFullName);
		}
		char* sNewName = (char*)malloc(strlen(pFolder->pParent->mapSourceFolderNames[*pSourceID]) + strlen(pFolder->sName) + 2);
		strcpy(sNewName, pFolder->pParent->mapSourceFolderNames[*pSourceID]);
		strcat(sNewName, "\\");
		strcat(sNewName, pFolder->sName);
		pFolder->mapSourceFolderNames[*pSourceID] = sNewName;
		return;
	}
	throw new CRainmanException(0, __FILE__, __LINE__, "Cannot ensure output folder (%s)", pFolder->sFullName);
}

IFileStore::IOutputStream* CDoWFileView::VOpenOutputStream(const char* sFile, bool bEraseIfPresent)
{
	_VirtFolder* pFolder;
	_VirtFile* pFile;
	try
	{
		pFile = _FindFile(sFile, &pFolder);
	}
	catch(CRainmanException *pE)
	{
		pFile = 0;
		if(pFolder == 0)
		{
			throw new CRainmanException(pE, __FILE__, __LINE__, "Failed to file \'%s\'", sFile);
		}
		pE->destroy();
	}
	if(pFile)
	{
		sFile = pFile->sName;
		if(m_vSourceFlags[pFile->iSourceID].first || m_vSourceFlags[pFile->iSourceID].second)
		{
			char *sStoragePath = new char[strlen(sFile) + 2 + strlen(pFolder->mapSourceFolderNames[pFile->iSourceID])];
			if(sStoragePath == 0) throw new CRainmanException(__FILE__, __LINE__, "Cannot allocate memory");
			strcpy(sStoragePath,pFolder->mapSourceFolderNames[pFile->iSourceID]);
			if(*sStoragePath && sStoragePath[strlen(sStoragePath)-1] != '\\') strcat(sStoragePath, "\\");
			strcat(sStoragePath, sFile);

			try
			{
				IFileStore::IOutputStream *pStream = m_vSourceStores[pFile->iSourceID]->VOpenOutputStream(sStoragePath, bEraseIfPresent);
				delete[] sStoragePath;
				return pStream;
			}
			catch(CRainmanException* pE)
			{
				delete[] sStoragePath;
				throw new CRainmanException(pE, __FILE__, __LINE__, "Cannot open \'%s\'", sFile);
			}
		}
	}
	if(pFolder || pFile)
	{
		if(!pFile)
		{
			sFile = strrchr(sFile, '\\') + 1;
		}
		unsigned long iSource;
		try
		{
			_EnsureOutputFolder(pFolder, &iSource);
		}
		catch(CRainmanException* pE)
		{
			throw new CRainmanException(pE, __FILE__, __LINE__, "Cannot ensure output folder for \'%s\'", sFile);
		}
		if(! (!pFile || (iSource < pFile->iSourceID)) )
		{
			throw new CRainmanException(0, __FILE__, __LINE__, "Problem ensuring output folder for \'%s\'", sFile);
		}
		if(!pFile)
		{
			pFile = new _VirtFile;
			pFile->bInReqMod = false;
			pFile->pParent = pFolder;
			pFile->sName = strdup(sFile);
			pFolder->vChildFiles.push_back(pFile);
			std::sort(pFolder->vChildFiles.begin(), pFolder->vChildFiles.end(), _SortFiles);
		}
		pFile->iSourceID = iSource;
		pFile->iModID = iSource; // it's the same :)
		char *sStoragePath = new char[strlen(sFile) + 2 + strlen(pFolder->mapSourceFolderNames[pFile->iSourceID])];
		if(sStoragePath == 0) throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
		strcpy(sStoragePath,pFolder->mapSourceFolderNames[pFile->iSourceID]);
		if(*sStoragePath && sStoragePath[strlen(sStoragePath)-1] != '\\') strcat(sStoragePath, "\\");
		strcat(sStoragePath, sFile);

		try
		{
			IFileStore::IOutputStream *pStream = m_vSourceStores[pFile->iSourceID]->VOpenOutputStream(sStoragePath, bEraseIfPresent);
			delete[] sStoragePath;
			return pStream;
		}
		catch(CRainmanException* pE)
		{
			delete[] sStoragePath;
			throw new CRainmanException(pE, __FILE__, __LINE__, "Failed to open source store for \'%s\'", sFile);
		}
	}
	throw new CRainmanException(0, __FILE__, __LINE__, "Failed to open \'%s\'", sFile);
}

IFileStore::IStream* CDoWFileView::VOpenStream(const char* sPath)
{
	_VirtFolder* pFolder;
	_VirtFile* pFile;
	try
	{
		pFile = _FindFile(sPath, &pFolder);
	}
	catch(CRainmanException* pE)
	{
		throw new CRainmanException(pE, __FILE__, __LINE__, "Could not find \'%s\'", sPath);
	}
	sPath = pFile->sName;
	char *sStoragePath = new char[strlen(sPath) + 2 + strlen(pFolder->mapSourceFolderNames[pFile->iSourceID])];
	if(sStoragePath == 0) throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
	strcpy(sStoragePath,pFolder->mapSourceFolderNames[pFile->iSourceID]);
	if(*sStoragePath && sStoragePath[strlen(sStoragePath)-1] != '\\') strcat(sStoragePath, "\\");
	strcat(sStoragePath, sPath);

	try
	{
		IFileStore::IStream *pStream = m_vSourceStores[pFile->iSourceID]->VOpenStream(sStoragePath);
		delete[] sStoragePath;
		return pStream;
	}
	catch(CRainmanException* pE)
	{
		throw new CRainmanException(pE, __FILE__, __LINE__, "Could not open source \'%s\'", sPath);
	}
}

CDoWFileView::CIterator::CIterator(_VirtFolder* pFolder, CDoWFileView* pStore)
{
	m_sParentPath = pFolder->sFullName;
	m_sFullPath = 0;
	m_pStore = pStore;
	m_pDirectory = pFolder;
	m_FoldIter = pFolder->vChildFolders.begin();
	m_FileIter = pFolder->vChildFiles.begin();
	m_iWhat = 1; // 1 = folder, 2 = file, 0 = nothing
	if(m_FoldIter == pFolder->vChildFolders.end())
	{
		m_iWhat = 2;
		if(m_FileIter == pFolder->vChildFiles.end())
		{
			m_iWhat = 0;
		}
		else
		{
			m_sFullPath = new char[strlen(m_sParentPath) + 2 + strlen((**m_FileIter).sName)];
			if(m_sFullPath == 0)
			{
				m_iWhat = 0;
				throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
			}
			strcpy(m_sFullPath, m_sParentPath);
			if(*m_sFullPath) strcat(m_sFullPath, "\\");
			strcat(m_sFullPath, (**m_FileIter).sName);
		}
	}
	else
	{
		m_sFullPath = new char[strlen(m_sParentPath) + 2 + strlen((**m_FoldIter).sName)];
		if(m_sFullPath == 0)
		{
			m_iWhat = 0;
			throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
		}
		strcpy(m_sFullPath, m_sParentPath);
		if(*m_sFullPath) strcat(m_sFullPath, "\\");
		strcat(m_sFullPath, (**m_FoldIter).sName);
	}
}

CDoWFileView::CIterator::~CIterator()
{
	if(m_sFullPath) delete[] m_sFullPath;
}

IDirectoryTraverser::IIterator::eTypes CDoWFileView::CIterator::VGetType()
{
	if(m_iWhat == 1) return T_Directory;
	if(m_iWhat == 2) return T_File;
	return T_Nothing;
}

IDirectoryTraverser::IIterator* CDoWFileView::CIterator::VOpenSubDir()
{
	if(m_iWhat != 1) throw new CRainmanException(__FILE__, __LINE__, "Can only open directories");
	CIterator* pItr = new CIterator(*m_FoldIter, m_pStore);
	if(pItr == 0) throw new CRainmanException(__FILE__, __LINE__, "Memory allocate error");
	if(pItr && pItr->m_iWhat == 0)
	{
		delete pItr;
		pItr = 0;
	}
	return pItr;
}

IFileStore::IStream* CDoWFileView::CIterator::VOpenFile()
{
	try
	{
		return m_pStore->VOpenStream(m_sFullPath);
	}
	catch(CRainmanException* pE)
	{
		throw new CRainmanException(pE, __FILE__, __LINE__, "Could not open \'%s\'", m_sFullPath);
	}
}

const char* CDoWFileView::CIterator::VGetName()
{
	if(m_iWhat == 1) return (**m_FoldIter).sName;
	if(m_iWhat == 2) return (**m_FileIter).sName;
	throw new CRainmanException(__FILE__, __LINE__, "No item");
}

const char* CDoWFileView::CIterator::VGetFullPath()
{
	return m_sFullPath;
}

const char* CDoWFileView::CIterator::VGetDirectoryPath()
{
	return m_sParentPath;
}

tLastWriteTime CDoWFileView::VGetLastWriteTime(const char* sPath)
{
	_VirtFolder *pFolder;
	_VirtFile *pFile;
	try
	{
		pFile = _FindFile(sPath, &pFolder);
	}
	catch(CRainmanException* pE)
	{
		throw new CRainmanException(pE, __FILE__, __LINE__, "Cannot find file \'%s\'", sPath);
	}
	if(pFile->bGotWriteTime) return pFile->oWriteTime;

	sPath = pFile->sName;
	char *sStoragePath = new char[strlen(sPath) + 2 + strlen(pFolder->mapSourceFolderNames[pFile->iSourceID])];
	if(sStoragePath == 0) throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
	strcpy(sStoragePath,pFolder->mapSourceFolderNames[pFile->iSourceID]);
	if(*sStoragePath && sStoragePath[strlen(sStoragePath)-1] != '\\') strcat(sStoragePath, "\\");
	strcat(sStoragePath, sPath);

	pFile->bGotWriteTime = true;
	try
	{
		pFile->oWriteTime = m_vSourceDirItrs[pFile->iSourceID]->VGetLastWriteTime(sStoragePath);
	}
	catch(CRainmanException *pE)
	{
		delete[] sStoragePath;
		throw new CRainmanException(pE, __FILE__, __LINE__, "Cannot get source write time for file \'%s\'", sPath);
	}
	delete[] sStoragePath;
	return pFile->oWriteTime;
}

tLastWriteTime CDoWFileView::CIterator::VGetLastWriteTime()
{
	if(m_iWhat == 2)
	{
		_VirtFile *pFile = *m_FileIter;
		if( pFile->bGotWriteTime ) return pFile->oWriteTime;
		
		const char* sPath = pFile->sName;
		char *sStoragePath = new char[strlen(sPath) + 2 + strlen(m_pDirectory->mapSourceFolderNames[pFile->iSourceID])];
		if(sStoragePath == 0) throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
		strcpy(sStoragePath,m_pDirectory->mapSourceFolderNames[pFile->iSourceID]);
		if(*sStoragePath && sStoragePath[strlen(sStoragePath)-1] != '\\') strcat(sStoragePath, "\\");
		strcat(sStoragePath, sPath);

		pFile->bGotWriteTime = true;
		try
		{
			pFile->oWriteTime = m_pStore->m_vSourceDirItrs[pFile->iSourceID]->VGetLastWriteTime(sStoragePath);
		}
		catch(CRainmanException *pE)
		{
			delete[] sStoragePath;
			throw new CRainmanException(pE, __FILE__, __LINE__, "Cannot get source write time for file \'%s\'", sPath);
		}
		delete[] sStoragePath;
		return pFile->oWriteTime;
	}
	throw new CRainmanException(__FILE__, __LINE__, "Can only get write times for files");
}

IDirectoryTraverser::IIterator::eErrors CDoWFileView::CIterator::VNextItem()
{
	if(m_iWhat == 0) throw new CRainmanException(__FILE__, __LINE__, "Nothing here");
	if(m_iWhat == 1)
	{
		if(++m_FoldIter == m_pDirectory->vChildFolders.end())
		{
			m_iWhat = 2;
			if(m_FileIter == m_pDirectory->vChildFiles.end())
			{
				m_iWhat = 0;
				return E_AtEnd;
			}
			goto skip_to_files;
		}
		else
		{
			if(m_sFullPath) delete[] m_sFullPath;
			m_sFullPath = new char[strlen(m_sParentPath) + 2 + strlen((**m_FoldIter).sName)];
			if(m_sFullPath == 0)
			{
				m_iWhat = 0;
				throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
			}
			strcpy(m_sFullPath, m_sParentPath);
			if(*m_sFullPath) strcat(m_sFullPath, "\\");
			strcat(m_sFullPath, (**m_FoldIter).sName);
		}
	}
	if(m_iWhat == 2)
	{
		if(++m_FileIter == m_pDirectory->vChildFiles.end())
		{
			m_iWhat = 0;
			return E_AtEnd;
		}
		else
		{
skip_to_files:
			if(m_sFullPath) delete[] m_sFullPath;
			m_sFullPath = new char[strlen(m_sParentPath) + 2 + strlen((**m_FileIter).sName)];
			if(m_sFullPath == 0)
			{
				m_iWhat = 0;
				throw new CRainmanException(__FILE__, __LINE__, "Failed to allocate memory");
			}
			strcpy(m_sFullPath, m_sParentPath);
			if(*m_sFullPath) strcat(m_sFullPath, "\\");
			strcat(m_sFullPath, (**m_FileIter).sName);
		}
	}

	return E_OK;
}

void* CDoWFileView::CIterator::VGetTag(long iTag) // 0 = ModName | 1 = Source Name
{
	if(m_iWhat != 2) throw new CRainmanException(__FILE__, __LINE__, "Invalid tag");

	switch(iTag)
	{
	case 0:
		return m_pStore->m_vModNames[(**m_FileIter).iModID];
	case 1:
		return m_pStore->m_vSourceNames[(**m_FileIter).iSourceID];
	default:
		throw new CRainmanException(__FILE__, __LINE__, "Invalid tag");
	};
}

IDirectoryTraverser::IIterator* CDoWFileView::VIterate(const char* sPath)
{
	_VirtFolder* pFolder = &m_RootFolder;
	unsigned long iPartLength;
	const char* sSlashLoc = strchr(sPath, '\\');

	iPartLength = (unsigned long)(sSlashLoc ? sSlashLoc - sPath : strlen(sPath));
	while(iPartLength)
	{
		bool bFound = false;
		for(std::vector<_VirtFolder*>::iterator itr = pFolder->vChildFolders.begin(); itr != pFolder->vChildFolders.end(); ++itr)
		{
			if(strlen((**itr).sName) == iPartLength && strnicmp((**itr).sName, sPath, iPartLength) == 0)
			{
				pFolder = *itr;
				bFound = true;
				break;
			}
		}
		if(!bFound) throw new CRainmanException(0, __FILE__, __LINE__, "Could not find \'%s\'", sPath);

		sPath += iPartLength + (sSlashLoc ? 1 : 0);
		sSlashLoc = strchr(sPath, '\\');
		iPartLength = (unsigned long)(sSlashLoc ? sSlashLoc - sPath : strlen(sPath));
	}

	CIterator *pItr = new CIterator(pFolder, (CDoWFileView*)this);
	if(pItr && pItr->VGetType() == IDirectoryTraverser::IIterator::T_Nothing)
	{
		delete pItr;
		pItr = 0;
	}
	return pItr;
}

unsigned long CDoWFileView::VGetEntryPointCount()
{
	return 1;
}

const char* CDoWFileView::VGetEntryPoint(unsigned long iID)
{
	return m_RootFolder.sName;
}

void CDoWFileView::AddFileSource(IDirectoryTraverser* pTrav, IDirectoryTraverser::IIterator* pDirectory, IFileStore* pIOStore, const char* sMod, const char* sSourceType, bool bIsReqMod, bool bCanWrite, bool bIsOutput)
{
	if(pDirectory == 0) throw new CRainmanException(__FILE__, __LINE__, "Null pDirectory");
	if(pIOStore == 0) throw new CRainmanException(__FILE__, __LINE__, "Null pIOStore");

	unsigned long iModID = (unsigned long)m_vModNames.size();
	unsigned long iSourceID = (unsigned long)m_vSourceNames.size();

	m_vModNames.push_back(strdup(sMod));
	m_vSourceNames.push_back(strdup(sSourceType));
	m_vSourceStores.push_back(pIOStore);
	m_vSourceFlags.push_back(std::make_pair(bCanWrite, bIsOutput));
	m_vSourceDirItrs.push_back(pTrav);

	try
	{
		_RawMapFolder(iModID, iSourceID, pDirectory, &m_RootFolder, bIsReqMod);
	}
	catch(CRainmanException *pE)
	{
		throw new CRainmanException(__FILE__, __LINE__, "Raw map failed", pE);
	}
}

CDoWFileView::_VirtFile::_VirtFile()
{
	sName = 0;
	pParent = 0;
	bInReqMod = false;
	bGotWriteTime = false;
	oWriteTime = GetInvalidWriteTime();
}

CDoWFileView::_VirtFolder::_VirtFolder()
{
	sName = 0;
	sFullName = 0;
	pParent = 0;
}

void CDoWFileView::_Clean()
{
	_CleanFolder(&m_RootFolder);

	for(std::vector<char*>::iterator itr = m_vModNames.begin(); itr != m_vModNames.end(); ++itr)
	{
		if(*itr) free(*itr);
	}
	m_vModNames.clear();

	for(std::vector<char*>::iterator itr = m_vSourceNames.begin(); itr != m_vSourceNames.end(); ++itr)
	{
		if(*itr) free(*itr);
	}
	m_vSourceNames.clear();

	m_vSourceStores.clear();
}

void CDoWFileView::_CleanFolder(_VirtFolder* pFolder)
{
	// Clean source folder names
	for(std::map<unsigned long, char*>::iterator itr = pFolder->mapSourceFolderNames.begin(); itr != pFolder->mapSourceFolderNames.end(); ++itr)
	{
		if(itr->second) free(itr->second);
	}
	pFolder->mapSourceFolderNames.clear();

	// Clean files
	for(std::vector<_VirtFile*>::iterator itr = pFolder->vChildFiles.begin(); itr != pFolder->vChildFiles.end(); ++itr)
	{
		if((**itr).sName) free((**itr).sName);
		delete *itr;
	}
	pFolder->vChildFiles.clear();

	// Clean sub-folders
	for(std::vector<_VirtFolder*>::iterator itr = pFolder->vChildFolders.begin(); itr != pFolder->vChildFolders.end(); ++itr)
	{
		if((**itr).sName) free((**itr).sName);
		if((**itr).sFullName) delete[] (**itr).sFullName;
		_CleanFolder(*itr);
		delete *itr;
	}
	pFolder->vChildFolders.clear();
}

void CDoWFileView::_RawMapFolder(unsigned long iModID, unsigned long iSourceID, IDirectoryTraverser::IIterator* pSourceDirectory, _VirtFolder* pDestination, bool bIsReqMod)
{
	if(pSourceDirectory == 0) throw new CRainmanException(__FILE__, __LINE__, "No source directory");

	if(pDestination->mapSourceFolderNames[iSourceID] == 0)
	{
		pDestination->mapSourceFolderNames[iSourceID] = strdup(pSourceDirectory->VGetDirectoryPath());
	}

	while(pSourceDirectory->VGetType() != IDirectoryTraverser::IIterator::T_Nothing)
	{
		if(pSourceDirectory->VGetType() == IDirectoryTraverser::IIterator::T_File)
		{
			_VirtFile* pOldFile = 0;
			for(std::vector<_VirtFile*>::iterator itr = pDestination->vChildFiles.begin();!pOldFile && itr != pDestination->vChildFiles.end(); ++itr)
			{
				if(stricmp((**itr).sName,pSourceDirectory->VGetName()) == 0)
				{
					pOldFile = *itr;
					break;
				}
			}
			if(!pOldFile)
			{
				_VirtFile* pNewFile = new _VirtFile;
				if(pNewFile == 0) throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
				if(pNewFile->bInReqMod = bIsReqMod)
				{
					//! \todo Establish if anything was meant to go here
				}
				pNewFile->sName = strdup(pSourceDirectory->VGetName());
				pNewFile->pParent = pDestination;
				pNewFile->iModID = iModID;
				pNewFile->iSourceID = iSourceID;
				pNewFile->bGotWriteTime = true;
				pNewFile->oWriteTime = pSourceDirectory->VGetLastWriteTime();
				pDestination->vChildFiles.push_back(pNewFile);
			}
			else if(! pOldFile->bInReqMod)
			{
				pOldFile->bInReqMod = true;
				pOldFile->iReqModID = iModID;
				pOldFile->iReqSourceID = iSourceID;
			}
		}
		else
		{
			_VirtFolder* pTarget = 0;
			for(std::vector<_VirtFolder*>::iterator itr = pDestination->vChildFolders.begin();!pTarget && itr != pDestination->vChildFolders.end(); ++itr)
			{
				if(stricmp((**itr).sName,pSourceDirectory->VGetName()) == 0) pTarget = *itr;
			}
			if(!pTarget)
			{
				_VirtFolder* pNewFolder = new _VirtFolder;
				if(pNewFolder == 0) throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
				pNewFolder->pParent = pDestination;
				pNewFolder->sName = strdup(pSourceDirectory->VGetName());
				pNewFolder->sFullName = new char[strlen(pNewFolder->sName) + strlen(pDestination->sFullName) + 2];
				if(pNewFolder->sFullName == 0) throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
				strcpy(pNewFolder->sFullName, pDestination->sFullName);
				if(*pNewFolder->sFullName) strcat(pNewFolder->sFullName, "\\");
				strcat(pNewFolder->sFullName, pNewFolder->sName);

				pDestination->vChildFolders.push_back(pNewFolder);

				pTarget = pNewFolder;
			}

			IDirectoryTraverser::IIterator* pItr = pSourceDirectory->VOpenSubDir();
			try
			{
				_RawMapFolder(iModID,iSourceID, pItr, pTarget, bIsReqMod);
			}
			catch(CRainmanException* pE)
			{
				delete pItr;
				throw new CRainmanException(__FILE__, __LINE__, "Raw map of child failed", pE);
			}
			delete pItr;
		}

		if(pSourceDirectory->VNextItem() != IDirectoryTraverser::IIterator::E_OK) break;
	}

	std::sort(pDestination->vChildFiles.begin(), pDestination->vChildFiles.end(), _SortFiles);
	std::sort(pDestination->vChildFolders.begin(), pDestination->vChildFolders.end(), _SortFolds);
}

