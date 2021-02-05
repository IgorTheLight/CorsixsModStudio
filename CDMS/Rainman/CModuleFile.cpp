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

#include "CModuleFile.h"
#include "Internal_Util.h"
#include "Exception.h"
#include <new>
#include "memdebug.h"

CModuleFile::CModuleFile()
{
	m_eModuleType = MT_DawnOfWar;
	m_sLocale = strdup("");
	m_sApplicationPath = 0;
	m_sFilename = 0;
	m_pParentModule = 0;
	m_sScenarioPackRootFolder = 0;
	m_saScenarioPackRootFolder = 0;

	m_sUiName = 0;
	m_sFileMapName = 0;
	m_sName = 0;
	m_sDescription = 0;
	m_sDllName = 0;
	m_sModFolder =0;
	m_iModVersionMajor = 0;
	m_iModVersionMinor = 0;
	m_iModVersionRevision = 0;
	m_sTextureFe = 0;
	m_sTextureIcon = 0;
	m_sPatcherUrl = 0;
	m_sLocFolder = 0;
	m_sScenarioPackFolder = 0;

	m_pFSS = 0;
	m_sCohThisModFolder = 0;
	m_pNewFileMap = 0;

	m_iFileMapModNumber = 0;

	m_bIsFauxModule = false;
}

CModuleFile::~CModuleFile()
{
	if(!m_pParentModule)
	{
		_Clean();
		if(m_pFSS) delete m_pFSS;
		if(m_sLocale) free(m_sLocale);
	}
	else
	{
		_Clean();
	}
}

void CModuleFile::SetLocale(const char* sLocale)
{
	if(m_sLocale) free(m_sLocale);
	m_sLocale = strdup(sLocale);
}

const char* CModuleFile::GetLocale() const
{
	return m_sLocale;
}

void CModuleFile::SetMapPackRootFolder(const wchar_t* sFolder)
{
	if(m_sScenarioPackRootFolder) free(m_sScenarioPackRootFolder);
	free(m_saScenarioPackRootFolder);
	m_sScenarioPackRootFolder = wcsdup(sFolder);
	m_saScenarioPackRootFolder = (char*)malloc(wcslen(sFolder) + 1);

	int i = -1;
	do
	{
		++i;
		m_saScenarioPackRootFolder[i] = (char)m_sScenarioPackRootFolder[i];
	} while(m_sScenarioPackRootFolder[i]);
}

const wchar_t* CModuleFile::GetMapPackRootFolder() const
{
	return m_sScenarioPackRootFolder;
}

#define GET_SET_DIRECTIVE(name, int_name) const char* CModuleFile::Get ## name() const \
{ \
	return int_name; \
} \
void CModuleFile::Set ## name(const char* value) \
{ \
	if(int_name) free(int_name); \
	int_name = strdup(value); \
}

	GET_SET_DIRECTIVE(Name, m_sName)
	GET_SET_DIRECTIVE(ModFolder, m_sModFolder)
	GET_SET_DIRECTIVE(LocaleFolder, m_sLocFolder)
	GET_SET_DIRECTIVE(Description, m_sDescription)
	GET_SET_DIRECTIVE(TextureFe, m_sTextureFe)
	GET_SET_DIRECTIVE(TextureIcon, m_sTextureIcon)
	GET_SET_DIRECTIVE(UiName, m_sUiName)
	GET_SET_DIRECTIVE(DllName, m_sDllName)

#undef GET_SET_DIRECTIVE
#define GET_SET_DIRECTIVE(name, int_name) long CModuleFile::Get ## name() const \
{ \
	return int_name; \
} \
void CModuleFile::Set ## name(long value) \
{ \
	int_name = value; \
}

	GET_SET_DIRECTIVE(VersionMajor, m_iModVersionMajor)
	GET_SET_DIRECTIVE(VersionMinor, m_iModVersionMinor)
	GET_SET_DIRECTIVE(VersionRevision, m_iModVersionRevision)

#undef GET_SET_DIRECTIVE

size_t CModuleFile::GetEngineCount() const
{
	return m_vEngines.size();
}

CModuleFile* CModuleFile::GetEngine(size_t iId)
{
	if(iId < 0 || iId >= GetEngineCount()) throw new CRainmanException(0, __FILE__, __LINE__, "ID %lu is outside of range 0-%lu", (unsigned long) iId, (unsigned long)GetEngineCount());
	return m_vEngines[iId];
}

const CModuleFile* CModuleFile::GetEngine(size_t iId) const
{
	if(iId < 0 || iId >= GetEngineCount()) throw new CRainmanException(0, __FILE__, __LINE__, "ID %lu is outside of range 0-%lu", (unsigned long) iId, (unsigned long)GetEngineCount());
	return m_vEngines[iId];
}

CModuleFile::eModuleType CModuleFile::GetModuleType() const
{
	return m_eModuleType;
}

#define QUICK_TRYCAT(op) try { op } CATCH_THROW("File view error")

// IFileStore Interface
void CModuleFile::VInit(void* pUnused)
{
	if(m_pNewFileMap == 0) m_pNewFileMap = new CFileMap;
	QUICK_TRYCAT(m_pNewFileMap->VInit(pUnused);)
}

IFileStore::IStream* CModuleFile::VOpenStream(const char* sFile)
{
	if(m_pNewFileMap == 0) m_pNewFileMap = new CFileMap;
	QUICK_TRYCAT(return m_pNewFileMap->VOpenStream(sFile);)
}

IFileStore::IOutputStream* CModuleFile::VOpenOutputStream(const char* sIdentifier, bool bEraseIfPresent)
{
	if(m_pNewFileMap == 0) m_pNewFileMap = new CFileMap;
	QUICK_TRYCAT(return m_pNewFileMap->VOpenOutputStream(sIdentifier, bEraseIfPresent);)
}

// IDirectoryTraverser Interface
tLastWriteTime CModuleFile::VGetLastWriteTime(const char* sPath)
{
	if(m_pNewFileMap == 0) m_pNewFileMap = new CFileMap;
	QUICK_TRYCAT(return m_pNewFileMap->VGetLastWriteTime(sPath);)
}

void CModuleFile::VCreateFolderIn(const char* sPath, const char* sNewFolderName)
{
	if(m_pNewFileMap == 0) m_pNewFileMap = new CFileMap;
	QUICK_TRYCAT(return m_pNewFileMap->VCreateFolderIn(sPath, sNewFolderName);)
}

IDirectoryTraverser::IIterator* CModuleFile::VIterate(const char* sPath)
{
	if(m_pNewFileMap == 0) m_pNewFileMap = new CFileMap;
	QUICK_TRYCAT(return m_pNewFileMap->VIterate(sPath);)
}

unsigned long CModuleFile::VGetEntryPointCount()
{
	if(m_pNewFileMap == 0) m_pNewFileMap = new CFileMap;
	QUICK_TRYCAT(return m_pNewFileMap->VGetEntryPointCount();)
}

const char* CModuleFile::VGetEntryPoint(unsigned long iID)
{
	if(m_pNewFileMap == 0) m_pNewFileMap = new CFileMap;
	QUICK_TRYCAT(return m_pNewFileMap->VGetEntryPoint(iID);)
}

#undef QUICK_TRYCAT

long CModuleFile::GetSgaOutputVersion()
{
	if(m_eModuleType == MT_DawnOfWar)
	{
		return 2;
	}
	else if(m_eModuleType == MT_CompanyOfHeroesEarly)
	{
		return 4;
	}
	else if(m_eModuleType == MT_CompanyOfHeroes)
	{
		return 4;
	}
	return 2;
}

CModuleFile::CUcsHandler::CUcsHandler()
{
	m_sName = 0;
	m_pHandle = 0;
}

CModuleFile::CUcsHandler::~CUcsHandler() {}

const char* CModuleFile::CUcsHandler::GetFileName() const
{
	return m_sName;
}

const CUcsFile* CModuleFile::CUcsHandler::GetUcsHandle() const
{
	return m_pHandle;
}

CUcsFile* CModuleFile::CUcsHandler::GetUcsHandle()
{
	return m_pHandle;
}

CModuleFile::CFolderHandler::CFolderHandler()
{
	m_iNumber = 0;
	m_sName = 0;
}

const char* CModuleFile::CFolderHandler::GetName() const
{
	return m_sName;
}

