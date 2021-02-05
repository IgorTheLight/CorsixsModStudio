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

#include "CFileMap.h"
#include "Exception.h"
#include <algorithm>
#include "memdebug.h"

CFileMap::CFileMap()
{
	m_fAuxOutputSupply = 0;
	m_pAuxOutputContext = 0;
}

void CFileMap::SetAuxOutputSupply(tAuxOutputSupply fAuxOutputSupply, void* pContext)
{
	m_fAuxOutputSupply = fAuxOutputSupply;
	m_pAuxOutputContext = pContext;
}

CFileMap::~CFileMap()
{
	_Clean();
}

void CFileMap::VInit(void* pUnused) {}

IFileStore::IStream* CFileMap::_OpenFile(CFileMap::_File* pFile, CFileMap::_Folder* pFolder, const char* sFile)
{
	if(pFile == 0) throw new CRainmanException(0, __FILE__, __LINE__, "Could not find \'%s\'", sFile);
	if(pFolder == 0) pFolder = pFile->pParent;
	if(pFolder == 0) throw new CRainmanException(0, __FILE__, __LINE__, "Found \'%s\', but did not find folder", sFile);

	if(pFile->mapSources.size() == 0) throw new CRainmanException(0, __FILE__, __LINE__, "Found \'%s\', but it is not mapped to a source", sFile);
	_DataSource* pSrc = pFile->mapSources.begin()->first;
	char* sSourceFolder = pFolder->mapSourceNames[pSrc];
	if(sSourceFolder == 0) throw new CRainmanException(0, __FILE__, __LINE__, "Found \'%s\', but folder is not mapped to the source", sFile);

	char* sFullPath = new char[strlen(sSourceFolder) + strlen(pFile->sName) + 2];

	strcpy(sFullPath, sSourceFolder);
	if(*sFullPath && sFullPath[strlen(sFullPath) - 1] != '\\') strcat(sFullPath, "\\");
	strcat(sFullPath, pFile->sName);;

	IFileStore::IStream* pStream = 0;
	try
	{
		pStream = pSrc->pStore->VOpenStream(sFullPath);
	}
	catch(CRainmanException *pE)
	{
		delete[] sFullPath;
		throw new CRainmanException(pE, __FILE__, __LINE__, "Error opening \'%s\' from file source", sFile);
	}

	delete[] sFullPath;
	return pStream;
}

void CFileMap::VCreateFolderIn(const char* sPath, const char* sNewFolderName)
{
	throw new CRainmanException(__FILE__, __LINE__, "Not coded yet ^.^");
}

tLastWriteTime CFileMap::VGetLastWriteTime(const char* sFile)
{
	_File* pFile;
	try
	{
		pFile = _FindFile(sFile, 0);
	}
	catch(CRainmanException *pE)
	{
		throw new CRainmanException(pE, __FILE__, __LINE__, "Could not find \'%s\'", sFile);
	}
	if(!pFile) throw new CRainmanException(0, __FILE__, __LINE__, "Could not find \'%s\'", sFile);
	if(pFile->mapSources.size() == 0) throw new CRainmanException(0, __FILE__, __LINE__, "Found \'%s\', but it is not mapped to anything", sFile);

	return pFile->mapSources.begin()->second;
}

CFileMap::_DataSource* CFileMap::_MakeFolderWritable(CFileMap::_Folder* pFolder, CFileMap::_File* pFile)
{
	// Work out the source sort number
	unsigned long iSortNumExisting = 0x0000ffff; // DS sort num. must be less than or equal to this
	if(pFile->mapSources.size())
	{
		_DataSource* pDS = pFile->mapSources.begin()->first;
		if(pDS->iSortNumber < iSortNumExisting) iSortNumExisting = pDS->iSortNumber;
	}

	// Check if the file already uses a suitable source
	if(pFile->pParent == pFolder && pFile->mapSources.size())
	{
		_DataSource* pDS = pFile->mapSources.begin()->first;
		if(pDS->iSortNumber <= iSortNumExisting && (pDS->bIsWritable || pDS->bIsDefaultOutput)) return pDS;
	}

	// Check the folder for siutable sources
	for(std::map<_DataSource*, char*>::iterator itr = pFolder->mapSourceNames.begin(); itr != pFolder->mapSourceNames.end(); ++itr)
	{
		_DataSource* pDS = itr->first;
		if(pDS->iSortNumber <= iSortNumExisting && pDS->bIsDefaultOutput) return pDS;
	}

	// Look in parent for a suitable source
	if(pFolder->pParent)
	{
		_DataSource* pDS = 0;
		try
		{
			pDS = _MakeFolderWritable(pFolder->pParent, pFile);
		}
		catch(CRainmanException *pE)
		{
			throw new CRainmanException(pE, __FILE__, __LINE__, "Unable to make \'%s\' writable for \'%s\'", pFolder->sFullName, pFile->sName);
		}

		char* sSourceName = pFolder->pParent->mapSourceNames[pDS];

		try
		{
			pDS->pTraverser->VCreateFolderIn(sSourceName, pFolder->sName);
		}
		catch(CRainmanException *pE)
		{
			throw new CRainmanException(pE, __FILE__, __LINE__, "Cannot create folder in \'%s\' to make \'%s\' writable for \'%s\'", sSourceName, pFolder->sFullName, pFile->sName);
		}

		size_t iL = strlen(sSourceName);
		char* sNewFolderName = CHECK_MEM( (char*) malloc(iL + 2 + strlen(pFolder->sName)) );
		if( (sSourceName[iL - 1] == '/') || (sSourceName[iL - 1] == '\\') )
		{
			sprintf(sNewFolderName, "%s%s%c", sSourceName, pFolder->sName, sSourceName[iL - 1]);
		}
		else
		{
			sprintf(sNewFolderName, "%s\\%s", sSourceName, pFolder->sName);
		}

		pFolder->mapSourceNames[pDS] = sNewFolderName;

		return pDS;
	}

	throw new CRainmanException(0, __FILE__, __LINE__, "Unable to make \'%s\' writable for \'%s\'", pFolder->sFullName, pFile->sName);
}