long CModuleFile::CFolderHandler::GetNumber() const
{
	return m_iNumber;
}

CModuleFile::CFolderHandler::~CFolderHandler() {}

CModuleFile::CArchiveHandler::CArchiveHandler()
{
	m_iNumber = 0;
	m_sName = 0;
	m_pHandle = 0;
}

const char* CModuleFile::CArchiveHandler::GetFileName() const
{
	return m_sName;
}

CSgaFile* CModuleFile::CArchiveHandler::GetFileHandle() const
{
	return m_pHandle;
}

long CModuleFile::CArchiveHandler::GetNumber() const
{
	return m_iNumber;
}


CModuleFile::CArchiveHandler::~CArchiveHandler() {}

CModuleFile::CRequiredHandler::CRequiredHandler()
{
	m_iNumber = 0;
	m_sName = 0;
	m_pHandle = 0;
}

const char* CModuleFile::CRequiredHandler::GetFileName() const
{
	return m_sName;
}

CModuleFile* CModuleFile::CRequiredHandler::GetModHandle()
{
	return m_pHandle;
}

const CModuleFile* CModuleFile::CRequiredHandler::GetModHandle() const
{
	return m_pHandle;
}

long CModuleFile::CRequiredHandler::GetNumber() const
{
	return m_iNumber;
}

CModuleFile::CRequiredHandler::~CRequiredHandler() {}

CModuleFile::CCompatibleHandler::CCompatibleHandler()
{
	m_iNumber = 0;
	m_sName = 0;
}

const char* CModuleFile::CCompatibleHandler::GetFileName() const
{
	return m_sName;
}

long CModuleFile::CCompatibleHandler::GetNumber() const
{
	return m_iNumber;
}

CModuleFile::CCompatibleHandler::~CCompatibleHandler() {}

CModuleFile::CCohDataSource::CCohDataSource()
{
	m_iNumber = 0;
	m_sToc = 0;
	m_sOption = 0;
	m_sFolder = 0;
	m_bIsLoaded = false;
	m_bCanWriteToFolder = true;
}

CModuleFile::CCohDataSource::~CCohDataSource() {}

void CModuleFile::_Clean()
{
	m_eModuleType = MT_DawnOfWar;
	if(!m_pParentModule)
	{
		if(m_sApplicationPath) free(m_sApplicationPath);
	}
	m_sApplicationPath = 0;
	if(m_sFilename) free(m_sFilename);
	m_sFilename = 0;
	if(m_sScenarioPackRootFolder) free(m_sScenarioPackRootFolder);
	m_sScenarioPackRootFolder = 0;
	if(m_saScenarioPackRootFolder) free(m_saScenarioPackRootFolder);
	m_saScenarioPackRootFolder = 0;

	if(m_sFileMapName) free(m_sFileMapName);
	m_sFileMapName = 0;
	if(m_sUiName) free(m_sUiName);
	m_sUiName = 0;
	if(m_sName) free(m_sName);
	m_sName = 0;
	if(m_sDescription) free(m_sDescription);
	m_sDescription = 0;
	if(m_sDllName) free(m_sDllName);
	m_sDllName = 0;
	if(m_sModFolder) free(m_sModFolder);
	m_sModFolder = 0;
	m_iModVersionMajor = 0;
	m_iModVersionMinor = 0;
	m_iModVersionRevision = 0;
	if(m_sTextureFe) free(m_sTextureFe);
	m_sTextureFe = 0;
	if(m_sTextureIcon) free(m_sTextureIcon);
	m_sTextureIcon = 0;
	if(m_sPatcherUrl) free(m_sPatcherUrl);
	m_sPatcherUrl = 0;
	if(m_sLocFolder) free(m_sLocFolder);
	m_sLocFolder = 0;
	if(m_sScenarioPackFolder) free(m_sScenarioPackFolder);
	m_sScenarioPackFolder = 0;

	if(m_sCohThisModFolder) delete[] m_sCohThisModFolder;
	m_sCohThisModFolder = 0;

	m_iFileMapModNumber = 0;

	_CleanResources();

	m_pParentModule = 0;

	for(std::vector<CFolderHandler*>::iterator itr = m_vFolders.begin(); itr != m_vFolders.end(); ++itr)
	{
		if((**itr).m_sName) free((**itr).m_sName);
		delete *itr;
	}
	m_vFolders.clear();

	for(std::vector<CArchiveHandler*>::iterator itr = m_vArchives.begin(); itr != m_vArchives.end(); ++itr)
	{
		if((**itr).m_sName) free((**itr).m_sName);
		delete *itr;
	}
	m_vArchives.clear();

	for(std::vector<CRequiredHandler*>::iterator itr = m_vRequireds.begin(); itr != m_vRequireds.end(); ++itr)
	{
		if((**itr).m_sName) free((**itr).m_sName);
		delete *itr;
	}
	m_vRequireds.clear();

	for(std::vector<CCompatibleHandler*>::iterator itr = m_vCompatibles.begin(); itr != m_vCompatibles.end(); ++itr)
	{
		if((**itr).m_sName) free((**itr).m_sName);
		delete *itr;
	}
	m_vCompatibles.clear();

	for(std::vector<CCohDataSource*>::iterator itr = m_vDataSources.begin(); itr != m_vDataSources.end(); ++itr)
	{
		if((**itr).m_sToc) free((**itr).m_sToc);
		if((**itr).m_sOption) free((**itr).m_sOption);
		if((**itr).m_sFolder) free((**itr).m_sFolder);

		for(std::vector<CArchiveHandler*>::iterator itr2 = (**itr).m_vArchives.begin(); itr2 != (**itr).m_vArchives.end(); ++itr2)
		{
			if((**itr2).m_sName) free((**itr2).m_sName);
			delete *itr2;
		}
		(**itr).m_vArchives.clear();

		delete *itr;
	}
	m_vDataSources.clear();

	m_vEngines.clear();

	m_bIsFauxModule = false;
}

void CModuleFile::_CleanResources()
{
	if(!m_pParentModule)
	{
		if(m_pNewFileMap) delete m_pNewFileMap;
		m_pNewFileMap = 0;
	}

	for(std::vector<CArchiveHandler*>::iterator itr = m_vArchives.begin(); itr != m_vArchives.end(); ++itr)
	{
		if((**itr).m_pHandle)
		{
			if((**itr).m_pHandle->GetInputStream()) delete (**itr).m_pHandle->GetInputStream();
			delete (**itr).m_pHandle;
		}
		(**itr).m_pHandle = 0;
	}

	for(std::vector<CRequiredHandler*>::iterator itr = m_vRequireds.begin(); itr != m_vRequireds.end(); ++itr)
	{
		if((**itr).m_pHandle) delete (**itr).m_pHandle;
		(**itr).m_pHandle = 0;
	}

	for(std::vector<CUcsHandler*>::iterator itr = m_vLocaleTexts.begin(); itr != m_vLocaleTexts.end(); ++itr)
	{
		if((**itr).m_pHandle) delete (**itr).m_pHandle;
		if((**itr).m_sName) free((**itr).m_sName);
		delete *itr;
	}
	m_vLocaleTexts.clear();

	for(std::vector<CCohDataSource*>::iterator itr = m_vDataSources.begin(); itr != m_vDataSources.end(); ++itr)
	{
		for(std::vector<CArchiveHandler*>::iterator itr2 = (**itr).m_vArchives.begin(); itr2 != (**itr).m_vArchives.end(); ++itr2)
		{
			if((**itr2).m_pHandle)
			{
				if((**itr2).m_pHandle->GetInputStream()) delete (**itr2).m_pHandle->GetInputStream();
				delete (**itr2).m_pHandle;
			}
			(**itr2).m_pHandle = 0;
		}
	}

	for(std::vector<CModuleFile*>::iterator itr = m_vEngines.begin(); itr != m_vEngines.end(); ++itr)
	{
		delete *itr;
	}
	m_vEngines.clear();
}