IFileStore::IOutputStream* CFileMap::VOpenOutputStream(const char* sFile, bool bEraseIfPresent)
{
	bool bFileWasCreated = false;
	_File* pFile;
	_Folder* pFolder = 0;

	try
	{
		pFile = _FindFile(sFile, &pFolder);
	}
	catch(CRainmanException *pE)
	{
		throw new CRainmanException(pE, __FILE__, __LINE__, "Could not find \'%s\'", sFile);
	}

	if(pFolder == 0) throw new CRainmanException(0, __FILE__, __LINE__, "No folder found for \'%s\'", sFile);

	if(pFile == 0)
	{
		pFile = new _File;
		bFileWasCreated = true;

		pFile->pParent = pFolder;
		pFile->sName = strdup(sFile + strlen(pFolder->sFullName) + 1);
	}

	_DataSource *pDS = 0;

	try
	{
		pDS = _MakeFolderWritable(pFolder, pFile);
	}
	catch(CRainmanException *pE)
	{
		if(bFileWasCreated)
		{
			free(pFile->sName);
			delete pFile;
		}
		if(m_fAuxOutputSupply)
		{
			try
			{
				IFileStore::IOutputStream* pOutStr = m_fAuxOutputSupply(sFile, bEraseIfPresent, m_pAuxOutputContext);
				if(pOutStr)
				{
					pE->destroy();
					return pOutStr;
				}
			} IGNORE_EXCEPTIONS
		}
		if(strnicmp(sFile, "data", 4) == 0)
			throw new CRainmanException(pE, __FILE__, __LINE__, "Could not find suitable output location for \'%s\' (could be because the Data folder is missing)", sFile);
		else
			throw new CRainmanException(pE, __FILE__, __LINE__, "Could not find suitable output location for \'%s\' (could be because you\'re trying to mod RelicCOH)", sFile);
	}

	if(!bEraseIfPresent)
	{
		if(pFile->mapSources.size() && pDS != pFile->mapSources.begin()->first)
		{
			if(bFileWasCreated)
			{
				free(pFile->sName);
				delete pFile;
			}
			throw new CRainmanException(0, __FILE__, __LINE__, "The output location for \'%s\' is incompatible with \'bEraseIfPresent = false\'", sFile);
		}
	}

	char* sFolderPath = pFolder->mapSourceNames[pDS];
	size_t iL = strlen(sFolderPath);
	char* sFullFilePath = new char[iL + 2 + strlen(pFile->sName)];

	strcpy(sFullFilePath, sFolderPath);
	if( ! ((sFolderPath[iL - 1] == '\\') || (sFolderPath[iL - 1] == '/')) )
	{
		strcat(sFullFilePath, "\\");
	}
	strcat(sFullFilePath, pFile->sName);

	IFileStore::IOutputStream* pOutStr = 0;

	try
	{
		pOutStr = pDS->pStore->VOpenOutputStream(sFullFilePath, bEraseIfPresent);
		if(!pDS->bIsPureOutput)
			pFile->mapSources[pDS] = pDS->pTraverser->VGetLastWriteTime(sFullFilePath);

		if(bFileWasCreated) pFolder->vChildFiles.push_back(pFile);
		bFileWasCreated = false;
		std::sort(pFolder->vChildFiles.begin(), pFolder->vChildFiles.end(), _SortFiles);
	}
	catch(CRainmanException *pE)
	{
		PAUSE_THROW(pE, __FILE__, __LINE__, "Error opening \'%s\' from file store (\'%s\')", sFile, sFullFilePath);

		if(pOutStr) delete pOutStr;
		delete[] sFullFilePath;

		if(bFileWasCreated)
		{
			free(pFile->sName);
			delete pFile;
		}
		
		UNPAUSE_THROW;
	}

	delete[] sFullFilePath;

	return pOutStr;
}