void CModuleFile::LoadSgaAsMod(const char* sFileName, CALLBACK_ARG)
{
	_Clean();

	m_bIsFauxModule = true;

	if(m_pFSS == 0) m_pFSS = new CFileSystemStore;
	if(m_pNewFileMap == 0) m_pNewFileMap = new CFileMap;

	{ // Set m_sApplicationPath and m_sFilename from sFileName
		m_sApplicationPath = CHECK_MEM(strdup(sFileName));
		char* sSlashLocation = strrchr(m_sApplicationPath, '\\');
		if(sSlashLocation == 0) sSlashLocation = strrchr(m_sApplicationPath, '/');
		++sSlashLocation;
		m_sFilename = CHECK_MEM(strdup(sSlashLocation));
		m_sFileMapName = CHECK_MEM(strdup(sSlashLocation));
		*sSlashLocation = 0;
	}

	CSgaFile *pSga;
	_DoLoadArchive(sFileName, &pSga, 0, m_sFileMapName, THE_CALLBACK);
	
	switch(pSga->GetVersion())
	{
	case 2:
		m_eModuleType = MT_DawnOfWar;
		break;

	case 4:
		m_eModuleType = MT_CompanyOfHeroes;
		break;
	};

	m_sModFolder = strdup("");

	CArchiveHandler* pHandler = new CArchiveHandler;
	pHandler->m_iNumber = 0;
	pHandler->m_pHandle = pSga;
	pHandler->m_sName = strdup(m_sFilename);
	m_vArchives.push_back(pHandler);
}

void CModuleFile::LoadModuleFile(const char* sFileName, CALLBACK_ARG)
{
	_Clean();

	// Set m_sApplicationPath and m_sFilename from sFileName
	m_sApplicationPath = CHECK_MEM(strdup(sFileName));
	char* sSlashLocation = strrchr(m_sApplicationPath, '\\');
	if(sSlashLocation == 0) sSlashLocation = strrchr(m_sApplicationPath, '/');
	++sSlashLocation;
	m_sFilename = CHECK_MEM(strdup(sSlashLocation));
	*sSlashLocation = 0;

	m_sFileMapName = CHECK_MEM(strdup(m_sFilename));
	char* sDotLocation = strrchr(m_sFileMapName, '.');
	if(sDotLocation) *sDotLocation = 0;

	// Open module file and start processing directives
	FILE* fModule = fopen(sFileName, "rb");
	if(fModule == 0) throw new CRainmanException(0, __FILE__, __LINE__, "Unable to open \'%s\' for reading", sFileName);
	CallCallback(THE_CALLBACK, "Parsing file \'%s\'", sFileName);

	CCohDataSource* pCurrentDataSource = 0; // If we're in a CoH data source section, this is the pointer to it
	bool bInGlobal; // Are we in the "global" section
	while(!feof(fModule))
	{
		char* sLine = Util_fgetline(fModule);
		if(sLine == 0)
		{
			fclose(fModule);
			throw new CRainmanException(__FILE__, __LINE__, "Error reading from file");
		}
		char* sCommentBegin = strchr(sLine, ';');
		if(sCommentBegin) *sCommentBegin = 0;
		char* sEqualsChar = strchr(sLine, '=');
		if(sEqualsChar)
		{
			// Key = Value
			char* sKey = sLine;
			char* sValue = sEqualsChar + 1;
			*sEqualsChar = 0;
			Util_TrimWhitespace(&sKey);
			Util_TrimWhitespace(&sValue);
			if(bInGlobal)
			{
				try
				{
					if(stricmp(sKey, "UIName") == 0)
					{
						if(m_sUiName) free(m_sUiName);
						m_sUiName = CHECK_MEM(strdup(sValue));
					}
					else if(stricmp(sKey, "Name") == 0)
					{
						if(m_sName) free(m_sName);
						m_sName = CHECK_MEM(strdup(sValue));
					}
					else if(stricmp(sKey, "Description") == 0)
					{
						if(m_sDescription) free(m_sDescription);
						m_sDescription = CHECK_MEM(strdup(sValue));
					}
					else if(stricmp(sKey, "DllName") == 0)
					{
						if(m_sDllName) free(m_sDllName);
						m_sDllName = CHECK_MEM(strdup(sValue));
					}
					else if(stricmp(sKey, "ModFolder") == 0)
					{
						if(m_sModFolder) free(m_sModFolder);
						m_sModFolder = CHECK_MEM(strdup(sValue));
					}
					else if(stricmp(sKey, "TextureFE") == 0)
					{
						if(m_sTextureFe) free(m_sTextureFe);
						m_sTextureFe = CHECK_MEM(strdup(sValue));
					}
					else if(stricmp(sKey, "TextureIcon") == 0)
					{
						if(m_sTextureIcon) free(m_sTextureIcon);
						m_sTextureIcon = CHECK_MEM(strdup(sValue));
					}
					else if(stricmp(sKey, "PatcherUrl") == 0)
					{
						if(m_sPatcherUrl) free(m_sPatcherUrl);
						m_sPatcherUrl = CHECK_MEM(strdup(sValue));
					}
					else if(stricmp(sKey, "LocFolder") == 0)
					{
						if(m_sLocFolder) free(m_sLocFolder);
						m_sLocFolder = CHECK_MEM(strdup(sValue));
					}
					else if(stricmp(sKey, "ScenarioPackFolder") == 0)
					{
						if(m_sScenarioPackFolder) free(m_sScenarioPackFolder);
						m_sScenarioPackFolder = CHECK_MEM(strdup(sValue));
					}
					else if(stricmp(sKey, "ModVersion") == 0)
					{
						char* sDotChar = strchr(sValue, '.');
						if(sDotChar) *sDotChar = 0;
						m_iModVersionMajor = atol(sValue);
						if(sDotChar)
						{
							sValue = sDotChar + 1;
							sDotChar = strchr(sValue, '.');
							if(sDotChar) *sDotChar = 0;
							m_iModVersionMinor = atol(sValue);
							if(sDotChar)
							{
								sValue = sDotChar + 1;
								m_iModVersionRevision = atol(sValue);
							}
						}
					}
					else if(strnicmp(sKey, "DataFolder.", 11) == 0)
					{
						CFolderHandler* pObject = CHECK_MEM(new CFolderHandler);
						m_vFolders.push_back(pObject);
						pObject->m_sName = CHECK_MEM(strdup(sValue));
						pObject->m_iNumber = atol(sKey + 11);
					}
					else if(strnicmp(sKey, "ArchiveFile.", 12) == 0)
					{
						CArchiveHandler* pObject = CHECK_MEM(new CArchiveHandler);
						m_vArchives.push_back(pObject);
						pObject->m_sName = CHECK_MEM( (char*)malloc(strlen(sValue) + 5) );
						sprintf(pObject->m_sName, "%s.sga", sValue);
						pObject->m_iNumber = atol(sKey + 12);
					}
					else if(strnicmp(sKey, "RequiredMod.", 12) == 0)
					{
						CRequiredHandler* pObject = CHECK_MEM(new CRequiredHandler);
						m_vRequireds.push_back(pObject);
						pObject->m_sName = CHECK_MEM(strdup(sValue));
						pObject->m_iNumber = atol(sKey + 12);
					}
					else if(strnicmp(sKey, "CompatableMod.", 14) == 0)
					{
						CCompatibleHandler* pObject = CHECK_MEM(new CCompatibleHandler);
						m_vCompatibles.push_back(pObject);
						pObject->m_sName = CHECK_MEM(strdup(sValue));
						pObject->m_iNumber = atol(sKey + 12);
					}
				}
				catch(CRainmanException *pE)
				{
					delete[] sLine;
					fclose(fModule);
					throw pE;
				}
			}
			else if(pCurrentDataSource)
			{
				try
				{
					if(stricmp(sKey, "folder") == 0)
					{
						if(pCurrentDataSource->m_sFolder) free(pCurrentDataSource->m_sFolder);
						pCurrentDataSource->m_sFolder = CHECK_MEM(strdup(sValue));
					}
					if(strnicmp(sKey, "archive.", 8) == 0)
					{
						CArchiveHandler* pObject = CHECK_MEM(new CArchiveHandler);
						pCurrentDataSource->m_vArchives.push_back(pObject);
						pObject->m_sName = CHECK_MEM( (char*)malloc(strlen(sValue) + 5) );
						sprintf(pObject->m_sName, "%s.sga", sValue);
						pObject->m_iNumber = atol(sKey + 8);
					}
				}
				catch(CRainmanException *pE)
				{
					delete[] sLine;
					fclose(fModule);
					throw pE;
				}
			}
		}
		else
		{
			char* sLeftBraceChar = strchr(sLine, '[');
			char* sRightBraceChar = sLeftBraceChar ? strchr(sLeftBraceChar, ']') : 0;
			if(sLeftBraceChar && sRightBraceChar)
			{
				// [Section]
				*sRightBraceChar = 0;
				++sLeftBraceChar;
				bInGlobal = false;
				pCurrentDataSource = 0;
				if(stricmp(sLeftBraceChar, "global") == 0)
				{
					bInGlobal = true;
				}
				else
				{
					char* sColonChar = strchr(sLeftBraceChar, ':');
					if(sColonChar)
					{
						try
						{
							pCurrentDataSource = CHECK_MEM(new CCohDataSource);
							m_vDataSources.push_back(pCurrentDataSource);
							*sColonChar = 0;
							pCurrentDataSource->m_sToc = CHECK_MEM(strdup(sLeftBraceChar));
							sLeftBraceChar = sColonChar + 1;
							sColonChar = strchr(sLeftBraceChar, ':');
							if(sColonChar) *sColonChar = 0;
							pCurrentDataSource->m_sOption = CHECK_MEM(strdup(sLeftBraceChar));
							if(sColonChar) pCurrentDataSource->m_iNumber = atol(sColonChar + 1);
						}
						catch(CRainmanException *pE)
						{
							delete[] sLine;
							fclose(fModule);
							throw pE;
						}
					}
				}
			}
		}
		delete[] sLine;
	}
	fclose(fModule);

	// Decide what kind of module file it is
	size_t iHeuristicDow = 0, iHeuristicCoh = 0, iHeuristicCohEarly = 0;
	if(m_sUiName && *m_sUiName) iHeuristicDow += 3;
	if(m_sName && *m_sName) iHeuristicCoh += 1, iHeuristicCohEarly += 1;
	if(m_sDescription && *m_sDescription) iHeuristicCoh += 1, iHeuristicCohEarly += 1, iHeuristicDow += 1;
	if(m_sDllName && *m_sDllName) iHeuristicCoh += 1, iHeuristicCohEarly += 1, iHeuristicDow += 1;
	if(m_sModFolder && *m_sModFolder) iHeuristicCoh += 1, iHeuristicCohEarly += 1, iHeuristicDow += 1;
	if(m_sTextureFe && *m_sTextureFe) iHeuristicDow += 3;
	if(m_sTextureIcon && *m_sTextureIcon) iHeuristicDow += 3;
	if(m_sPatcherUrl && *m_sPatcherUrl) iHeuristicCohEarly += 3;
	if(m_sLocFolder && *m_sLocFolder) iHeuristicCoh += 3;
	if(m_sScenarioPackFolder && *m_sScenarioPackFolder) iHeuristicCoh += 3;
	iHeuristicDow += (m_vFolders.size() * 3);
	iHeuristicDow += (m_vArchives.size() * 3);
	iHeuristicDow += (m_vRequireds.size() * 3);
	iHeuristicDow += (m_vCompatibles.size() * 3);
	iHeuristicCoh += (m_vDataSources.size() * 3);
	if(m_vDataSources.size() == 0) iHeuristicDow += 5, iHeuristicCohEarly += 5;
	if(m_vFolders.size() == 0 && m_vArchives.size() == 0 && m_vRequireds.size() == 0) iHeuristicCoh += 5, iHeuristicCohEarly += 5;

	if(iHeuristicDow == 0 && iHeuristicCoh == 0 && iHeuristicCohEarly == 0)
	{
		_Clean();
		throw new CRainmanException(__FILE__, __LINE__, "File is not a valid .module file");
	}

	if(iHeuristicDow > iHeuristicCoh && iHeuristicDow > iHeuristicCohEarly) m_eModuleType = MT_DawnOfWar;
	else if(iHeuristicCoh > iHeuristicDow && iHeuristicCoh > iHeuristicCohEarly) m_eModuleType = MT_CompanyOfHeroes;
	else if(iHeuristicCohEarly > iHeuristicDow && iHeuristicCohEarly > iHeuristicCoh) m_eModuleType = MT_CompanyOfHeroesEarly;
	else
	{
		_Clean();
		throw new CRainmanException(__FILE__, __LINE__, "Unable to determine what kind of .module file this is");
	}

	if(m_eModuleType == MT_CompanyOfHeroes)
	{
		m_sCohThisModFolder = new char[strlen(m_sApplicationPath) + strlen(m_sModFolder) + 1];
		sprintf(m_sCohThisModFolder, "%s%s", m_sApplicationPath, m_sModFolder);
		char* sSlashLoc = strrchr(m_sCohThisModFolder, '\\');
		++sSlashLoc;
		*sSlashLoc = 0;
	}
}

char* CModuleFile::_DawnOfWarRemoveDynamics(const char* sStr, const char* sAlsoAppend)
{
	size_t iLocaleCount = 0; // %LOCALE% expands to a value longer than itself, so we need to know how many of these there are
	{
		const char* sLocalePosition;
		const char* sSearchFrom = sStr;
		while(sLocalePosition = strstr(sSearchFrom, "%LOCALE%")) ++iLocaleCount, sSearchFrom = sLocalePosition + 1;
	}

	size_t iNewLength = strlen(sStr) + (strlen(m_sLocale) * iLocaleCount) + (sAlsoAppend ? strlen(sAlsoAppend) : 0) + 1;
	char* sNewString = new char[iNewLength];
	if(sNewString == 0) return 0;
	char* sNewPosition = sNewString;

	while(*sStr)
	{
		if(*sStr == '%')
		{
			if(strncmp(sStr, "%LOCALE%", 8) == 0)
			{
				strcpy(sNewPosition, "Locale\\");
				strcat(sNewPosition, m_sLocale);
				sNewPosition += 7 + strlen(m_sLocale);
				sStr += 8;
				continue;
			}
			if(strncmp(sStr, "%TEXTURE-LEVEL%", 15) == 0)
			{
				strcpy(sNewPosition, "Full");
				sNewPosition += 4;
				sStr += 15;
				continue;
			}
			if(strncmp(sStr, "%SOUND-LEVEL%", 13) == 0)
			{
				strcpy(sNewPosition, "Full");
				sNewPosition += 4;
				sStr += 13;
				continue;
			}
			if(strncmp(sStr, "%MODEL-LEVEL%", 13) == 0)
			{
				strcpy(sNewPosition, "High");
				sNewPosition += 4;
				sStr += 13;
				continue;
			}
		}
		*sNewPosition = *sStr;

		++sStr;
		++sNewPosition;
	}
	*sNewPosition = 0;
	if(sAlsoAppend) strcat(sNewString, sAlsoAppend);
	return sNewString;
}

const char* CModuleFile::GetFileMapName() const
{
	return m_sFileMapName;
}

const char* CModuleFile::GetFileMapName()
{
	return m_sFileMapName;
}

void CModuleFile::_DoLoadDataGeneric(CALLBACK_ARG)
{
	char* sDataGenericValue = 0;
	// Get folder from pipeline.ini
	char* sPipelineFile = new char[strlen(m_sApplicationPath) + 16];
	sprintf(sPipelineFile, "%spipeline.ini", m_sApplicationPath);
	FILE* fModule = fopen(sPipelineFile, "rb");
	if(fModule == 0)
	{
		delete[] sPipelineFile;
		return;
	}
	CallCallback(THE_CALLBACK, "Parsing file \'pipeline.ini\'");
	delete[] sPipelineFile;

	char* sSectionName = new char[16 + strlen(m_sModFolder)];
	sprintf(sSectionName, "project:%s", m_sModFolder);

	bool bInSection; // Are we in the "project:xxx" section
	while(!feof(fModule))
	{
		char* sLine = Util_fgetline(fModule);
		if(sLine == 0)
		{
			fclose(fModule);
			delete[] sSectionName;
			if(sDataGenericValue) free(sDataGenericValue);
			throw new CRainmanException(__FILE__, __LINE__, "Error reading from file");
		}
		char* sCommentBegin = strchr(sLine, ';');
		if(sCommentBegin) *sCommentBegin = 0;
		char* sEqualsChar = strchr(sLine, '=');
		if(sEqualsChar)
		{
			// Key = Value
			char* sKey = sLine;
			char* sValue = sEqualsChar + 1;
			*sEqualsChar = 0;
			Util_TrimWhitespace(&sKey);
			Util_TrimWhitespace(&sValue);
			if(bInSection)
			{
				try
				{
					if(stricmp(sKey, "DataGeneric") == 0)
					{
						if(sDataGenericValue) free(sDataGenericValue);
						sDataGenericValue = CHECK_MEM(strdup(sValue));
					}
				}
				catch(CRainmanException *pE)
				{
					delete[] sLine;
					delete[] sSectionName;
					if(sDataGenericValue) free(sDataGenericValue);
					fclose(fModule);
					throw pE;
				}
			}
		}
		else
		{
			char* sLeftBraceChar = strchr(sLine, '[');
			char* sRightBraceChar = sLeftBraceChar ? strchr(sLeftBraceChar, ']') : 0;
			if(sLeftBraceChar && sRightBraceChar)
			{
				// [Section]
				*sRightBraceChar = 0;
				++sLeftBraceChar;
				bInSection = false;
				if(stricmp(sLeftBraceChar, sSectionName) == 0)
				{
					bInSection = true;
				}
			}
		}
		delete[] sLine;
	}
	delete[] sSectionName;
	fclose(fModule);

	if(sDataGenericValue)
	{
		char* sDataGenericFullPath = new char[strlen(m_sApplicationPath) + strlen(sDataGenericValue) + 1];
		sprintf(sDataGenericFullPath, "%s%s", m_sApplicationPath, sDataGenericValue);
		_DoLoadFolder(sDataGenericFullPath, (m_pParentModule == 0), 99, "Generic", "DataGeneric", THE_CALLBACK);
		delete[] sDataGenericFullPath;
		free(sDataGenericValue);
	}
}