IFileStore::IStream* CFileMap::VOpenStream(const char* sFile)
{
	_File* pFile;
	_Folder* pFolder = 0;

	try
	{
		pFile = _FindFile(sFile, &pFolder);
		return _OpenFile(pFile, pFolder, sFile);
	}
	catch(CRainmanException *pE)
	{
		throw new CRainmanException(pE, __FILE__, __LINE__, "Could not find \'%s\'", sFile);
	}
}

unsigned long CFileMap::VGetEntryPointCount()
{
	return (unsigned long) m_vTOCs.size();
}

const char* CFileMap::VGetEntryPoint(unsigned long iID)
{
	if(iID >= VGetEntryPointCount()) throw new CRainmanException(0, __FILE__, __LINE__, "TOC %lu is beyond maximum of %lu", iID, VGetEntryPointCount());
	return m_vTOCs[iID]->sName;
}

void CFileMap::CIterator::_MakeFullName()
{
	const char* sPartB = "";
	if(m_eWhat == IW_Folders)
	{
		sPartB = (**m_FoldIter).sName;
	}
	else if(m_eWhat == IW_Files)
	{
		sPartB = (**m_FileIter).sName;
	}

	if(m_sFullPath) delete[] m_sFullPath;
	m_sFullPath = CHECK_MEM(new char[strlen(m_sParentPath) + strlen(sPartB) + 1]);
	sprintf(m_sFullPath, "%s%s", m_sParentPath, sPartB);
}

CFileMap::CIterator::CIterator(CFileMap::_Folder* pFolder, CFileMap* pFileMap)
{
	if(!pFolder) throw new CRainmanException(__FILE__, __LINE__, "No folder specified");
	if(!pFileMap) throw new CRainmanException(__FILE__, __LINE__, "No file map specified");
	m_pDirectory = pFolder;
	m_pFileMap = pFileMap;
	m_sFullPath = 0;

	m_sParentPath = CHECK_MEM(new char[strlen(pFolder->sFullName) + 2]);
	sprintf(m_sParentPath, "%s\\", pFolder->sFullName);

	m_eWhat = IW_Folders;
	m_FoldIter = pFolder->vChildFolders.begin();
	if(m_FoldIter == pFolder->vChildFolders.end())
	{
		m_eWhat = IW_Files;
		m_FileIter = pFolder->vChildFiles.begin();
		if(m_FileIter == pFolder->vChildFiles.end())
		{
			m_eWhat = IW_None;
		}
	}

	_MakeFullName();
}

CFileMap::CIterator::~CIterator()
{
	delete[] m_sParentPath;
	if(m_sFullPath) delete[] m_sFullPath;
}

IDirectoryTraverser::IIterator::eTypes CFileMap::CIterator::VGetType()
{
	switch(m_eWhat)
	{
	case IW_Folders:
		return IDirectoryTraverser::IIterator::T_Directory;

	case IW_Files:
		return IDirectoryTraverser::IIterator::T_File;

	default:
		return IDirectoryTraverser::IIterator::T_Nothing;
	}
}

IDirectoryTraverser::IIterator* CFileMap::CIterator::VOpenSubDir()
{
	if(m_eWhat != IW_Folders) throw new CRainmanException(__FILE__, __LINE__, "Current item is not a folder");
	try
	{
		return new CIterator(*m_FoldIter, m_pFileMap);
	}
	catch(CRainmanException *pE)
	{
		throw new CRainmanException(__FILE__, __LINE__, "Unable to iterate sub-folder", pE);
	}
}

IFileStore::IStream* CFileMap::CIterator::VOpenFile()
{
	if(m_eWhat != IW_Files) throw new CRainmanException(__FILE__, __LINE__, "Current item is not a file");
	try
	{
		return m_pFileMap->_OpenFile(*m_FileIter, m_pDirectory, m_sFullPath);
	}
	catch(CRainmanException *pE)
	{
		throw new CRainmanException(__FILE__, __LINE__, "Unable to open file", pE);
	}
}

IDirectoryTraverser::IIterator* CFileMap::VIterate(const char* sPath)
{
	_Folder *pFold = 0;
	try
	{
		_FindFile(sPath, &pFold, true);
	}
	catch(CRainmanException *pE)
	{
		throw new CRainmanException(pE, __FILE__, __LINE__, "Problem finding \'%s\'", sPath);
	}
	if(pFold == 0) throw new CRainmanException(0, __FILE__, __LINE__, "Problem finding \'%s\'", sPath);

	return new CIterator(pFold, (CFileMap*)this );
}