void CModuleFile::_DoLoadFolder(const char* sFullPath, bool bIsDefaultWrite, unsigned short iNum, const char* sTOC, const char *sUiName, CALLBACK_ARG, bool* bIsWritable)
{
	if(bIsWritable != 0) *bIsWritable = false;
	// Get source name
	const char* sSlashChar;
	if(sUiName)
	{
		sSlashChar = sUiName;
	}
	else
	{
		sSlashChar = strrchr(sFullPath, '\\');
		if(!sSlashChar) sSlashChar = strrchr(sFullPath, '/');
		++sSlashChar;
	}

	bool bIsThisMod = true;
	char* sActualModName = 0;
	if(m_eModuleType == MT_CompanyOfHeroes)
	{
		if(strnicmp(m_sCohThisModFolder, sFullPath, strlen(m_sCohThisModFolder)) != 0)
		{
			bIsThisMod = false;
			sActualModName = strdup(sFullPath + strlen(m_sApplicationPath));
			char* sSlash = strrchr(sActualModName, '\\');
			if(sSlash)
				*sSlash = 0;
		}
	}

	// Add
	IDirectoryTraverser::IIterator *pDirItr = 0;
	try
	{
		CallCallback(THE_CALLBACK, "Loading data folder \'%s\' for mod \'%s\'", sSlashChar, m_sFileMapName);
		pDirItr = m_pFSS->VIterate(sFullPath);
		if(pDirItr)
		{
			void* pSrc;
			if(bIsThisMod)
			{
				pSrc = m_pNewFileMap->RegisterSource(m_iFileMapModNumber, false, iNum, GetFileMapName(), sSlashChar, m_pFSS, m_pFSS, m_pParentModule ? false : true, bIsDefaultWrite);
				if(bIsWritable != 0) *bIsWritable = m_pParentModule ? false : true;
			}
			else
				pSrc = m_pNewFileMap->RegisterSource(15000, false, iNum, sActualModName, sSlashChar, m_pFSS, m_pFSS, false, false);

			m_pNewFileMap->MapIterator(pSrc, sTOC, pDirItr);
		}
	}
	catch(CRainmanException *pE) {pE->destroy();}
	if(pDirItr) delete pDirItr;
	if(sActualModName) free(sActualModName);
}

void CModuleFile::_DoLoadArchive(const char* sFullPath, CSgaFile** ppSga, unsigned short iNum, const char *sUiName, CALLBACK_ARG)
{
	// Get source name
	const char* sSlashChar;
	if(sUiName)
	{
		sSlashChar = sUiName;
	}
	else
	{
		sSlashChar = strrchr(sFullPath, '\\');
		if(!sSlashChar) sSlashChar = strrchr(sFullPath, '/');
		++sSlashChar;
	}

	CSgaFile* pSga = CHECK_MEM(new CSgaFile);
	IFileStore::IStream *pInputStream = 0;
	IDirectoryTraverser::IIterator *pDirItr = 0;
	try
	{
		pInputStream = m_pFSS->VOpenStream(sFullPath);
	}
	catch(CRainmanException *pE)
	{
		pE->destroy();
		delete pSga;
		pSga = 0;
		return;
	}

	bool bIsThisMod = true;
	char* sActualModName = 0;
	if(m_eModuleType == MT_CompanyOfHeroes)
	{
		if(strnicmp(m_sCohThisModFolder, sFullPath, strlen(m_sCohThisModFolder)) != 0)
		{
			if(m_saScenarioPackRootFolder && strnicmp(m_saScenarioPackRootFolder, sFullPath, strlen(m_saScenarioPackRootFolder)) == 0)
			{
				bIsThisMod = false;
				sActualModName = strdup(sFullPath + strlen(m_saScenarioPackRootFolder) + 1);
				char* sSlash = strrchr(sActualModName, '\\');
				if(sSlash)
					*sSlash = 0;
			}
			else
			{
				bIsThisMod = false;
				sActualModName = strdup(sFullPath + strlen(m_sApplicationPath));
				char* sSlash = strrchr(sActualModName, '\\');
				if(sSlash)
					*sSlash = 0;
				sSlash = strrchr(sActualModName, '\\');
				if(sSlash)
					*sSlash = 0;
			}
		}
	}

	try
	{
		CallCallback(THE_CALLBACK, "Loading data archive \'%s\' for mod \'%s\'", sSlashChar, m_sFileMapName);
		pSga->Load(pInputStream, m_pFSS->VGetLastWriteTime(sFullPath));
		pSga->VInit(pInputStream);
		pDirItr = pSga->VIterate(pSga->VGetEntryPoint(0));
		if(pDirItr)
		{
			void* pSrc;
			if(bIsThisMod)
				pSrc = m_pNewFileMap->RegisterSource(m_iFileMapModNumber, true, iNum, GetFileMapName(), sSlashChar, pSga, pSga, false, false);
			else
				pSrc = m_pNewFileMap->RegisterSource(15000, true, iNum, sActualModName, sSlashChar, pSga, pSga, false, false);
			m_pNewFileMap->MapSga(pSrc, pSga);
		}
		delete pDirItr;
	}
	catch(CRainmanException *pE)
	{
		pE = new CRainmanException(pE, __FILE__, __LINE__, "(rethrow for \'%s\')", sFullPath);
		delete pDirItr;
		delete pInputStream;
		delete pSga;
		if(sActualModName) free(sActualModName);
		throw pE;
	}
	*ppSga = pSga;
	if(sActualModName) free(sActualModName);
}

bool CModuleFile::_IsToBeLoaded(CCohDataSource* pDataSource)
{
	return( (strcmp(pDataSource->m_sOption, "common") == 0) || (strcmp(pDataSource->m_sOption, "sound_high") == 0)
		|| (strcmp(pDataSource->m_sOption, "art_high") == 0) || (stricmp(pDataSource->m_sOption, m_sLocale) == 0) );
}

void CModuleFile_ArchiveForEach(IDirectoryTraverser::IIterator* pItr, void* pModuleFile)
{
	CModuleFile* pThis = (CModuleFile*)pModuleFile;

	CModuleFile::CArchiveHandler* pArchEntry = new CModuleFile::CArchiveHandler;
	pArchEntry->m_iNumber = -7291;
	pArchEntry->m_sName = strdup(pItr->VGetName());
	pThis->m_vArchives.push_back(pArchEntry);

	pThis->_DoLoadArchive(pItr->VGetFullPath(), &pArchEntry->m_pHandle, 15000, pArchEntry->m_sName);
}