const char* CFileMap::CIterator::VGetName()
{
	switch(m_eWhat)
	{
	case IW_Folders:
		return (**m_FoldIter).sName;

	case IW_Files:
		return (**m_FileIter).sName;

	default:
		throw new CRainmanException(__FILE__, __LINE__, "Only files and folders can have names");
	}
}

const char* CFileMap::CIterator::VGetFullPath()
{
	return m_sFullPath;
}

const char* CFileMap::CIterator::VGetDirectoryPath()
{
	return m_sParentPath;
}

IDirectoryTraverser::IIterator::eErrors CFileMap::CIterator::VNextItem()
{
	if(m_eWhat == IW_Files)
	{
		++m_FileIter;
		if(m_FileIter == m_pDirectory->vChildFiles.end())
		{
			m_eWhat = IW_None;
		}
	}
	else if(m_eWhat == IW_Folders)
	{
		++m_FoldIter;
		if(m_FoldIter == m_pDirectory->vChildFolders.end())
		{
			m_eWhat = IW_Files;
			m_FileIter = m_pDirectory->vChildFiles.begin();
			if(m_FileIter == m_pDirectory->vChildFiles.end())
			{
				m_eWhat = IW_None;
			}
		}
	}

	_MakeFullName();

	if(m_eWhat == IW_None) return E_AtEnd;

	return E_OK;
}

tLastWriteTime CFileMap::CIterator::VGetLastWriteTime()
{
	if(m_eWhat != IW_Files) throw new CRainmanException(__FILE__, __LINE__, "Current item is not a file");
	if((**m_FileIter).mapSources.size() == 0) throw new CRainmanException(__FILE__, __LINE__, "File is not mapped");
	return (**m_FileIter).mapSources.begin()->second;
}

void* CFileMap::CIterator::VGetTag(long iTag)
{
	if(m_eWhat == IW_Files)
	{
		CFileMap::_DataSource *pSrc = 0;
		if((**m_FileIter).mapSources.size() == 0) return 0;
		pSrc = (**m_FileIter).mapSources.begin()->first;

		if(iTag == 0) return (void*) pSrc->sModName;
		if(iTag == 1) return (void*) pSrc->sSourceName;
		if(iTag == 2) return (void*) (unsigned long) pSrc->GetMod();
		if(iTag == 3) return (void*) (unsigned long) (pSrc->GetSourceType() ? 1 : 0);
	}

	return 0;
}

void CFileMap::RewriteToC(const char* sOld, const char* sNew)
{
	_TOC *pTocKey = 0;

	for(std::vector<_TOC*>::iterator itr = m_vTOCs.begin(); itr != m_vTOCs.end(); ++itr)
	{
		if( stricmp((**itr).sName, sOld) == 0 )
		{
			pTocKey = *itr;
			break;
		}
	}

	if(pTocKey == 0)
	{
		pTocKey = new _TOC;
		pTocKey->sName = strdup(sOld);
		pTocKey->pRootFolder = new _Folder;
		pTocKey->pRootFolder->pParent = 0;
		pTocKey->pRootFolder->sFullName = strdup(sOld);
		pTocKey->pRootFolder->sName = pTocKey->pRootFolder->sFullName;

		m_vTOCs.push_back(pTocKey);
	}

	if(sNew == 0)
		m_mapTOCRewrites.erase(pTocKey);
	else
	{
		_TOC *pTocVal = 0;

		for(std::vector<_TOC*>::iterator itr = m_vTOCs.begin(); itr != m_vTOCs.end(); ++itr)
		{
			if( stricmp((**itr).sName, sNew) == 0 )
			{
				pTocVal = *itr;
				break;
			}
		}

		if(pTocVal == 0)
		{
			pTocVal = new _TOC;
			pTocVal->sName = strdup(sNew);
			pTocVal->pRootFolder = new _Folder;
			pTocVal->pRootFolder->pParent = 0;
			pTocVal->pRootFolder->sFullName = strdup(sNew);
			pTocVal->pRootFolder->sName = pTocVal->pRootFolder->sFullName;

			m_vTOCs.push_back(pTocVal);
		}

		m_mapTOCRewrites[pTocKey] = pTocVal;
	}
}

void CFileMap::UnrewriteToC(const char* sOld)
{
	RewriteToC(sOld, 0);
}