void CModuleFile_ArchiveForEachNoErrors(IDirectoryTraverser::IIterator* pItr, void* pModuleFile)
{
	CModuleFile* pThis = (CModuleFile*)pModuleFile;

	CSgaFile* pFile = 0;
	char* s = strdup(pItr->VGetName());
	try
	{
		pThis->_DoLoadArchive(pItr->VGetFullPath(), &pFile, 15000, s);
	}
	catch(CRainmanException* pE)
	{
		free(s);
		pE->destroy();
		return;
	}

	CModuleFile::CArchiveHandler* pArchEntry = new CModuleFile::CArchiveHandler;
	pArchEntry->m_iNumber = -7291;
	pArchEntry->m_sName = s;
	pArchEntry->m_pHandle = pFile;
	pThis->m_vArchives.push_back(pArchEntry);
}

void CModuleFile_UcsForEach(IDirectoryTraverser::IIterator* pItr, void* pModuleFile)
{
	CModuleFile* pThis = (CModuleFile*)pModuleFile;
	const char* sName = pItr->VGetName();
	const char* sDot = strrchr(sName, '.');
	if(sDot && ((stricmp(sDot, ".ucs") == 0) || (stricmp(sDot, ".dat") == 0)))
	{
		CModuleFile::CUcsHandler* pUcsEntry = new CModuleFile::CUcsHandler;
		pUcsEntry->m_sName = strdup(sName);
		pUcsEntry->m_pHandle = new CUcsFile;

		IFileStore::IStream* pStream = 0;
		try
		{
			pStream = pItr->VOpenFile();
			if(stricmp(sDot, ".ucs") == 0)
				pUcsEntry->m_pHandle->Load(pStream);
			else
				pUcsEntry->m_pHandle->LoadDat(pStream);
		}
		catch(CRainmanException *pE)
		{
			delete pStream;
			delete pUcsEntry->m_pHandle;
			free(pUcsEntry->m_sName);
			delete pUcsEntry;
			throw new CRainmanException(pE, __FILE__, __LINE__, "Error loading UCS file \'%s\'", sName);
		}
		delete pStream;

		pThis->m_vLocaleTexts.push_back(pUcsEntry);
	}
}

void CModuleFile::ReloadResources(unsigned long iReloadWhat, unsigned long iReloadWhatRequiredMods, unsigned long iReloadWhatEngines, CALLBACK_ARG)
{
	_CleanResources();

	if(m_pFSS == 0) m_pFSS = new CFileSystemStore;
	if(m_pNewFileMap == 0) m_pNewFileMap = new CFileMap;

	if(iReloadWhat & RR_LocalisedText)
	{
		// m_sLocFolder
		char* sUcsPath;
		if(m_sLocFolder && *m_sLocFolder)
		{
			sUcsPath = new char[strlen(m_sApplicationPath) + strlen(m_sLocFolder) + strlen(m_sLocale) + 2];
			sprintf(sUcsPath, "%s%s\\%s", m_sApplicationPath, m_sLocFolder, m_sLocale);
		}
		else
		{
			sUcsPath = new char[strlen(m_sApplicationPath) + strlen(m_sModFolder) + strlen(m_sLocale) + 10];
			sprintf(sUcsPath, "%s%s\\Locale\\%s", m_sApplicationPath, m_sModFolder, m_sLocale);
		}

		IDirectoryTraverser::IIterator* pItr = 0;
		try
		{
			pItr = m_pFSS->VIterate(sUcsPath);
		}
		catch(CRainmanException *pE)
		{
			pE->destroy();
		}
		if(pItr != 0)
		{
			try
			{
				CallCallback(THE_CALLBACK, "Loading UCS files for mod \'%s\'", m_sFileMapName);
				Util_ForEach(pItr, CModuleFile_UcsForEach, (void*)this, false);
			}
			catch(CRainmanException *pE)
			{
				PAUSE_THROW(pE, __FILE__, __LINE__, "Error loading UCS from \'%s\'", sUcsPath);
				delete[] sUcsPath;
				delete pItr;
				UNPAUSE_THROW;
			}
			delete pItr;
		}
		delete[] sUcsPath;
	}
	if(iReloadWhat & RR_DataFolders)
	{
		if(m_eModuleType == MT_DawnOfWar)
		{
			char* sOutFolder = 0;
			for(std::vector<CFolderHandler*>::iterator itr = m_vFolders.begin(); itr != m_vFolders.end(); ++itr)
			{
				if(stricmp((**itr).m_sName, "data") == 0)
				{
					sOutFolder = (**itr).m_sName;
					break;
				}
			}

			if(!sOutFolder)
			{
				for(std::vector<CFolderHandler*>::iterator itr = m_vFolders.begin(); itr != m_vFolders.end(); ++itr)
				{
					if(strchr((**itr).m_sName, '%') == 0)
					{
						sOutFolder = (**itr).m_sName;
						break;
					}
				}
			}

			for(std::vector<CFolderHandler*>::iterator itr = m_vFolders.begin(); itr != m_vFolders.end(); ++itr)
			{
				char* sWithoutDynamics = _DawnOfWarRemoveDynamics((**itr).m_sName);
				char* sFolderPath = new char[strlen(m_sApplicationPath) + strlen(m_sModFolder) + strlen(sWithoutDynamics) + 2];
				sprintf(sFolderPath, "%s%s\\%s", m_sApplicationPath, m_sModFolder, sWithoutDynamics);
				_DoLoadFolder(sFolderPath, (sOutFolder == (**itr).m_sName), 15000 + (**itr).m_iNumber, "Data", sWithoutDynamics, THE_CALLBACK);
				delete[] sFolderPath;
				delete[] sWithoutDynamics;
			}
		}
		else if(m_eModuleType == MT_CompanyOfHeroesEarly)
		{
			char* sFolderPath = new char[strlen(m_sApplicationPath) + strlen(m_sModFolder) + 10];
			sprintf(sFolderPath, "%s%s\\Data", m_sApplicationPath, m_sModFolder);
			_DoLoadFolder(sFolderPath, true, 15000, "Data", 0, THE_CALLBACK);
			sprintf(sFolderPath, "%s%s\\Movies", m_sApplicationPath, m_sModFolder);
			_DoLoadFolder(sFolderPath, true, 15000, "Movies", 0, THE_CALLBACK);
			delete[] sFolderPath;
		}
		else if(m_eModuleType == MT_CompanyOfHeroes)
		{
			for(std::vector<CCohDataSource*>::iterator itr = m_vDataSources.begin(); itr != m_vDataSources.end(); ++itr)
			{
				(**itr).m_bIsLoaded = _IsToBeLoaded(*itr);
				(**itr).m_bCanWriteToFolder = false;
				if(_IsToBeLoaded(*itr) && (**itr).m_sFolder && *(**itr).m_sFolder)
				{
					char* sFolderPath = new char[strlen(m_sApplicationPath) + strlen((**itr).m_sFolder) + 1];
					sprintf(sFolderPath, "%s%s", m_sApplicationPath, (**itr).m_sFolder);
					bool bIsWrite = ( (strcmp((**itr).m_sOption, "common") == 0) && ((**itr).m_iNumber < 2) );
					_DoLoadFolder(sFolderPath, bIsWrite, 15000 + (**itr).m_iNumber, (**itr).m_sToc, (**itr).m_sFolder, THE_CALLBACK, &(**itr).m_bCanWriteToFolder);
					delete[] sFolderPath;
				}
			}
		}
	}
	if(iReloadWhat & RR_DataArchives)
	{
		if(m_eModuleType == MT_DawnOfWar)
		{
			for(std::vector<CArchiveHandler*>::iterator itr = m_vArchives.begin(); itr != m_vArchives.end(); ++itr)
			{
				char* sWithoutDynamics = _DawnOfWarRemoveDynamics((**itr).m_sName);
				char* sArchivePath = new char[strlen(m_sApplicationPath) + strlen(m_sModFolder) + strlen(sWithoutDynamics) + 2];
				sprintf(sArchivePath, "%s%s\\%s", m_sApplicationPath, m_sModFolder, sWithoutDynamics);
				_DoLoadArchive(sArchivePath, &(**itr).m_pHandle, 15000 + (**itr).m_iNumber ,(**itr).m_sName, THE_CALLBACK);
				delete[] sArchivePath;
				delete[] sWithoutDynamics;
			}
		}
		else if(m_eModuleType == MT_CompanyOfHeroesEarly)
		{
			char* sArchivesPath = new char[strlen(m_sApplicationPath) + strlen(m_sModFolder) + 10];
			sprintf(sArchivesPath, "%s%s\\Archives", m_sApplicationPath, m_sModFolder);
			IDirectoryTraverser::IIterator* pItr = 0;
			try
			{
				pItr = m_pFSS->VIterate(sArchivesPath);
				CallCallback(THE_CALLBACK, "Loading data archives for mod \'%s\'", m_sFileMapName);
				Util_ForEach(pItr, CModuleFile_ArchiveForEach, (void*)this, false);
			}
			catch(CRainmanException *pE)
			{
				PAUSE_THROW(pE, __FILE__, __LINE__, "Error loading archives from \'%s\'", sArchivesPath);
				delete[] sArchivesPath;
				delete pItr;
				UNPAUSE_THROW;
			}
			delete pItr;
			delete[] sArchivesPath;
		}
		else if(m_eModuleType == MT_CompanyOfHeroes)
		{
			for(std::vector<CCohDataSource*>::iterator itr2 = m_vDataSources.begin(); itr2 != m_vDataSources.end(); ++itr2)
			{
				(**itr2).m_bIsLoaded = _IsToBeLoaded(*itr2);
				if(_IsToBeLoaded(*itr2))
				{
					for(std::vector<CArchiveHandler*>::iterator itr = (**itr2).m_vArchives.begin(); itr != (**itr2).m_vArchives.end(); ++itr)
					{
						char* sArchivePath = new char[strlen(m_sApplicationPath) + strlen((**itr).m_sName) + 1];
						sprintf(sArchivePath, "%s%s", m_sApplicationPath, (**itr).m_sName);
						_DoLoadArchive(sArchivePath, &(**itr).m_pHandle, 15000 + (500 * (**itr2).m_iNumber) + (**itr).m_iNumber ,(**itr).m_sName, THE_CALLBACK);
						delete[] sArchivePath;
					}
				}
			}
		}
	}
	if(iReloadWhat & RR_MapArchives)
	{
		if(m_sScenarioPackRootFolder && *m_sScenarioPackRootFolder && m_sScenarioPackFolder && *m_sScenarioPackFolder)
		{
			wchar_t* sArchivesPath = new wchar_t[wcslen(m_sScenarioPackRootFolder) + strlen(m_sScenarioPackFolder) + 2];
			swprintf(sArchivesPath, L"%s\\%S", m_sScenarioPackRootFolder, m_sScenarioPackFolder);
			IDirectoryTraverser::IIterator* pItr = 0;
			try
			{
				pItr = m_pFSS->IterateW(sArchivesPath);
			}
			catch(CRainmanException *pE)
			{
				pE->destroy();
				pItr = 0;
			}
			if(pItr)
			{
				try
				{
					CallCallback(THE_CALLBACK, "Loading map archives for mod \'%s\'", m_sFileMapName);
					Util_ForEach(pItr, CModuleFile_ArchiveForEachNoErrors, (void*)this, false);
				}
				catch(CRainmanException *pE)
				{
					PAUSE_THROW(pE, __FILE__, __LINE__, "Error loading map archives from \'%S\'", sArchivesPath);
					delete pItr;
					delete[] sArchivesPath;
					UNPAUSE_THROW;
				}
				delete pItr;
			}
			delete[] sArchivesPath;
		}
	}
	if(iReloadWhat & RR_RequiredMods)
	{
		if(m_eModuleType == MT_DawnOfWar)
		{
			for(std::vector<CRequiredHandler*>::iterator itr = m_vRequireds.begin(); itr != m_vRequireds.end(); ++itr)
			{
				CModuleFile *pReq = CHECK_MEM(new CModuleFile);
				free(pReq->m_sLocale);
				pReq->m_sLocale = m_sLocale;

				char* sFilePath = new char[strlen(m_sApplicationPath) + strlen((**itr).m_sName) + 8];
				sprintf(sFilePath, "%s%s.module", m_sApplicationPath, (**itr).m_sName);
				pReq->LoadModuleFile(sFilePath);
				delete[] sFilePath;

				pReq->m_pParentModule = (CModuleFile*)this;
				pReq->m_pNewFileMap = m_pNewFileMap;
				pReq->m_pFSS = m_pFSS;
				free(pReq->m_sApplicationPath);
				pReq->m_sApplicationPath = m_sApplicationPath;
				pReq->m_iFileMapModNumber = (unsigned short)(15000 + (**itr).m_iNumber);

				(**itr).m_pHandle = pReq;

				pReq->ReloadResources(iReloadWhatRequiredMods & (~RR_Engines) & (~RR_RequiredMods), 0, 0, THE_CALLBACK);
			}
		}
		// Only for MT_DawnOfWar, doesn't apply to MT_CompanyOfHeroesEarly or MT_CompanyOfHeroes
	}
	if(iReloadWhat & RR_Engines)
	{
		if(m_eModuleType == MT_DawnOfWar)
		{
			CModuleFile *pEngine = CHECK_MEM(new CModuleFile);

			pEngine->m_eModuleType = MT_DawnOfWar;
			pEngine->m_sUiName = strdup("(Engine)");
			pEngine->m_sFileMapName = strdup("(Engine)");
			pEngine->m_sDescription = strdup("(Engine)");
			pEngine->m_sModFolder = strdup("Engine");

			free(pEngine->m_sLocale);
			pEngine->m_sLocale = m_sLocale;
			pEngine->m_pParentModule = (CModuleFile*)this;
			pEngine->m_pNewFileMap = m_pNewFileMap;
			pEngine->m_pFSS = m_pFSS;
			pEngine->m_sApplicationPath = m_sApplicationPath;
			pEngine->m_iFileMapModNumber = (unsigned short)(30001);

			{
				CFolderHandler* pObject = CHECK_MEM(new CFolderHandler);
				pEngine->m_vFolders.push_back(pObject);
				pObject->m_sName = CHECK_MEM(strdup("Data"));
				pObject->m_iNumber = 1;
			}
			{
				CArchiveHandler* pObject = CHECK_MEM(new CArchiveHandler);
				pEngine->m_vArchives.push_back(pObject);
				pObject->m_sName = CHECK_MEM(strdup("Engine.sga"));
				pObject->m_iNumber = 1;
			}

			m_vEngines.push_back(pEngine);

			pEngine->ReloadResources(iReloadWhatEngines & (~RR_Engines), 0, 0, THE_CALLBACK);
		}
		else if(m_eModuleType == MT_CompanyOfHeroesEarly)
		{
			_DoLoadCohEngine("Engine", "(Engine)", iReloadWhatEngines, 30001, THE_CALLBACK);
			_DoLoadCohEngine("RelicOnline", "(Relic Online)", iReloadWhatEngines, 30000, THE_CALLBACK);
		}
		else if(m_eModuleType == MT_CompanyOfHeroes)
		{
			_DoLoadCohEngine("Engine", "(Engine)", iReloadWhatEngines, 30001, THE_CALLBACK);
			_DoLoadCohEngine("RelicOnline", "(Relic Online)", iReloadWhatEngines, 30000, THE_CALLBACK);
		}
	}
	if(iReloadWhat && RR_DataGeneric)
	{
		_DoLoadDataGeneric(THE_CALLBACK);
	}
}

const char* CModuleFile::GetApplicationPath() const
{
	return m_sApplicationPath;
}

void CModuleFile::_DoLoadCohEngine(const char* sFolder, const char* sUiName, unsigned long iLoadWhat, unsigned short iNum, CALLBACK_ARG)
{
	CModuleFile *pEngine = CHECK_MEM(new CModuleFile);

	pEngine->m_eModuleType = MT_CompanyOfHeroesEarly;
	pEngine->m_sUiName = strdup(sUiName);
	pEngine->m_sDescription = strdup(sUiName);
	pEngine->m_sFileMapName = strdup(sUiName);
	pEngine->m_sModFolder = strdup(sFolder);

	free(pEngine->m_sLocale);
	pEngine->m_sLocale = m_sLocale;
	pEngine->m_pParentModule = (CModuleFile*)this;
	pEngine->m_pNewFileMap = m_pNewFileMap;
	pEngine->m_pFSS = m_pFSS;
	pEngine->m_sApplicationPath = m_sApplicationPath;
	pEngine->m_iFileMapModNumber = iNum;

	m_vEngines.push_back(pEngine);

	pEngine->ReloadResources(iLoadWhat & (~RR_Engines), 0, 0, THE_CALLBACK);
}