CFileMap::_File* CFileMap::_FindFile(const char* sName, CFileMap::_Folder** ppFolder, bool bIsFolder)
{
	const char* sSlashLoc = strchr(sName, '\\');
	if(sSlashLoc == 0) sSlashLoc = strchr(sName, '/');
	//if(sSlashLoc == 0) throw new CRainmanException(0, __FILE__, __LINE__, "No TOC in path \'%s\'", sName);

	size_t iPartLen = sSlashLoc ? sSlashLoc - sName : strlen(sName);
	_TOC *pToc = 0;

	for(std::vector<_TOC*>::iterator itr = m_vTOCs.begin(); itr != m_vTOCs.end(); ++itr)
	{
		if( (strlen((**itr).sName) == iPartLen) &&  (strnicmp((**itr).sName, sName, iPartLen) == 0) )
		{
			pToc = *itr;
			break;
		}
	}
	if(m_mapTOCRewrites[pToc])
		pToc = m_mapTOCRewrites[pToc];

	if(pToc == 0)
	{
		throw new CRainmanException(0, __FILE__, __LINE__, "Unknown TOC in path \'%s\'", sName);
	}

	_Folder* pFolder = pToc->pRootFolder;
	const char* sNameLeft = 0;
	while(sSlashLoc)
	{
		sNameLeft = sSlashLoc + 1;
		sSlashLoc = strchr(sNameLeft, '\\');
		if(sSlashLoc == 0) sSlashLoc = strchr(sNameLeft, '/');
		if(!bIsFolder && sSlashLoc == 0) break;
		iPartLen = sSlashLoc ? sSlashLoc - sNameLeft : strlen(sNameLeft);

		if(iPartLen == 0) break;

		bool bFound = false;
		size_t iBSL = 0;
		size_t iBSH = pFolder->vChildFolders.size();
		size_t iBSM = (iBSL + iBSH) >> 1;
		while(!bFound && iBSH > iBSL)
		{
			int iNiRes = strnicmp(pFolder->vChildFolders[iBSM]->sName, sNameLeft, iPartLen);
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
				if(strlen(pFolder->vChildFolders[iBSM]->sName) != iPartLen)
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
		if(!bFound)
		{
			_Folder* pNewFolder = new _Folder;
			pNewFolder->pParent = pFolder;
			size_t iLParent = strlen(pFolder->sFullName);
			pNewFolder->sFullName = new char[iLParent + iPartLen + 2];
			sprintf(pNewFolder->sFullName, "%s\\%.*s", pFolder->sFullName, (int)iPartLen, sNameLeft);
			pNewFolder->sName = pNewFolder->sFullName + iLParent + 1;

			pFolder->vChildFolders.push_back(pNewFolder);
			std::sort(pFolder->vChildFolders.begin(), pFolder->vChildFolders.end(), _SortFolds);

			pFolder = pNewFolder;
		}

		if(bIsFolder && sSlashLoc == 0)
		{
			sNameLeft = 0;
			break;
		}
	}

	if(ppFolder) *ppFolder = pFolder;

	if(sNameLeft == 0 || *sNameLeft == 0)
	{
		// Nothing after the final slash, thus is a folder name only
		return 0;
	}

	size_t iBSL = 0;
	size_t iBSH = pFolder->vChildFiles.size();
	size_t iBSM = (iBSL + iBSH) >> 1;
	while(iBSH > iBSL)
	{
		int iNiRes = stricmp(pFolder->vChildFiles[iBSM]->sName, sNameLeft);
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
			return pFolder->vChildFiles[iBSM];
		}
	}

	// file not found
	return 0;
}

bool CFileMap::_SortFolds(CFileMap::_Folder* a, CFileMap::_Folder* b)
{
	return (stricmp(a->sName, b->sName) < 0);
}

bool CFileMap::_SortFiles(CFileMap::_File* a, CFileMap::_File* b)
{
	return (stricmp(a->sName, b->sName) < 0);
}

void* CFileMap::RegisterSource(unsigned short iModNum, bool bIsSga, unsigned short iSourceNum, const char* sModName, const char* sSourceName, IFileStore* pStore, IDirectoryTraverser* pTraverser, bool bIsWritable, bool bIsOutput, bool bIsPureOutput)
{
	_DataSource* pSource = CHECK_MEM(new _DataSource);
	
	pSource->iSortNumber = 0;
	pSource->SetMod(iModNum);
	pSource->SetSourceType(bIsSga ? 1 : 0);
	pSource->SetSourceNumber(iSourceNum);
	pSource->sModName = strdup(sModName);
	pSource->sSourceName = strdup(sSourceName);
	pSource->pStore = pStore;
	pSource->pTraverser = pTraverser;
	pSource->bIsWritable = bIsWritable;
	pSource->bIsDefaultOutput = bIsOutput;
	pSource->bIsPureOutput = bIsPureOutput;

	m_vDataSources.push_back(pSource);

	return (void*) pSource;
}

void CFileMap::MapSga(void* pSource, CSgaFile* pSga)
{
	unsigned long iL = pSga->VGetEntryPointCount();
	for(unsigned long i = 0; i < iL; ++i)
	{
		const char* sToc = pSga->VGetEntryPoint(i);
		IDirectoryTraverser::IIterator* pItr = pSga->VIterate(sToc);
		MapIterator(pSource, sToc, pItr);
		delete pItr;
	}
}

void CFileMap::_CleanFolder(_Folder* pFolder)
{
	free(pFolder->sFullName);

	for(std::vector<_File*>::iterator itr = pFolder->vChildFiles.begin(); itr != pFolder->vChildFiles.end(); ++itr)
	{
		free((**itr).sName);
		delete *itr;
	}

	for(std::vector<_Folder*>::iterator itr = pFolder->vChildFolders.begin(); itr != pFolder->vChildFolders.end(); ++itr)
	{
		_CleanFolder(*itr);
	}

	for(std::map<_DataSource*, char*>::iterator itr = pFolder->mapSourceNames.begin(); itr != pFolder->mapSourceNames.end(); ++itr)
	{
		free(itr->second);
	}

	delete pFolder;
}

void CFileMap::_Clean()
{
	for(std::vector<_TOC*>::iterator itr = m_vTOCs.begin(); itr != m_vTOCs.end(); ++itr)
	{
		free((**itr).sName);
		_CleanFolder((**itr).pRootFolder);
		delete *itr;
	}

	for(std::vector<_DataSource*>::iterator itr = m_vDataSources.begin(); itr != m_vDataSources.end(); ++itr)
	{
		free((**itr).sModName);
		free((**itr).sSourceName);
		delete *itr;
	}
}

void CFileMap::MapSingleFile(void* pSource, const char* sTocName, const char* sFullPath, const char* sPathPartial)
{
	// Get ToC
	_TOC* pToc = 0;
	for(std::vector<_TOC*>::iterator itr = m_vTOCs.begin(); itr != m_vTOCs.end(); ++itr)
	{
		if(stricmp((**itr).sName, sTocName) == 0)
		{
			pToc = *itr;
			break;
		}
	}
	if(!pToc)
	{
		pToc = new _TOC;
		pToc->sName = strdup(sTocName);
		pToc->pRootFolder = new _Folder;
		pToc->pRootFolder->pParent = 0;
		pToc->pRootFolder->sFullName = strdup(sTocName);
		pToc->pRootFolder->sName = pToc->pRootFolder->sFullName;

		m_vTOCs.push_back(pToc);
	}

	// Get Folder
	if(*sPathPartial == '\\') ++sPathPartial;
	_Folder* pCurrentFolder = pToc->pRootFolder;
	_FolderSetupSourceNameFromSingleFileMap(pCurrentFolder, (_DataSource*)pSource, sFullPath, sPathPartial);
	const char* sPathSeperator = strchr(sPathPartial, '\\');
	while(sPathSeperator)
	{
		size_t iPartL = sPathSeperator - sPathPartial;

		_Folder* pTheFolder = 0;
		for(std::vector<_Folder*>::iterator itr = pCurrentFolder->vChildFolders.begin(); itr != pCurrentFolder->vChildFolders.end(); ++itr)
		{
			if(strlen((**itr).sName) == iPartL && strnicmp((**itr).sName, sPathPartial, iPartL) == 0)
			{
				pTheFolder = *itr;
				break;
			}
		}
		if(pTheFolder == 0)
		{
			pTheFolder = new _Folder;
			pTheFolder->pParent = pCurrentFolder;
			size_t iLParent = strlen(pCurrentFolder->sFullName);
			pTheFolder->sFullName = new char[iLParent + iPartL + 2];
			sprintf(pTheFolder->sFullName, "%s\\", pCurrentFolder->sFullName);
			strncat(pTheFolder->sFullName, sPathPartial, iPartL);
			pTheFolder->sName = pTheFolder->sFullName + iLParent + 1;

			pCurrentFolder->vChildFolders.push_back(pTheFolder);
			std::sort(pCurrentFolder->vChildFolders.begin(), pCurrentFolder->vChildFolders.end(), _SortFolds);

			pCurrentFolder = pTheFolder;
			sPathPartial = sPathSeperator + 1;
			sPathSeperator = strchr(sPathPartial, '\\');
			_FolderSetupSourceNameFromSingleFileMap(pCurrentFolder, (_DataSource*)pSource, sFullPath, sPathPartial);
		}
		else
		{
			pCurrentFolder = pTheFolder;
			sPathPartial = sPathSeperator + 1;
			sPathSeperator = strchr(sPathPartial, '\\');
		}
	}

	// Do File
	_File* pTheFile = 0;
	for(std::vector<_File*>::iterator itr = pCurrentFolder->vChildFiles.begin(); itr != pCurrentFolder->vChildFiles.end(); ++itr)
	{
		if(stricmp(sPathPartial, (**itr).sName) == 0)
		{
			pTheFile = *itr;
			break;
		}
	}
	if(pTheFile == 0)
	{
		pTheFile = new _File;
		pTheFile->pParent = pCurrentFolder;
		pTheFile->sName = strdup(sPathPartial);
		pCurrentFolder->vChildFiles.push_back(pTheFile);
		std::sort(pCurrentFolder->vChildFiles.begin(), pCurrentFolder->vChildFiles.end(), _SortFiles);
	}
	pTheFile->mapSources[(_DataSource*)pSource] = ((_DataSource*)pSource)->pTraverser->VGetLastWriteTime(sFullPath);
}