void CModuleFile::NewUCS(const char* sName, CUcsFile* pUcs)
{
	CUcsHandler* pHandler = new CUcsHandler;
	pHandler->m_sName = strdup(sName);
	pHandler->m_pHandle = pUcs;

	m_vLocaleTexts.push_back(pHandler);
}

const wchar_t* CModuleFile::ResolveUCS(const char* sDollarString)
{
	if(!CUcsFile::IsDollarString(sDollarString)) throw new CRainmanException(__FILE__, __LINE__, "sDollarString must be a dollar string");
	try
	{
		return ResolveUCS((unsigned long)atol(sDollarString + 1));
	}
	CATCH_THROW("Cannot resolve value as integer")
}

const wchar_t* CModuleFile::ResolveUCS(const wchar_t* sDollarString)
{
	if(!CUcsFile::IsDollarString(sDollarString)) throw new CRainmanException(__FILE__, __LINE__, "sDollarString must be a dollar string");
	try
	{
		return ResolveUCS(wcstoul(sDollarString + 1, 0, 10));
	}
	CATCH_THROW("Cannot resolve value as integer")
}

const wchar_t* CModuleFile::ResolveUCS(unsigned long iStringID)
{
	const wchar_t *t;

	for(std::vector<CUcsHandler*>::iterator itr = m_vLocaleTexts.begin(); itr != m_vLocaleTexts.end(); ++itr)
	{
		if( (*itr) && (**itr).m_pHandle && (t = (**itr).m_pHandle->ResolveStringID(iStringID)) ) return t;
	}

	for(std::vector<CRequiredHandler*>::iterator itr = m_vRequireds.begin(); itr != m_vRequireds.end(); ++itr)
	{
		if( (*itr) && (**itr).m_pHandle && (t = (**itr).m_pHandle->ResolveUCS(iStringID)) ) return t;
	}

	for(std::vector<CModuleFile*>::iterator itr = m_vEngines.begin(); itr != m_vEngines.end(); ++itr)
	{
		if( (*itr) && (t = (*itr)->ResolveUCS(iStringID)) ) return t;
	}

	return 0;
}

size_t CModuleFile::GetUcsCount() const
{
	return m_vLocaleTexts.size();
}

CModuleFile::CUcsHandler* CModuleFile::GetUcs(size_t iId)
{
	if(iId < 0 || iId >= GetUcsCount()) throw new CRainmanException(0, __FILE__, __LINE__, "ID %lu is outside of range 0-%lu", (unsigned long) iId, (unsigned long)GetUcsCount());
	return m_vLocaleTexts[iId];
}

const CModuleFile::CUcsHandler* CModuleFile::GetUcs(size_t iId) const
{
	if(iId < 0 || iId >= GetUcsCount()) throw new CRainmanException(0, __FILE__, __LINE__, "ID %lu is outside of range 0-%lu", (unsigned long) iId, (unsigned long)GetUcsCount());
	return m_vLocaleTexts[iId];
}

void CModuleFile::DeleteUcs(size_t iId)
{
	if(iId < 0 || iId >= GetUcsCount()) throw new CRainmanException(0, __FILE__, __LINE__, "ID %lu is outside of range 0-%lu", (unsigned long) iId, (unsigned long)GetUcsCount());
	m_vLocaleTexts.erase(m_vLocaleTexts.begin() + iId);
}

size_t CModuleFile::GetRequiredCount() const
{
	return m_vRequireds.size();
}

CModuleFile::CRequiredHandler* CModuleFile::GetRequired(size_t iId)
{
	if(iId < 0 || iId >= GetRequiredCount()) throw new CRainmanException(0, __FILE__, __LINE__, "ID %lu is outside of range 0-%lu", (unsigned long) iId, (unsigned long)GetRequiredCount());
	return m_vRequireds[iId];
}

const CModuleFile::CRequiredHandler* CModuleFile::GetRequired(size_t iId) const
{
	if(iId < 0 || iId >= GetRequiredCount()) throw new CRainmanException(0, __FILE__, __LINE__, "ID %lu is outside of range 0-%lu", (unsigned long) iId, (unsigned long)GetRequiredCount());
	return m_vRequireds[iId];
}

size_t CModuleFile::GetFolderCount()
{
	return m_vFolders.size();
}

CModuleFile::CFolderHandler* CModuleFile::GetFolder(size_t iId)
{
	if(iId < 0 || iId >= GetFolderCount()) throw new CRainmanException(0, __FILE__, __LINE__, "ID %lu is outside of range 0-%lu", (unsigned long) iId, (unsigned long)GetFolderCount());
	return m_vFolders[iId];
}

size_t CModuleFile::GetArchiveCount()
{
	return m_vArchives.size();
}

CModuleFile::CArchiveHandler* CModuleFile::GetArchive(size_t iId)
{
	if(iId < 0 || iId >= GetArchiveCount()) throw new CRainmanException(0, __FILE__, __LINE__, "ID %lu is outside of range 0-%lu", (unsigned long) iId, (unsigned long)GetArchiveCount());
	return m_vArchives[iId];
}

size_t CModuleFile::GetArchiveFullPath(size_t iId, char* sOutput)
{
	if(iId < 0 || iId >= GetArchiveCount()) throw new CRainmanException(0, __FILE__, __LINE__, "ID %lu is outside of range 0-%lu", (unsigned long) iId, (unsigned long)GetArchiveCount());
	CModuleFile::CArchiveHandler* pHandler = m_vArchives[iId];

	size_t iLen;
	if(m_eModuleType == MT_DawnOfWar)
	{
		char* sWithoutDynamics = _DawnOfWarRemoveDynamics(pHandler->m_sName);
		iLen = strlen(m_sApplicationPath) + strlen(m_sModFolder) + strlen(sWithoutDynamics) + 2;
		if(sOutput)
			sprintf(sOutput, "%s%s\\%s", m_sApplicationPath, m_sModFolder, sWithoutDynamics);
		delete[] sWithoutDynamics;
	}
	else if(m_eModuleType == MT_CompanyOfHeroesEarly)
	{
		iLen = strlen(m_sApplicationPath) + strlen(m_sModFolder) + strlen(pHandler->m_sName) + 11;
		if(sOutput)
			sprintf(sOutput, "%s%s\\Archives\\%s", m_sApplicationPath, m_sModFolder, pHandler->m_sName);
	}
	else if(m_eModuleType == MT_CompanyOfHeroes)
	{
		iLen = strlen(m_sApplicationPath) + strlen(pHandler->m_sName) + 1;
		if(sOutput)
			sprintf(sOutput, "%s%s", m_sApplicationPath, pHandler->m_sName);
	}
	else
	{
		iLen = 1;
		if(sOutput)
			*sOutput = 0;
	}
	return iLen;
}

size_t CModuleFile::GetCompatibleCount()
{
	return m_vCompatibles.size();
}

CModuleFile::CCompatibleHandler* CModuleFile::GetCompatible(size_t iId)
{
	if(iId < 0 || iId >= GetCompatibleCount()) throw new CRainmanException(0, __FILE__, __LINE__, "ID %lu is outside of range 0-%lu", (unsigned long) iId, (unsigned long)GetCompatibleCount());
	return m_vCompatibles[iId];
}

size_t CModuleFile::GetDataSourceCount()
{
	return m_vDataSources.size();
}

CModuleFile::CCohDataSource* CModuleFile::GetDataSource(size_t iId)
{
	if(iId < 0 || iId >= GetDataSourceCount()) throw new CRainmanException(0, __FILE__, __LINE__, "ID %lu is outside of range 0-%lu", (unsigned long) iId, (unsigned long)GetDataSourceCount());
	return m_vDataSources[iId];
}

size_t CModuleFile::CCohDataSource::GetArchiveCount()
{
	return m_vArchives.size();
}

CModuleFile::CArchiveHandler* CModuleFile::CCohDataSource::GetArchive(size_t iId)
{
	if(iId < 0 || iId >= GetArchiveCount()) throw new CRainmanException(0, __FILE__, __LINE__, "ID %lu is outside of range 0-%lu", (unsigned long) iId, (unsigned long)GetArchiveCount());
	return m_vArchives[iId];
}

const char* CModuleFile::CCohDataSource::GetFolder() const
{
	return m_sFolder;
}

bool CModuleFile::CCohDataSource::IsLoaded() const
{
	return m_bIsLoaded;
}