void CFileMap::MapIteratorDeep(void* pSource, const char* sPath, IDirectoryTraverser::IIterator* pItr)
{
	const char* sPathSplit = strchr(sPath, '\\');
	size_t iPartLen = sPathSplit ? (sPathSplit - sPath) : strlen(sPath);

	_TOC* pToc = 0;
	for(std::vector<_TOC*>::iterator itr = m_vTOCs.begin(); itr != m_vTOCs.end(); ++itr)
	{
		if(strlen((**itr).sName) == iPartLen && strnicmp((**itr).sName, sPath, iPartLen) == 0)
		{
			pToc = *itr;
			break;
		}
	}
	if(!pToc)
	{
		pToc = new _TOC;
		pToc->sName = (char*) malloc(iPartLen+1);
		memcpy(pToc->sName, sPath, iPartLen);
		pToc->sName[iPartLen] = 0;
		pToc->pRootFolder = new _Folder;
		pToc->pRootFolder->pParent = 0;
		pToc->pRootFolder->sFullName = strdup(pToc->sName);
		pToc->pRootFolder->sName = pToc->pRootFolder->sFullName;

		m_vTOCs.push_back(pToc);
	}
	_Folder *pFolder = pToc->pRootFolder, *pChild = 0;
	while(sPathSplit)
	{
		sPath = sPathSplit + 1;
		sPathSplit = strchr(sPath, '\\');
		iPartLen = sPathSplit ? (sPathSplit - sPath) : strlen(sPath);

		pChild = 0;
		for(std::vector<_Folder*>::iterator itr = pFolder->vChildFolders.begin(); itr != pFolder->vChildFolders.end(); ++itr)
		{
			if(strlen((**itr).sName) == iPartLen && strnicmp((**itr).sName, sPath, iPartLen) == 0)
			{
				pChild = *itr;
				break;
			}
		}
		if(pChild == 0)
		{
			pChild = new _Folder;
			pChild->pParent = pFolder;
			pChild->sName = (char*) malloc(iPartLen + 1);
			memcpy(pChild->sName, sPath, iPartLen);
			pChild->sName[iPartLen] = 0;
			pChild->sFullName = (char*) malloc(strlen(pFolder->sFullName) + iPartLen + 2);
			sprintf(pChild->sFullName, "%s\\%s", pFolder->sFullName, pChild->sName);

			pFolder->vChildFolders.push_back(pChild);
			std::sort(pFolder->vChildFolders.begin(), pFolder->vChildFolders.end(), _SortFolds);
		}

		pFolder = pChild;
	}
	_RawMap((_DataSource*) pSource, pItr, pFolder);
}

void CFileMap::MapIterator(void* pSource, const char* sTocName, IDirectoryTraverser::IIterator* pItr)
{
	_TOC* pToc = 0;
	for(std::vector<_TOC*>::iterator itr = m_vTOCs.begin(); itr != m_vTOCs.end(); ++itr)
	{
		if(stricmp((**itr).sName, sTocName) == 0)
		{
			pToc = *itr;
			break;
		}
	}
	if(!pToc)
	{
		pToc = new _TOC;
		pToc->sName = strdup(sTocName);
		pToc->pRootFolder = new _Folder;
		pToc->pRootFolder->pParent = 0;
		pToc->pRootFolder->sFullName = strdup(sTocName);
		pToc->pRootFolder->sName = pToc->pRootFolder->sFullName;

		m_vTOCs.push_back(pToc);
	}

	_RawMap((_DataSource*) pSource, pItr, pToc->pRootFolder);
}

void CFileMap::_FolderSetupSourceNameFromSingleFileMap(_Folder* pFolder, _DataSource* pDataSource, const char* sPathFull, const char* sPathPartLeft)
{
	if(pFolder->mapSourceNames[pDataSource] == 0)
	{
		size_t iLFull = strlen(sPathFull);
		size_t iLPart = strlen(sPathPartLeft);
		size_t iL = iLFull - iLPart - 1;
		char* sVal = (char*)malloc(iL + 1);
		memcpy(sVal, sPathFull, iL);
		sVal[iL] = 0;
		pFolder->mapSourceNames[pDataSource] = sVal;
	}
}

void CFileMap::EraseSource(void* _pSource)
{
	_DataSource* pSource = (_DataSource*)_pSource;
	for(std::vector<_TOC*>::iterator itr = m_vTOCs.begin(); itr != m_vTOCs.end(); ++itr)
	{
		_EraseSourceFromFolder(pSource, (**itr).pRootFolder);
	}

	for(std::vector<_DataSource*>::iterator itr = m_vDataSources.begin(); itr != m_vDataSources.end(); ++itr)
	{
		if(*itr == pSource)
		{
			free((**itr).sModName);
			free((**itr).sSourceName);
			delete *itr;
			m_vDataSources.erase(itr);
			break;
		}
	}
}

void CFileMap::_EraseSourceFromFolder(_DataSource* pSource, _Folder* pFolder)
{
	if(pFolder->mapSourceNames.find(pSource) != pFolder->mapSourceNames.end())
	{
		free(pFolder->mapSourceNames[pSource]);
		pFolder->mapSourceNames.erase(pSource);
	}

	for(std::vector<_Folder*>::iterator itr = pFolder->vChildFolders.begin(); itr != pFolder->vChildFolders.end(); ++itr)
	{
		_EraseSourceFromFolder(pSource, *itr);
	}

	for(std::vector<_File*>::iterator itr = pFolder->vChildFiles.begin(); itr != pFolder->vChildFiles.end(); ++itr)
	{
		if( (**itr).mapSources.find(pSource) != (**itr).mapSources.end() )
		{
			(**itr).mapSources.erase(pSource);
		}
	}
}

void CFileMap::_RawMap(_DataSource *pSource, IDirectoryTraverser::IIterator* pItr, _Folder* pFolder)
{
	if(pFolder->mapSourceNames[pSource] == 0)
	{
		pFolder->mapSourceNames[pSource] = strdup(pItr->VGetDirectoryPath());
	}

	while(pItr->VGetType() != IDirectoryTraverser::IIterator::T_Nothing)
	{
		if(pItr->VGetType() == IDirectoryTraverser::IIterator::T_File)
		{
			if(!pSource->bIsPureOutput)
			{
				_File* pTheFile = 0;
				for(std::vector<_File*>::iterator itr = pFolder->vChildFiles.begin(); itr != pFolder->vChildFiles.end(); ++itr)
				{
					if(stricmp((**itr).sName, pItr->VGetName()) == 0)
					{
						pTheFile = *itr;
						break;
					}
				}
				if(pTheFile == 0)
				{
					pTheFile = new _File;
					pTheFile->pParent = pFolder;
					pTheFile->sName = strdup(pItr->VGetName());
					pFolder->vChildFiles.push_back(pTheFile);
				}
				pTheFile->mapSources[pSource] = pItr->VGetLastWriteTime();
			}
		}
		else
		{
			_Folder* pTheFolder = 0;
			for(std::vector<_Folder*>::iterator itr = pFolder->vChildFolders.begin(); itr != pFolder->vChildFolders.end(); ++itr)
			{
				if(stricmp((**itr).sName, pItr->VGetName()) == 0)
				{
					pTheFolder = *itr;
					break;
				}
			}
			if(pTheFolder == 0)
			{
				pTheFolder = new _Folder;
				pTheFolder->pParent = pFolder;
				size_t iLParent = strlen(pFolder->sFullName);
				size_t iLThis = strlen(pItr->VGetName());
				pTheFolder->sFullName = new char[iLParent + iLThis + 2];
				sprintf(pTheFolder->sFullName, "%s\\%s", pFolder->sFullName, pItr->VGetName());
				pTheFolder->sName = pTheFolder->sFullName + iLParent + 1;

				pFolder->vChildFolders.push_back(pTheFolder);
			}

			IDirectoryTraverser::IIterator* pSubItr = pItr->VOpenSubDir();
			try
			{
				_RawMap(pSource, pSubItr, pTheFolder);
			}
			catch(CRainmanException* pE)
			{
				delete pSubItr;
				throw new CRainmanException(pE, __FILE__, __LINE__, "Raw map of \'%s\' failed", pFolder->sFullName);
			}
			delete pSubItr;
		}

		if(pItr->VNextItem() != IDirectoryTraverser::IIterator::E_OK) break;
	}

	std::sort(pFolder->vChildFiles.begin(), pFolder->vChildFiles.end(), _SortFiles);
	std::sort(pFolder->vChildFolders.begin(), pFolder->vChildFolders.end(), _SortFolds);
}

