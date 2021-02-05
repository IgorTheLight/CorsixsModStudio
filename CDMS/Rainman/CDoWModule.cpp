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

#ifdef _DO_INCLUDE_C_DOW_MODULE_H_

#include "CDoWModule.h"
#include <time.h>
#include "memdebug.h"
#include "Exception.h"

static char* mystrdup(const char* sStr)
{
	char* s = new char[strlen(sStr) + 1];
	if(s == 0) return 0;
	strcpy(s, sStr);
	return s;
}

long CDoWModule::GetSgaOutputVersion()
{
	return m_iSgaOutVer;
}

CDoWModule::CDoWModule(void)
{
	m_sUIName = 0;
	m_sName = 0;
	m_sDescription = 0;
	m_sDllName = 0;
	m_sModFolder = 0;
	m_sPatcherUrl = 0;
	m_iModVersionMajor = 0;
	m_iModVersionMinor = 0;
	m_iModVersionRevision = 0;
	m_sTextureFE = 0;
	m_sTextureIcon = 0;
	m_sModFileName = 0;
	m_bInvalidVersionNumber = false;
	m_bLoadFSToCustom = false;
	m_bNoEngine = false;
	m_bIsCohMod = false;
	m_iSgaOutVer = 2;
	m_sLocale = mystrdup("english");

	m_pFiles = 0;
	m_pFSO = 0;

	m_iFileViewState = _MakeFileSourcesHash();
}

CDoWModule::~CDoWModule(void)
{
	_Clean();
	if(m_sLocale)
	{
		delete[] m_sLocale;
	}
}

unsigned long CDoWModule::GetFileviewHash() const
{
	return m_iFileViewState;
}

#define QUICK_TRYCAT(op) try { op } CATCH_THROW("File view error")

// IFileStore Interface
void CDoWModule::VInit(void* pUnused)
{
	if(m_pFiles == 0) QUICK_THROW("No file view")
	QUICK_TRYCAT(m_pFiles->VInit(pUnused);)
}

IFileStore::IStream* CDoWModule::VOpenStream(const char* sFile)
{
	if(m_pFiles == 0) QUICK_THROW("No file view")
	QUICK_TRYCAT(return m_pFiles->VOpenStream(sFile);)
}

IFileStore::IOutputStream* CDoWModule::VOpenOutputStream(const char* sIdentifier, bool bEraseIfPresent)
{
	if(m_pFiles == 0) QUICK_THROW("No file view")
	QUICK_TRYCAT(return m_pFiles->VOpenOutputStream(sIdentifier, bEraseIfPresent);)
}

// IDirectoryTraverser Interface
tLastWriteTime CDoWModule::VGetLastWriteTime(const char* sPath)
{
	if(m_pFiles == 0) QUICK_THROW("No file view")
	QUICK_TRYCAT(return m_pFiles->VGetLastWriteTime(sPath);)
}

void CDoWModule::VCreateFolderIn(const char* sPath, const char* sNewFolderName)
{
	if(m_pFiles == 0) QUICK_THROW("No file view")
	QUICK_TRYCAT(return m_pFiles->VCreateFolderIn(sPath, sNewFolderName);)
}

IDirectoryTraverser::IIterator* CDoWModule::VIterate(const char* sPath)
{
	if(m_pFiles == 0) QUICK_THROW("No file view")
	QUICK_TRYCAT(return m_pFiles->VIterate(sPath);)
}

unsigned long CDoWModule::VGetEntryPointCount()
{
	if(m_pFiles == 0) QUICK_THROW("No file view")
	QUICK_TRYCAT(return m_pFiles->VGetEntryPointCount();)
}

const char* CDoWModule::VGetEntryPoint(unsigned long iID)
{
	if(m_pFiles == 0) QUICK_THROW("No file view")
	QUICK_TRYCAT(return m_pFiles->VGetEntryPoint(iID);)
}

#undef QUICK_TRYCAT

void CDoWModule::SetLocale(const char* sLocale)
{
	if(m_pFSO) QUICK_THROW("Can only set locale on a blank slate");
	if(m_sLocale) delete[] m_sLocale;
	m_sLocale = mystrdup(sLocale);
}

const char* CDoWModule::GetLocale() const
{
	return m_sLocale;
}

void CDoWModule::New()
{
	_Clean();
	m_pFiles = CHECK_MEM(new CDoWFileView());
	try
	{
		m_pFiles->VInit();
	}
	catch(CRainmanException* pE)
	{
		_Clean();
		throw new CRainmanException(__FILE__, __LINE__, "Cannot init file view", pE);
	}
	m_pFSO = CHECK_MEM(new CFileSystemStore());
	try
	{
		m_pFSO->VInit();
	}
	catch(CRainmanException* pE)
	{
		_Clean();
		throw new CRainmanException(__FILE__, __LINE__, "Cannot init file system store", pE);
	}
}

static char* fgetline(FILE *f, unsigned int iInitSize = 32)
{
	unsigned int iTotalLen;
	if(f == 0) return 0;
	if(iInitSize < 4) iInitSize = 4;
	iTotalLen = iInitSize;
	char *sBuffer = new char[iInitSize];
	char *sReadTo = sBuffer;
	if(sBuffer == 0) return 0;

	do
	{
		if(fgets(sReadTo, iInitSize, f) == 0)
		{
			if(feof(f))
			{
				if(sReadTo[strlen(sReadTo) - 1] == '\n') sReadTo[strlen(sReadTo) - 1] = 0;
				return sBuffer;
			}
			delete[] sBuffer;
			return 0;
		}
		if(sReadTo[strlen(sReadTo) - 1] == '\n')
		{
			sReadTo[strlen(sReadTo) - 1] = 0;
			return sBuffer;
		}
		iTotalLen += iInitSize;
		char *sTmp = new char[iTotalLen];
		if(sTmp == 0)
		{
			delete[] sBuffer;
			return 0;
		}
		strcpy(sTmp, sBuffer);
		delete[] sBuffer;
		sBuffer = sTmp;
		sReadTo = sBuffer + strlen(sBuffer);
	}while(1);
}

// VERY hacky function this one
static void _CountWS(const char* s, unsigned long* iBegin, unsigned long* iEnd)
{
	*iBegin = 0;
	*iEnd = 0;
	if(s)
	{
		while(1)
		{
			switch(s[*iBegin])
			{
			case ' ':
			case '\t':
			case 10:
			case 13:
				break;
			default:
				goto scan_end;
			}
			++*iBegin;
		}
scan_end:
		size_t l = strlen(s)-1;
		while(1)
		{
			switch(s[l-*iEnd])
			{
			case ' ':
			case '\t':
			case 10:
			case 13:
				break;
			default:
				return;
			}
			++*iEnd;
		}
	}
}

// Will duplicate a string, replacing the following:
// %LOCALE% -> Locale\{sLocale}
// %TEXTURE-LEVEL% -> Full
// %SOUND-LEVEL% -> Full
// %MODEL-LEVEL% -> High
static char* DupStrResolveDynamics(const char* sInputString, const char* sLocale, const char* sExtraAppend = "")
{
	unsigned long iLocaleCount = 0;
	char* sLoc;
	char* sTmp = (char*) sInputString;
	while(sLoc = strstr(sTmp, "%LOCALE%")) ++iLocaleCount, sTmp = sLoc + 1;

	char* sNewString = new char[strlen(sInputString) + (strlen(sLocale) * iLocaleCount) + strlen(sExtraAppend) + 1];
	if(sNewString == 0) return 0;

	sTmp = sNewString;
	while(*sInputString)
	{
		if(*sInputString == '%')
		{
			if(strncmp(sInputString, "%LOCALE%", 8) == 0)
			{
				strcpy(sTmp, "Locale\\");
				strcat(sTmp, sLocale);
				sTmp += 7 + strlen(sLocale);
				sInputString += 8;
				continue;
			}
			if(strncmp(sInputString, "%TEXTURE-LEVEL%", 15) == 0)
			{
				strcpy(sTmp, "Full");
				sTmp += 4;
				sInputString += 15;
				continue;
			}
			if(strncmp(sInputString, "%SOUND-LEVEL%", 13) == 0)
			{
				strcpy(sTmp, "Full");
				sTmp += 4;
				sInputString += 13;
				continue;
			}
			if(strncmp(sInputString, "%MODEL-LEVEL%", 13) == 0)
			{
				strcpy(sTmp, "High");
				sTmp += 4;
				sInputString += 13;
				continue;
			}
		}
		*sTmp = *sInputString;

		++sInputString;
		++sTmp;
	}
	*sTmp = 0;
	strcat(sNewString, sExtraAppend);
	return sNewString;
}

const char* CDoWModule::GetFilesModName() const
{
	return m_sModFileName;
}

const char* CDoWModule::GetEngineModName()
{
	return "(Engine)";
}

void CDoWModule::_TryLoadDataGeneric(const char* sModName, const char* sDoWPath, bool bReq, const char* sModFSName)
{
	char* sDGFolder = 0;

	// Look in pipeline.ini to begin with
	char* sPipeline = CHECK_MEM(new char[strlen(sDoWPath) + 14]);
	strcpy(sPipeline, sDoWPath);
	strcat(sPipeline, "\\pipeline.ini");
	FILE* fPipeline = fopen(sPipeline, "rb");
	if(fPipeline)
	{
		bool bInThisProject = false;
		while(!feof(fPipeline))
		{
			char *sLine = fgetline(fPipeline);
			if(sLine == 0)
			{
				fclose(fPipeline);
				delete[] sPipeline;
				delete[] sDGFolder;
				throw new CRainmanException(__FILE__, __LINE__, "fgetline() failed");
			}
			char *sCommentBegin = strstr(sLine, ";");
			if(sCommentBegin) *sCommentBegin = 0;

			char *sEquals = strchr(sLine,'=');
			char *sLeftBrace = strchr(sLine,'[');
			if(sLeftBrace && !sEquals)
			{
				char *sRightBrace = strchr(sLeftBrace,']');
				if(sRightBrace)
				{
					*sRightBrace = 0;
					if(strncmp(sLeftBrace + 1, "project:", 8) == 0)
					{
						if(stricmp(sLeftBrace + 9, sModName) == 0)
						{
							bInThisProject = true;
						}
					}
				}
			}
			else if(bInThisProject && sEquals)
			{
				char* sKey = sLine;
				char* sValue = sEquals + 1;
				*sEquals = 0;
				unsigned long iWSBefore, iWSAfter;
				_CountWS(sKey, &iWSBefore, &iWSAfter);
				sKey += iWSBefore;
				sKey[strlen(sKey) - iWSAfter] = 0;
				_CountWS(sValue, &iWSBefore, &iWSAfter);
				sValue += iWSBefore;
				sValue[strlen(sValue) - iWSAfter] = 0;

				if(stricmp(sKey, "DataGeneric") == 0)
				{
					if(!sDGFolder) sDGFolder = mystrdup(sValue);
					if(!sDGFolder)
					{
						fclose(fPipeline);
						delete[] sPipeline;
						delete[] sLine;
						throw new CRainmanException(__FILE__, __LINE__, "mystrdup() failed");
					}
				}
			}
			delete[] sLine;
		}
		fclose(fPipeline);
	}
	delete[] sPipeline;

	// Assume it's the default path if not found
	if(!sDGFolder)
	{
		sDGFolder = CHECK_MEM(new char[strlen(sModName) + 23]);
		strcpy(sDGFolder, "ModTools\\DataGeneric\\");
		strcat(sDGFolder, sModName);
	}

	// Try and iterate the folder
	char* sFullPath = new char[strlen(sDGFolder) + strlen(sDoWPath) + 2];
	if(sFullPath == 0)
	{
		delete[] sDGFolder;
		throw new CRainmanException(__FILE__, __LINE__, "memory allocation problem");
	}
	strcpy(sFullPath, sDoWPath);
	strcat(sFullPath, "\\");
	strcat(sFullPath, sDGFolder);
	delete[] sDGFolder;
	IDirectoryTraverser::IIterator *pDirItr;
	try
	{
		pDirItr = m_pFSO->VIterate(sFullPath);
	}
	catch(CRainmanException *pE)
	{
		delete pE;
		delete[] sFullPath;
		return;
	}
	delete[] sFullPath;

	try
	{
		m_pFiles->AddFileSource(m_pFSO, pDirItr, m_pFSO, sModFSName, "Data Generic", bReq, !bReq, false);
	}
	catch(CRainmanException *pE)
	{
		delete pDirItr;
		throw new CRainmanException(__FILE__, __LINE__, "Cannot add data generic as file source", pE);
	}

	delete pDirItr;
}

#ifndef DOXY_NODOC
/*
	Quick "Catch, Clean, Throw"
*/
#define QCCT(msg) \
	catch(CRainmanException *pE) \
	{ \
		_Clean(); \
		throw new CRainmanException(__FILE__, __LINE__, msg, pE); \
	}

#define QCZ(var) \
	if(var == 0) \
	{ \
		_Clean(); \
		throw new CRainmanException(__FILE__, __LINE__, #var " is zero"); \
	}

#endif

void CDoWModule::Load(const char* sFile, CALLBACK_ARG)
{
 	CDoWFileView *pTempFileViewPtr = m_pFiles;
	bool bTempCustomFS = m_bLoadFSToCustom;
	// Load up module file
	char* sTempLocale = CHECK_MEM(strdup(m_sLocale));
	_Clean();
	char* sModuleFilename = (strrchr(sFile, '\\')) + 1;
	m_sModFileName = CHECK_MEM(mystrdup(sModuleFilename));
	if(sModuleFilename = strstr(m_sModFileName, ".module")) *sModuleFilename = 0;
	try
	{
		SetLocale(sTempLocale);
	}
	QCCT("Cannot set locale")
	free(sTempLocale);

	try
	{
		CallCallback(THE_CALLBACK, "Processing module file directives in \'%s\'", m_sModFileName);
	}
	QCCT("Cannot call callback")

	if(sFile == 0)
	{
		_Clean();
		throw new CRainmanException(__FILE__, __LINE__, "No filename passed");
	}
	FILE *fFile = fopen(sFile, "rb");
	if(fFile == 0)
	{
		_Clean();
		throw new CRainmanException(0, __FILE__, __LINE__, "Could not open \'%s\'", sFile);
	}
	bool bInGlobal = false;

	m_iSgaOutVer = 0;
	while(!feof(fFile))
	{
		char *sLine = fgetline(fFile);
		char *sCommentBegin = strstr(sLine, ";;");
		if(sCommentBegin) *sCommentBegin = 0;

		char *sEquals = strchr(sLine,'=');
		char *sLeftBrace = strchr(sLine,'[');
		if(sLeftBrace && !sEquals)
		{
			char *sRightBrace = strchr(sLeftBrace,']');
			if(sRightBrace)
			{
				bInGlobal = (strnicmp(sLeftBrace + 1, "global", sRightBrace - sLeftBrace - 1) == 0);
			}
		}
		else if(bInGlobal && sEquals)
		{
			char* sKey = sLine;
			char* sValue = sEquals + 1;
			*sEquals = 0;
			unsigned long iWSBefore, iWSAfter;
			_CountWS(sKey, &iWSBefore, &iWSAfter);
			sKey += iWSBefore;
			sKey[strlen(sKey) - iWSAfter] = 0;
			_CountWS(sValue, &iWSBefore, &iWSAfter);
			sValue += iWSBefore;
			sValue[strlen(sValue) - iWSAfter] = 0;

			if(stricmp(sKey, "UIName") == 0)
			{
				if(m_sUIName) delete[] m_sUIName;
				m_sUIName = mystrdup(sValue);
				QCZ(m_sUIName)
			}
			if(stricmp(sKey, "Name") == 0)
			{
				if(m_sName) delete[] m_sName;
				m_sName = mystrdup(sValue);
				QCZ(m_sName)
			}
			else if(stricmp(sKey, "Description") == 0)
			{
				if(m_sDescription) delete[] m_sDescription;
				m_sDescription = mystrdup(sValue);
				QCZ(m_sDescription)
			}
			else if(stricmp(sKey, "NoEngine") == 0)
			{
				m_bNoEngine = true;
			}
			else if(stricmp(sKey, "ForceSga4") == 0)
			{
				m_iSgaOutVer = 4;
			}
			else if(stricmp(sKey, "DllName") == 0)
			{
				if(m_sDllName) delete[] m_sDllName;
				m_sDllName = mystrdup(sValue);
				QCZ(m_sDllName)
			}
			else if(stricmp(sKey, "ModFolder") == 0)
			{
				if(m_sModFolder) delete[] m_sModFolder;
				m_sModFolder = mystrdup(sValue);
				QCZ(m_sModFolder)
			}
			else if(stricmp(sKey, "PatcherUrl") == 0)
			{
				if(m_sPatcherUrl) delete[] m_sPatcherUrl;
				m_sPatcherUrl = mystrdup(sValue);
				QCZ(m_sPatcherUrl)
			}
			else if(stricmp(sKey, "TextureFE") == 0)
			{
				if(m_sTextureFE) delete[] m_sTextureFE;
				m_sTextureFE = mystrdup(sValue);
				QCZ(m_sTextureFE)
			}
			else if(stricmp(sKey, "TextureIcon") == 0)
			{
				if(m_sTextureIcon) delete[] m_sTextureIcon;
				m_sTextureIcon = mystrdup(sValue);
				QCZ(m_sTextureIcon)
			}
			else if(stricmp(sKey, "ModVersion") == 0)
			{
				long *lCurrentPart = &m_iModVersionMajor;
				char *sTmp = sValue;
				while(*sTmp)
				{
					if(*sTmp == '.')
					{
						if(lCurrentPart == &m_iModVersionMajor)
						{
							lCurrentPart = &m_iModVersionMinor;
						}
						else if(lCurrentPart == &m_iModVersionMinor)
						{
							lCurrentPart = &m_iModVersionRevision;
						}
						else if(lCurrentPart == &m_iModVersionRevision)
						{
							m_bInvalidVersionNumber = true;
							break;
						}
					}
					else if(*sTmp >= '0' && *sTmp <= '9')
					{
						*lCurrentPart *= 10;
						*lCurrentPart += (*sTmp - '0');
					}
					else
					{
						m_bInvalidVersionNumber = true;
					}
					++sTmp;
				}
			}
			else if(strnicmp("DataFolder.",sKey,11) == 0)
			{
				long iNum = atol(sKey + 11);
				if(m_mapDataFolders[iNum] == 0)
				{
					m_mapDataFolders[iNum] = strdup(sValue);
					QCZ(m_mapDataFolders[iNum])
				}
			}
			else if(strnicmp("ArchiveFile.",sKey,12) == 0)
			{
				long iNum = atol(sKey + 12);
				if(m_mapArchiveFiles[iNum] == 0)
				{
					m_mapArchiveFiles[iNum] = strdup(sValue);
					QCZ(m_mapArchiveFiles[iNum])
				}
			}
			else if(strnicmp("RequiredMod.",sKey,12) == 0)
			{
				long iNum = atol(sKey + 12);
				if(m_mapRequiredMods[iNum] == 0)
				{
					m_mapRequiredMods[iNum] = strdup(sValue);
					QCZ(m_mapRequiredMods[iNum])
				}
			}
			else if(strnicmp("CompatableMod.",sKey,14) == 0)
			{
				long iNum = atol(sKey + 14);
				if(m_mapCompatableMods[iNum] == 0)
				{
					m_mapCompatableMods[iNum] = strdup(sValue);
					QCZ(m_mapCompatableMods[iNum])
				}
			}
		}

		delete[] sLine;
	}
	fclose(fFile);

	// Is it a CoH or DoW mod?
	if(m_sName != 0 && m_sUIName == 0 && m_mapDataFolders.size() == 0 && m_mapArchiveFiles.size() == 0)
	{
		m_bIsCohMod = true;
		m_mapDataFolders[1] = strdup("Data");
	}

	if(m_iSgaOutVer == 0)
	{
		m_iSgaOutVer = m_bIsCohMod ? 4 : 2;
	}

	// Load Filesystem
	m_pFSO = new CFileSystemStore;
	QCZ(m_pFSO)
	try
	{
		m_pFSO->VInit();
	}
	QCCT("Cannot init file system object")

	m_bLoadFSToCustom = bTempCustomFS;
	if(!m_bLoadFSToCustom)
	{
		m_pFiles = new CDoWFileView;
		QCZ(m_pFiles)
		try
		{
			m_pFiles->VInit();
		}
		QCCT("Cannot init file system object")
	}
	else
	{
		m_pFiles = pTempFileViewPtr;
	}

	char* sDoWPath = strdup(sFile);
	QCZ(sDoWPath)
	char* sSlashPos = strrchr(sDoWPath, '\\');
	if(sSlashPos) *sSlashPos = 0;

	char* sModFolder = new char[strlen(sDoWPath) + strlen(m_sModFolder) + 2];
	if(sModFolder == 0)
	{
		free(sDoWPath);
		_Clean();
		QUICK_THROW("Memory error")
	}
	strcpy(sModFolder, sDoWPath);
	strcat(sModFolder, "\\");
	strcat(sModFolder, m_sModFolder);

	if(m_bIsCohMod)
	{
		char* sSgaFolderPath = new char[strlen(sModFolder) + 11];
		if(sSgaFolderPath == 0)
		{
			free(sDoWPath);
			delete[] sModFolder;
			_Clean();
			QUICK_THROW("Memory error")
		}
		strcpy(sSgaFolderPath, sModFolder);
		strcat(sSgaFolderPath, "\\Archives\\");

		IDirectoryTraverser::IIterator *pItr;
		try
		{
			pItr = m_pFSO->VIterate(sSgaFolderPath);
		}
		catch(CRainmanException *pE)
		{
			free(sDoWPath);
			delete[] sModFolder;
			_Clean();
			pE = new CRainmanException(pE, __FILE__, __LINE__, "Unable to iterate \'%s\'", sSgaFolderPath);
			delete[] sSgaFolderPath;
			throw pE;
		}
		delete[] sSgaFolderPath;
		if(pItr)
		{
			try
			{
				if(pItr->VGetType() != IDirectoryTraverser::IIterator::T_Nothing)
				{
					do
					{
						if(pItr->VGetType() == IDirectoryTraverser::IIterator::T_File)
						{
							const char* sPotentialFileName = pItr->VGetName();
							sPotentialFileName = strrchr(sPotentialFileName, '.');
							if(sPotentialFileName && (stricmp(sPotentialFileName + 1, "sga") == 0))
							{
								m_mapArchiveFiles[m_mapArchiveFiles.size() + 1] = CHECK_MEM(strdup(pItr->VGetName()));
							}
						}
					} while(pItr->VNextItem() == IDirectoryTraverser::IIterator::E_OK);
				}
				delete pItr;
			}
			catch(CRainmanException *pE)
			{
				free(sDoWPath);
				delete[] sModFolder;
				_Clean();
				pE = new CRainmanException(pE, __FILE__, __LINE__, "(rethrow)");
				delete pItr;
				throw pE;
			}
		}
	}

	// Pick an output folder
	char* sOutFolder = 0;
	if(!m_bLoadFSToCustom)
	{
		// Look for one called data
		for(std::map<long, char*>::iterator itr = m_mapDataFolders.begin(); itr != m_mapDataFolders.end(); ++itr)
		{
			if(stricmp(itr->second, "data") == 0)
			{
				sOutFolder = itr->second;
				break;
			}
		}

		if(!sOutFolder)
		{
			// Look for one with no dynamics
			for(std::map<long, char*>::iterator itr = m_mapDataFolders.begin(); itr != m_mapDataFolders.end(); ++itr)
			{
				if(strchr(itr->second, '%') == 0)
				{
					sOutFolder = itr->second;
					break;
				}
			}
		}
	}

	for(std::map<long, char*>::iterator itr = m_mapDataFolders.begin(); itr != m_mapDataFolders.end(); ++itr)
	{
		char* sWithoutDynamics = DupStrResolveDynamics(itr->second, m_sLocale);
		if(sWithoutDynamics == 0) continue;
		CallCallback(THE_CALLBACK, "Loading data folder \'%s\' for \'%s\'", sWithoutDynamics, m_sModFileName);
		char* sFolderFullPath = new char[strlen(sModFolder) + strlen(sWithoutDynamics) + 2];
		if(sFolderFullPath == 0)
		{
			delete[] sWithoutDynamics;
			continue;
		}

		strcpy(sFolderFullPath, sModFolder);
		strcat(sFolderFullPath, "\\");
		strcat(sFolderFullPath, sWithoutDynamics);

		IDirectoryTraverser::IIterator *pDirItr = 0;
		try
		{
			pDirItr = m_pFSO->VIterate(sFolderFullPath);
			if(pDirItr)  m_pFiles->AddFileSource(m_pFSO, pDirItr, m_pFSO, m_sModFileName, itr->second, m_bLoadFSToCustom, !m_bLoadFSToCustom, (!m_bLoadFSToCustom && sOutFolder == itr->second));
		}
		catch(CRainmanException *pE) {delete pE;}

		delete[] sFolderFullPath;
		delete[] sWithoutDynamics;
		delete pDirItr;
	}

	for(std::map<long, char*>::iterator itr = m_mapArchiveFiles.begin(); itr != m_mapArchiveFiles.end(); ++itr)
	{
		char* sWithoutDynamics = DupStrResolveDynamics(itr->second, m_sLocale, m_bIsCohMod ? "" : ".sga");
		CallCallback(THE_CALLBACK, "Loading data archive \'%s\' for \'%s\'", sWithoutDynamics, m_sModFileName);
		if(sWithoutDynamics == 0) continue;
		char* sFolderFullPath = new char[strlen(sModFolder) + strlen(sWithoutDynamics) + 16];
		if(sFolderFullPath == 0)
		{
			delete[] sWithoutDynamics;
			continue;
		}

		strcpy(sFolderFullPath, sModFolder);
		strcat(sFolderFullPath, m_bIsCohMod ? "\\Archives\\" : "\\");
		strcat(sFolderFullPath, sWithoutDynamics);

		CSgaFile* pSga = new CSgaFile;
		if(pSga)
		{
			IFileStore::IStream *pInputStream = 0;
			IDirectoryTraverser::IIterator *pDirItr = 0;
			try
			{
				pInputStream = m_pFSO->VOpenStream(sFolderFullPath);
			}
			catch(CRainmanException *pE)
			{
				delete pE;
				delete pSga;
				pSga = 0;
				goto archive_not_present;
			}
			try
			{
				pSga->Load(pInputStream, m_pFSO->VGetLastWriteTime(sFolderFullPath));
				pSga->VInit(pInputStream);
				pDirItr = pSga->VIterate(pSga->VGetEntryPoint(0));
				if(pDirItr) m_pFiles->AddFileSource(pSga, pDirItr, pSga, m_sModFileName, itr->second, m_bLoadFSToCustom);
				delete pDirItr;
			}
			catch(CRainmanException *pE)
			{
				free(sDoWPath);
				delete[] sWithoutDynamics;
				delete[] sFolderFullPath;
				delete[] sModFolder;
				_Clean();
				pE = new CRainmanException(pE, __FILE__, __LINE__, "(rethrow)");
				delete pDirItr;
				delete pInputStream;
				delete pSga;
				throw pE;
			}
archive_not_present:
			m_vSgas.push_back(pSga);
		}

		delete[] sFolderFullPath;
		delete[] sWithoutDynamics;
	}

	if(!m_bLoadFSToCustom)
	{
		for(std::map<long, char*>::iterator itr = m_mapRequiredMods.begin(); itr != m_mapRequiredMods.end(); ++itr)
		{
			CallCallback(THE_CALLBACK, "Loading required mod \'%s\' for \'%s\'", itr->second, m_sModFileName);
			char* sFolderFullPath = new char[strlen(sDoWPath) + strlen(itr->second) + 9];
			if(sFolderFullPath == 0)
			{
				continue;
			}

			strcpy(sFolderFullPath, sDoWPath);
			strcat(sFolderFullPath, "\\");
			strcat(sFolderFullPath, itr->second);
			strcat(sFolderFullPath, ".module");

			CDoWModule *pReqMod = new CDoWModule();
			if(pReqMod == 0)
			{
				delete[] sFolderFullPath;
				continue;
			}

			pReqMod->m_bLoadFSToCustom = true;
			pReqMod->m_pFiles = m_pFiles;
			m_vInheritedMods.push_back(pReqMod);
			try
			{
				pReqMod->SetLocale(m_sLocale);
				pReqMod->Load(sFolderFullPath, THE_CALLBACK);
			}
			catch(CRainmanException *pE)
			{
				free(sDoWPath);
				delete[] sModFolder;
				delete[] sFolderFullPath;
				_Clean();
				throw new CRainmanException(__FILE__, __LINE__, "Problem loading required mod", pE);
			}
			delete[] sFolderFullPath;
		}

		if(!m_bNoEngine)
		{
			CallCallback(THE_CALLBACK, "Loading engine files");
			char* sEngineFile = new char[strlen(sDoWPath) + 40];
			if(!sEngineFile)
			{
				free(sDoWPath);
				delete[] sModFolder;
				_Clean();
				throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
			}
			strcpy(sEngineFile, sDoWPath);
			strcat(sEngineFile, "\\Engine\\Data");
			IDirectoryTraverser::IIterator *pEngineDirItr;
			try
			{
				pEngineDirItr = m_pFSO->VIterate(sEngineFile);
			}
			catch(CRainmanException *pE)
			{
				delete pE;
				pEngineDirItr = 0;
			}
			if(pEngineDirItr) m_pFiles->AddFileSource(m_pFSO, pEngineDirItr, m_pFSO, GetEngineModName(), "Data", true);
			delete pEngineDirItr;

			strcpy(sEngineFile, sDoWPath);
			strcat(sEngineFile, m_bIsCohMod ? "\\Engine\\Archives\\Engine.sga" : "\\Engine\\Engine.sga");

			CSgaFile* pEngineSga = new CSgaFile;
			if(!pEngineSga)
			{
				free(sDoWPath);
				delete[] sModFolder;
				delete[] sEngineFile;
				_Clean();
				throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
			}

			IFileStore::IStream *pInputStream = 0;
			IDirectoryTraverser::IIterator *pDirItr = 0;
			try
			{
				pInputStream = m_pFSO->VOpenStream(sEngineFile);
				pEngineSga->Load(pInputStream, m_pFSO->VGetLastWriteTime(sEngineFile));
				pEngineSga->VInit(pInputStream);
				pDirItr = pEngineSga->VIterate(pEngineSga->VGetEntryPoint(0));
				if(pDirItr) m_pFiles->AddFileSource(pEngineSga, pDirItr, pEngineSga, GetEngineModName(), "Engine.sga", true);
				delete pDirItr;
			}
			catch(CRainmanException *pE)
			{
				delete pInputStream;
				delete pDirItr;
				free(sDoWPath);
				delete[] sModFolder;
				delete[] sEngineFile;
				_Clean();
				throw new CRainmanException(__FILE__, __LINE__, "Problem loading in engine SGA file", pE);
			}
			m_vSgas.push_back(pEngineSga);

			if(m_bIsCohMod)
			{
				strcpy(sEngineFile, sDoWPath);
				strcat(sEngineFile, "\\RelicOnline\\Archives\\RelicOnline.sga");

				pEngineSga = new CSgaFile;
				if(!pEngineSga)
				{
					free(sDoWPath);
					delete[] sModFolder;
					delete[] sEngineFile;
					_Clean();
					throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
				}

				IFileStore::IStream *pInputStream = 0;
				IDirectoryTraverser::IIterator *pDirItr = 0;
				try
				{
					pInputStream = m_pFSO->VOpenStream(sEngineFile);
					pEngineSga->Load(pInputStream, m_pFSO->VGetLastWriteTime(sEngineFile));
					pEngineSga->VInit(pInputStream);
					pDirItr = pEngineSga->VIterate(pEngineSga->VGetEntryPoint(0));
					if(pDirItr) m_pFiles->AddFileSource(pEngineSga, pDirItr, pEngineSga, GetEngineModName(), "RelicOnline.sga", true);
					delete pDirItr;
				}
				catch(CRainmanException *pE)
				{
					delete pInputStream;
					delete pDirItr;
					free(sDoWPath);
					delete[] sModFolder;
					delete[] sEngineFile;
					_Clean();
					throw new CRainmanException(__FILE__, __LINE__, "Cannot load RelicOnline.sga", pE);
				}
				m_vSgas.push_back(pEngineSga);
			}

			delete[] sEngineFile;
		}
	}
	else
	{
		for(std::map<long, char*>::iterator itr = m_mapRequiredMods.begin(); itr != m_mapRequiredMods.end(); ++itr)
		{
			m_vInheritedMods.push_back(0);
		}
	}

	// Map in Data generic (done after ALL other data to avoid conflicts)
	if(!m_bLoadFSToCustom)
	{
		CallCallback(THE_CALLBACK, "Loading data generic for \'%s\'", m_sModFileName);
		_TryLoadDataGeneric(m_sModFileName, sDoWPath, false, m_sModFileName);
		for(std::map<long, char*>::iterator itr = m_mapRequiredMods.begin(); itr != m_mapRequiredMods.end(); ++itr)
		{
			CallCallback(THE_CALLBACK, "Loading data generic for \'%s\'", itr->second);
			_TryLoadDataGeneric(itr->second, sDoWPath, true, itr->second);
		}
		CallCallback(THE_CALLBACK, "Loading data generic for engine");
		_TryLoadDataGeneric("Engine", sDoWPath, true, GetEngineModName());
	}

	// UCS Files
	char* sUcsFilesPath = new char[strlen(sModFolder) + strlen(m_sLocale) + 9];
	if(sUcsFilesPath == 0)
	{
		free(sDoWPath);
		delete[] sModFolder;
		_Clean();
		throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
	}
	strcpy(sUcsFilesPath, sModFolder);
	strcat(sUcsFilesPath, "\\Locale\\");
	strcat(sUcsFilesPath, m_sLocale);

	IDirectoryTraverser::IIterator *pItr = m_pFSO->VIterate(sUcsFilesPath);
	delete[] sUcsFilesPath;
	if(pItr)
	{
		if(pItr->VGetType() != IDirectoryTraverser::IIterator::T_Nothing)
		{
			do
			{
				if(pItr->VGetType() == IDirectoryTraverser::IIterator::T_File)
				{
					const char* sPotentialFileName = pItr->VGetName();
					sPotentialFileName = strrchr(sPotentialFileName, '.');
					if(sPotentialFileName && (stricmp(sPotentialFileName + 1, "ucs") == 0))
					{
						CallCallback(THE_CALLBACK, "Loading UCS file \'%s\' for \'%s\'", pItr->VGetName(), m_sModFileName);
						IFileStore::IStream *pFileStream;
						try
						{
							pFileStream = pItr->VOpenFile();
						}
						catch(CRainmanException *pE)
						{
							free(sDoWPath);
							delete[] sModFolder;
							delete pItr;
							_Clean();
							throw new CRainmanException(__FILE__, __LINE__, "Cannot open UCS file", pE);
						}

						CUcsFile* pUcs = new CUcsFile;
						if(!pUcs)
						{
							delete pFileStream;
							free(sDoWPath);
							delete[] sModFolder;
							_Clean();
							throw new CRainmanException(__FILE__, __LINE__, "Memory allocation error");
						}
						
						try
						{
							pUcs->Load(pFileStream);
							m_vUcsFiles.push_back(pUcs);
							m_vUcsNames.push_back(strdup(pItr->VGetName()));
							delete pFileStream;
						}
						catch(CRainmanException *pE)
						{
							delete pUcs;
							delete pFileStream;
							free(sDoWPath);
							delete[] sModFolder;
							_Clean();
							throw new CRainmanException(__FILE__, __LINE__, "Cannot load UCS file", pE);
						}
					}
				}
			} while(pItr->VNextItem() == IDirectoryTraverser::IIterator::E_OK);
		}
		delete pItr;
	}

	free(sDoWPath);
	delete[] sModFolder;
	m_iFileViewState = _MakeFileSourcesHash();
}

void CDoWModule::Save(const char* sFile)
{
	if(sFile == 0) QUICK_THROW("No file specified")
	FILE *f = fopen(sFile, "wb");
	if(!f) throw new CRainmanException(0, __FILE__, __LINE__, "Cannot open file \'%s\'", sFile);
	// Print nice header
	fputs(";; //////////////////////////////////////////////////////////////////\xD\n", f);
	fputs(";; ", f);
	fputs(strrchr(sFile, '\\') ? strrchr(sFile, '\\') + 1 : sFile, f);
	fputs("\xD\n;; \xD\n;; ", f);

	time_t tt;
	time(&tt);
	tm *ptm;
	ptm = localtime(&tt);

	const char* sModName = "mod";
	bool bWasDollar = false;
	if(m_sUIName && strlen(m_sUIName))
	{
		if(IsDollarString(m_sUIName))
		{
			bWasDollar = true;
			const wchar_t *tw = ResolveUCS(m_sUIName);
			size_t twl = wcslen(tw) + 1;
			sModName = new char[twl];
			for(size_t i = 0; i < twl; ++i)
			{
				((char*)sModName)[i] = tw[i];
			}
		}
		else
			sModName = m_sUIName;
	}

	fprintf(f, "(c) Copyright %li/%li The %s team\xD\n", ptm->tm_year + 1900, ptm->tm_year + 1901, sModName);

	if(bWasDollar) delete[] ((char*)sModName);

	// General
	fputs(";; \xD\n\xD\n[global]\xD\n", f);

	fputs("\xD\nUIName = ", f);
	fputs(m_sUIName ? m_sUIName : "", f);
	fputs("\xD\nDescription = ", f);
	fputs(m_sDescription ? m_sDescription : "", f);
	fputs("\xD\nDllName = ", f);
	fputs(m_sDllName ? m_sDllName : "", f);
	fputs("\xD\nModFolder = ", f);
	fputs(m_sModFolder ? m_sModFolder : "", f);
	fprintf(f, "\xD\nModVersion = %li.%li.%li", m_iModVersionMajor, m_iModVersionMinor, m_iModVersionRevision);
	fputs("\xD\nTextureFE = ", f);
	fputs(m_sTextureFE ? m_sTextureFE : "", f);
	fputs("\xD\nTextureIcon = ", f);
	fputs(m_sTextureIcon ? m_sTextureIcon : "", f);

	// Data folders
	fputs("\xD\n\xD\n;; //////////////////////////////////////////////////////////////////\xD\n;; List of folders to map for this MOD \xD\n;; order is important - the first folders registered will be scanned for files first\xD\n\xD\n", f);
	for(std::map<long, char*>::iterator itr = m_mapDataFolders.begin(); itr != m_mapDataFolders.end(); ++itr)
	{
		fprintf(f, "DataFolder.%li = %s\xD\n", itr->first, itr->second ? itr->second : "");
	}

	// Data archives
	fputs("\xD\n;; //////////////////////////////////////////////////////////////////\xD\n;; List of archives to map for this MOD \xD\n;; order is important - the first archives registered will be scanned for files first\xD\n\xD\n", f);
	for(std::map<long, char*>::iterator itr = m_mapArchiveFiles.begin(); itr != m_mapArchiveFiles.end(); ++itr)
	{
		fprintf(f, "ArchiveFile.%li = %s\xD\n", itr->first, itr->second ? itr->second : "");
	}

	// Required mods
	fputs("\xD\n;; //////////////////////////////////////////////////////////////////\xD\n;; List of MODs that this MOD requires to work\xD\n\xD\n", f);
	for(std::map<long, char*>::iterator itr = m_mapRequiredMods.begin(); itr != m_mapRequiredMods.end(); ++itr)
	{
		fprintf(f, "RequiredMod.%li = %s\xD\n", itr->first, itr->second ? itr->second : "");
	}

	// Compatible mods
	fputs("\xD\n;; //////////////////////////////////////////////////////////////////\xD\n;; List of mods that have scenarios compatible with this mod\xD\n\xD\n", f);
	for(std::map<long, char*>::iterator itr = m_mapCompatableMods.begin(); itr != m_mapCompatableMods.end(); ++itr)
	{
		fprintf(f, "CompatableMod.%li = %s\xD\n", itr->first, itr->second ? itr->second : "");
	}

	fclose(f);
}

/*
void CDoWModule::RebuildFileview(const char* sRefFile)
{
	if(m_bLoadFSToCustom) return false;
	if(m_pFiles) m_pFiles->Reset(); // it cannot fail
	else
	{
		m_pFiles = new CDoWFileView;
		if(m_pFiles == 0 || !m_pFiles->VInit())
		{
			return false;
		}
	}

	if(!m_pFSO)
	{
		m_pFSO = new CFileSystemStore;
		if(m_pFSO == 0 || !m_pFSO->VInit())
		{
			return false;
		}
	}

	// Do build
	char* sDoWPath = strdup(sRefFile);
	char* sSlashPos = strrchr(sDoWPath, '\\');
	if(sSlashPos) *sSlashPos = 0;

	char* sModFolder = new char[strlen(sDoWPath) + strlen(m_sModFolder) + 2];
	if(sModFolder == 0)
	{
		free(sDoWPath);
		m_pFiles->Reset();
		return false;
	}
	strcpy(sModFolder, sDoWPath);
	strcat(sModFolder, "\\");
	strcat(sModFolder, m_sModFolder);

	for(std::map<long, char*>::iterator itr = m_mapDataFolders.begin(); itr != m_mapDataFolders.end(); ++itr)
	{
		char* sWithoutDynamics = DupStrResolveDynamics(itr->second, m_sLocale);
		if(sWithoutDynamics == 0) continue;
		char* sFolderFullPath = new char[strlen(sModFolder) + strlen(sWithoutDynamics) + 2];
		if(sFolderFullPath == 0)
		{
			delete[] sWithoutDynamics;
			continue;
		}

		strcpy(sFolderFullPath, sModFolder);
		strcat(sFolderFullPath, "\\");
		strcat(sFolderFullPath, sWithoutDynamics);

		IDirectoryTraverser::IIterator *pDirItr = m_pFSO->VIterate(sFolderFullPath);
		if(pDirItr)  m_pFiles->AddFileSource(m_pFSO, pDirItr, m_pFSO, m_sUIName, sWithoutDynamics);

		delete[] sFolderFullPath;
		delete[] sWithoutDynamics;
		delete pDirItr;
	}

	for(std::map<long, char*>::iterator itr = m_mapArchiveFiles.begin(); itr != m_mapArchiveFiles.end(); ++itr)
	{
		char* sWithoutDynamics = DupStrResolveDynamics(itr->second, m_sLocale, ".sga");
		if(sWithoutDynamics == 0) continue;
		char* sFolderFullPath = new char[strlen(sModFolder) + strlen(sWithoutDynamics) + 2];
		if(sFolderFullPath == 0)
		{
			delete[] sWithoutDynamics;
			continue;
		}

		strcpy(sFolderFullPath, sModFolder);
		strcat(sFolderFullPath, "\\");
		strcat(sFolderFullPath, sWithoutDynamics);

		CSgaFile* pSga = new CSgaFile;
		if(pSga)
		{
			IFileStore::IStream *pInputStream = m_pFSO->VOpenStream(sFolderFullPath);
			if(pInputStream && pSga->Load(pInputStream) && pSga->VInit(pInputStream))
			{
				IDirectoryTraverser::IIterator *pDirItr = pSga->VIterate(pSga->VGetEntryPoint(0));
				if(pDirItr) m_pFiles->AddFileSource(pSga, pDirItr, pSga, m_sUIName, sWithoutDynamics);
				delete pDirItr;
			}
			else
			{
				delete pInputStream;
			}
			m_vSgas.push_back(pSga);
		}

		delete[] sFolderFullPath;
		delete[] sWithoutDynamics;
	}

	for(std::map<long, char*>::iterator itr = m_mapRequiredMods.begin(); itr != m_mapRequiredMods.end(); ++itr)
	{
		char* sFolderFullPath = new char[strlen(sDoWPath) + strlen(itr->second) + 9];
		if(sFolderFullPath == 0)
		{
			continue;
		}

		strcpy(sFolderFullPath, sDoWPath);
		strcat(sFolderFullPath, "\\");
		strcat(sFolderFullPath, itr->second);
		strcat(sFolderFullPath, ".module");

		CDoWModule *pReqMod = new CDoWModule();
		if(pReqMod == 0)
		{
			delete[] sFolderFullPath;
			continue;
		}

		pReqMod->m_bLoadFSToCustom = true;
		pReqMod->m_pFiles = m_pFiles;
		m_vInheritedMods.push_back(pReqMod);
		if(!pReqMod->SetLocale(m_sLocale) || !pReqMod->Load(sFolderFullPath))
		{
			free(sDoWPath);
			delete[] sModFolder;
			delete[] sFolderFullPath;
			m_pFiles->Reset();
			return false;
		}
		delete[] sFolderFullPath;
	}

	if(!m_bLoadFSToCustom)
	{
		char* sEngineFile = new char[strlen(sDoWPath) + 13];
		if(!sEngineFile)
		{
			free(sDoWPath);
			delete[] sModFolder;
			m_pFiles->Reset();
			return false;
		}
		strcpy(sEngineFile, sDoWPath);
		strcat(sEngineFile, "\\Engine\\Data");
		IDirectoryTraverser::IIterator *pEngineDirItr = m_pFSO->VIterate(sEngineFile);
		if(!pEngineDirItr)
		{
			free(sDoWPath);
			delete[] sModFolder;
			delete[] sEngineFile;
			m_pFiles->Reset();
			return false;
		}
		m_pFiles->AddFileSource(m_pFSO, pEngineDirItr, m_pFSO, "(Engine)", "Data");
		delete[] sEngineFile;

		sEngineFile = new char[strlen(sDoWPath) + 19];
		if(!sEngineFile)
		{
			free(sDoWPath);
			delete[] sModFolder;
			m_pFiles->Reset();
			return false;
		}
		strcpy(sEngineFile, sDoWPath);
		strcat(sEngineFile, "\\Engine\\Engine.sga");
		//IDirectoryTraverser::IIterator *pEngineDirItr = m_pFSO->VIterate(sEngineFile);

		CSgaFile* pEngineSga = new CSgaFile;
		if(!pEngineSga)
		{
			free(sDoWPath);
			delete[] sModFolder;
			delete[] sEngineFile;
			m_pFiles->Reset();
			return false;
		}

		IFileStore::IStream *pInputStream = m_pFSO->VOpenStream(sEngineFile);
		if(pInputStream && pEngineSga->Load(pInputStream) && pEngineSga->VInit(pInputStream))
		{
			IDirectoryTraverser::IIterator *pDirItr = pEngineSga->VIterate(pEngineSga->VGetEntryPoint(0));
			if(pDirItr) m_pFiles->AddFileSource(pEngineSga, pDirItr, pEngineSga, "(Engine)", "Engine.sga");
			delete pDirItr;
		}
		else
		{
			delete pInputStream;
			free(sDoWPath);
			delete[] sModFolder;
			delete[] sEngineFile;
			m_pFiles->Reset();
			return false;
		}
		m_vSgas.push_back(pEngineSga);

		if(!pEngineDirItr)
		{
			free(sDoWPath);
			delete[] sModFolder;
			delete[] sEngineFile;
			m_pFiles->Reset();
			return false;
		}
		m_pFiles->AddFileSource(m_pFSO, pEngineDirItr, m_pFSO, "(Engine)", "Data");
		delete[] sEngineFile;
	}

	return true;
}
*/

void CDoWModule::_Clean()
{
	if(m_sModFileName) delete[] m_sModFileName;
	m_sModFileName = 0;
	if(m_sUIName) delete[] m_sUIName;
	m_sUIName = 0;
	if(m_sName) delete[] m_sName;
	m_sName = 0;
	if(m_sDescription) delete[] m_sDescription;
	m_sDescription = 0;
	if(m_sDllName) delete[] m_sDllName;
	m_sDllName = 0;
	if(m_sModFolder) delete[] m_sModFolder;
	m_sModFolder = 0;
	if(m_sPatcherUrl) delete[] m_sPatcherUrl;
	m_sPatcherUrl = 0;
	if(m_sTextureFE) delete[] m_sTextureFE;
	m_sTextureFE = 0;
	if(m_sTextureIcon) delete[] m_sTextureIcon;
	m_sTextureIcon = 0;
	if(m_sLocale)
	{
		delete[] m_sLocale;
		m_sLocale = mystrdup("english");
	}

	std::map<long, char*>::iterator itr;
	for(itr = m_mapDataFolders.begin(); itr != m_mapDataFolders.end(); ++itr)
	{
		if(itr->second) free(itr->second);
	}
	m_mapDataFolders.clear();
	for(itr = m_mapArchiveFiles.begin(); itr != m_mapArchiveFiles.end(); ++itr)
	{
		if(itr->second) free(itr->second);
	}
	m_mapArchiveFiles.clear();
	for(itr = m_mapRequiredMods.begin(); itr != m_mapRequiredMods.end(); ++itr)
	{
		if(itr->second) free(itr->second);
	}
	m_mapRequiredMods.clear();
	for(itr = m_mapCompatableMods.begin(); itr != m_mapCompatableMods.end(); ++itr)
	{
		if(itr->second) free(itr->second);
	}
	m_mapCompatableMods.clear();

	if(m_pFiles)
	{
		if(!m_bLoadFSToCustom) delete m_pFiles;
		m_pFiles = 0;
	}

	if(m_pFSO)
	{
		delete m_pFSO;
		m_pFSO = 0;
	}

	for(std::vector<CSgaFile*>::iterator itr = m_vSgas.begin(); itr != m_vSgas.end(); ++itr)
	{
		if(*itr)
		{
			try
			{
				delete (*itr)->GetInputStream();
			}
			catch(CRainmanException *pE) {delete pE;}
			delete *itr;
		}
	}
	m_vSgas.clear();

	for(std::vector<CDoWModule*>::iterator itr = m_vInheritedMods.begin(); itr != m_vInheritedMods.end(); ++itr)
	{
		delete *itr;
	}
	m_vInheritedMods.clear();

	for(std::vector<CUcsFile*>::iterator itr = m_vUcsFiles.begin(); itr != m_vUcsFiles.end(); ++itr)
	{
		delete *itr;
	}
	m_vUcsFiles.clear();

	for(std::vector<char*>::iterator itr = m_vUcsNames.begin(); itr != m_vUcsNames.end(); ++itr)
	{
		delete[] *itr;
	}
	m_vUcsNames.clear();

	m_iModVersionMajor = 0;
	m_iModVersionMinor = 0;
	m_iModVersionRevision = 0;
	m_bInvalidVersionNumber = false;
	m_bLoadFSToCustom = false;
	m_bNoEngine = false;
	m_bIsCohMod = false;
	m_iSgaOutVer = 2;

	m_iFileViewState = _MakeFileSourcesHash();
}

const char* CDoWModule::GetUIName() const
{
	return m_sUIName;
}

const char* CDoWModule::GetDescription() const
{
	return m_sDescription;
}

const char* CDoWModule::GetDllName() const
{
	return m_sDllName;
}

const char* CDoWModule::GetModFolder() const
{
	return m_sModFolder;
}

const char* CDoWModule::GetTextureFE() const
{
	return m_sTextureFE;
}

const char* CDoWModule::GetTextureIcon() const
{
	return m_sTextureIcon;
}

long CDoWModule::GetVersionMajor() const
{
	return m_iModVersionMajor;
}

long CDoWModule::GetVersionMinor() const
{
	return m_iModVersionMinor;
}

long CDoWModule::GetVersionRevision() const
{
	return m_iModVersionRevision;
}

#define AUTO_SET(valname) char* sTmp = CHECK_MEM(mystrdup(sValue ? sValue : ""));if(valname) delete[] valname;valname = sTmp;

void CDoWModule::SetUIName(const char* sValue)
{
	AUTO_SET(m_sUIName)
}

void CDoWModule::SetDescription(const char* sValue)
{
	AUTO_SET(m_sDescription)
}

void CDoWModule::SetDllName(const char* sValue)
{
	AUTO_SET(m_sDllName)
}

void CDoWModule::SetModFolder(const char* sValue)
{
	AUTO_SET(m_sModFolder)
}

void CDoWModule::SetTextureFE(const char* sValue)
{
	AUTO_SET(m_sTextureFE)
}

void CDoWModule::SetTextureIcon(const char* sValue)
{
	AUTO_SET(m_sTextureIcon)
}

void CDoWModule::SetVersionMajor(long iValue)
{
	m_iModVersionMajor = iValue;
}

void CDoWModule::SetVersionMinor(long iValue)
{
	m_iModVersionMinor = iValue;
}

void CDoWModule::SetVersionRevision(long iValue)
{
	m_iModVersionRevision = iValue;
}

long CDoWModule::GetDataFolderCount() const
{
	return (long)m_mapDataFolders.size();
}

const char* CDoWModule::GetDataFolder(long iIndex)
{
	if(iIndex < 0 || iIndex >= GetDataFolderCount()) return 0;
	std::map<long, char*>::iterator itr = m_mapDataFolders.begin();
	while(iIndex) --iIndex, ++itr;
	return itr->second;
}

long CDoWModule::GetDataFolderID(long iIndex)
{
	if(iIndex < 0 || iIndex >= GetDataFolderCount()) return -1;
	std::map<long, char*>::iterator itr = m_mapDataFolders.begin();
	while(iIndex) --iIndex, ++itr;
	return itr->first;
}

void CDoWModule::SwapDataFolders(long iIndexA, long iIndexB)
{
	if(iIndexA < 0 || iIndexA >= GetDataFolderCount()) throw new CRainmanException(0, __FILE__, __LINE__, "IndexA %li is beyond %li or below 0", iIndexA, GetDataFolderCount());
	if(iIndexB < 0 || iIndexB >= GetDataFolderCount()) throw new CRainmanException(0, __FILE__, __LINE__, "IndexB %li is beyond %li or below 0", iIndexB, GetDataFolderCount());

	std::map<long, char*>::iterator itrA = m_mapDataFolders.begin();
	while(iIndexA) --iIndexA, ++itrA;
	std::map<long, char*>::iterator itrB = m_mapDataFolders.begin();
	while(iIndexB) --iIndexB, ++itrB;

	char* sTmp = itrA->second;
	itrA->second = itrB->second;
	itrB->second = sTmp;
}

void CDoWModule::AddDataFolder(const char* sName)
{
	long iNextID = 1;
	if(m_mapDataFolders.size()) iNextID = m_mapDataFolders.rbegin()->first + 1;
	char* sTmp = CHECK_MEM(mystrdup(sName));
	m_mapDataFolders[iNextID] = sTmp;
}

void CDoWModule::RemoveDataFolder(long iIndex)
{
	if(iIndex < 0 || iIndex >= GetDataFolderCount()) throw new CRainmanException(0, __FILE__, __LINE__, "Index %li is beyond %li or below 0", iIndex, GetDataFolderCount());
	std::map<long, char*>::iterator itr = m_mapDataFolders.begin();
	while(iIndex) --iIndex, ++itr;
	delete[] itr->second;

	long iToEraseID, iActualErase;
	std::map<long, char*>::reverse_iterator ritr = m_mapDataFolders.rbegin();
	iToEraseID = itr->first;
	iActualErase = ritr->first;

	char* sOld, *sTmp;
	if(ritr->first != iToEraseID)
	{
		sOld = ritr->second;
		do
		{
			++ritr;
			sTmp = ritr->second;
			ritr->second = sOld;
			sOld = sTmp;
		} while(ritr->first != iToEraseID);
	}

	m_mapDataFolders.erase(iActualErase);
}

long CDoWModule::GetArchiveCount() const 
{
	return (long)m_mapArchiveFiles.size();
}

const char* CDoWModule::GetArchive(long iIndex)
{
	if(iIndex < 0 || iIndex >= GetArchiveCount()) throw new CRainmanException(0, __FILE__, __LINE__, "Index %li is beyond %li or below 0", iIndex, GetArchiveCount());
	std::map<long, char*>::iterator itr = m_mapArchiveFiles.begin();
	while(iIndex) --iIndex, ++itr;
	return itr->second;
}

long CDoWModule::GetArchiveID(long iIndex)
{
	if(iIndex < 0 || iIndex >= GetArchiveCount()) throw new CRainmanException(0, __FILE__, __LINE__, "Index %li is beyond %li or below 0", iIndex, GetArchiveCount());
	std::map<long, char*>::iterator itr = m_mapArchiveFiles.begin();
	while(iIndex) --iIndex, ++itr;
	return itr->first;
}

CSgaFile* CDoWModule::GetArchiveHandle(long iIndex) const
{
	if(iIndex < 0 || iIndex >= GetArchiveCount()) throw new CRainmanException(0, __FILE__, __LINE__, "Index %li is beyond %li or below 0", iIndex, GetArchiveCount());
	return m_vSgas[iIndex];
}

void CDoWModule::SwapArchives(long iIndexA, long iIndexB)
{
	if(iIndexA < 0 || iIndexA >= GetArchiveCount()) throw new CRainmanException(0, __FILE__, __LINE__, "IndexA %li is beyond %li or below 0", iIndexA, GetArchiveCount());
	if(iIndexB < 0 || iIndexB >= GetArchiveCount()) throw new CRainmanException(0, __FILE__, __LINE__, "IndexB %li is beyond %li or below 0", iIndexB, GetArchiveCount());

	std::map<long, char*>::iterator itrA = m_mapArchiveFiles.begin();
	while(iIndexA) --iIndexA, ++itrA;
	std::map<long, char*>::iterator itrB = m_mapArchiveFiles.begin();
	while(iIndexB) --iIndexB, ++itrB;

	char* sTmp = itrA->second;
	itrA->second = itrB->second;
	itrB->second = sTmp;

	CSgaFile* pTmpSga = m_vSgas[iIndexA];
	m_vSgas[iIndexA] = m_vSgas[iIndexB];
	m_vSgas[iIndexB] = pTmpSga;

}

void CDoWModule::AddArchive(const char* sName)
{
	long iNextID = 1;
	if(m_mapArchiveFiles.size()) iNextID = m_mapArchiveFiles.rbegin()->first + 1;
	char* sTmp = CHECK_MEM(mystrdup(sName));
	m_mapArchiveFiles[iNextID] = sTmp;
	m_vSgas.push_back(0);
}

void CDoWModule::RemoveArchive(long iIndex)
{
	if(iIndex < 0 || iIndex >= GetArchiveCount()) throw new CRainmanException(0, __FILE__, __LINE__, "Index %li is beyond %li or below 0", iIndex, GetArchiveCount());
	std::map<long, char*>::iterator itr = m_mapArchiveFiles.begin();
	while(iIndex) --iIndex, ++itr;
	delete[] itr->second;

	long iToEraseID, iActualErase;
	std::map<long, char*>::reverse_iterator ritr = m_mapArchiveFiles.rbegin();
	iToEraseID = itr->first;
	iActualErase = ritr->first;

	char* sOld, *sTmp;
	if(ritr->first != iToEraseID)
	{
		sOld = ritr->second;
		do
		{
			++ritr;
			sTmp = ritr->second;
			ritr->second = sOld;
			sOld = sTmp;
		} while(ritr->first != iToEraseID);
	}

	m_mapArchiveFiles.erase(iActualErase);
	m_vSgas.erase(m_vSgas.begin() + iIndex);
}

long CDoWModule::GetRequiredCount() const
{
	return (long)m_mapRequiredMods.size();
}

const char* CDoWModule::GetRequired(long iIndex)
{
	if(iIndex < 0 || iIndex >= GetRequiredCount()) throw new CRainmanException(0, __FILE__, __LINE__, "Index %li is beyond %li or below 0", iIndex, GetRequiredCount());
	std::map<long, char*>::iterator itr = m_mapRequiredMods.begin();
	while(iIndex) --iIndex, ++itr;
	return itr->second;
}

long CDoWModule::GetRequiredID(long iIndex)
{
	if(iIndex < 0 || iIndex >= GetRequiredCount()) throw new CRainmanException(0, __FILE__, __LINE__, "Index %li is beyond %li or below 0", iIndex, GetRequiredCount());
	std::map<long, char*>::iterator itr = m_mapRequiredMods.begin();
	while(iIndex) --iIndex, ++itr;
	return itr->first;
}

CDoWModule* CDoWModule::GetRequiredHandle(long iIndex) const
{
	if(iIndex < 0 || iIndex >= GetArchiveCount()) throw new CRainmanException(0, __FILE__, __LINE__, "Index %li is beyond %li or below 0", iIndex, GetRequiredCount());
	return m_vInheritedMods[iIndex];
}

void CDoWModule::SwapRequireds(long iIndexA, long iIndexB)
{
	if(iIndexA < 0 || iIndexA >= GetRequiredCount()) throw new CRainmanException(0, __FILE__, __LINE__, "IndexA %li is beyond %li or below 0", iIndexA, GetRequiredCount());
	if(iIndexA < 0 || iIndexB >= GetRequiredCount()) throw new CRainmanException(0, __FILE__, __LINE__, "IndexB %li is beyond %li or below 0", iIndexB, GetRequiredCount());

	std::map<long, char*>::iterator itrA = m_mapRequiredMods.begin();
	while(iIndexA) --iIndexA, ++itrA;
	std::map<long, char*>::iterator itrB = m_mapRequiredMods.begin();
	while(iIndexB) --iIndexB, ++itrB;

	char* sTmp = itrA->second;
	itrA->second = itrB->second;
	itrB->second = sTmp;

	CDoWModule* pTmpSga = m_vInheritedMods[iIndexA];
	m_vInheritedMods[iIndexA] = m_vInheritedMods[iIndexB];
	m_vInheritedMods[iIndexB] = pTmpSga;
}

void CDoWModule::AddRequired(const char* sName)
{
	long iNextID = 1;
	if(m_mapRequiredMods.size()) iNextID = m_mapRequiredMods.rbegin()->first + 1;
	char* sTmp = CHECK_MEM(mystrdup(sName));
	m_mapRequiredMods[iNextID] = sTmp;
	m_vInheritedMods.push_back(0);
}

void CDoWModule::RemoveRequired(long iIndex)
{
	if(iIndex < 0 || iIndex >= GetRequiredCount()) throw new CRainmanException(0, __FILE__, __LINE__, "Index %li is beyond %li or below 0", iIndex, GetRequiredCount());
	std::map<long, char*>::iterator itr = m_mapRequiredMods.begin();
	while(iIndex) --iIndex, ++itr;
	delete[] itr->second;

	long iToEraseID, iActualErase;
	std::map<long, char*>::reverse_iterator ritr = m_mapRequiredMods.rbegin();
	iToEraseID = itr->first;
	iActualErase = ritr->first;

	char* sOld, *sTmp;
	if(ritr->first != iToEraseID)
	{
		sOld = ritr->second;
		do
		{
			++ritr;
			sTmp = ritr->second;
			ritr->second = sOld;
			sOld = sTmp;
		} while(ritr->first != iToEraseID);
	}

	m_mapRequiredMods.erase(iActualErase);
	m_vInheritedMods.erase(m_vInheritedMods.begin() + iIndex);
}

long CDoWModule::GetCompatableCount() const
{
	return (long)m_mapCompatableMods.size();
}

const char* CDoWModule::GetCompatable(long iIndex)
{
	if(iIndex < 0 || iIndex >= GetCompatableCount()) throw new CRainmanException(0, __FILE__, __LINE__, "Index %li is beyond %li or below 0", iIndex, GetCompatableCount());
	std::map<long, char*>::iterator itr = m_mapCompatableMods.begin();
	while(iIndex) --iIndex, ++itr;
	return itr->second;
}

long CDoWModule::GetCompatableID(long iIndex)
{
	if(iIndex < 0 || iIndex >= GetCompatableCount()) throw new CRainmanException(0, __FILE__, __LINE__, "Index %li is beyond %li or below 0", iIndex, GetCompatableCount());
	std::map<long, char*>::iterator itr = m_mapCompatableMods.begin();
	while(iIndex) --iIndex, ++itr;
	return itr->first;
}

void CDoWModule::SwapCompatables(long iIndexA, long iIndexB)
{
	if(iIndexA < 0 || iIndexA >= GetCompatableCount()) throw new CRainmanException(0, __FILE__, __LINE__, "IndexA %li is beyond %li or below 0", iIndexA, GetCompatableCount());
	if(iIndexA < 0 || iIndexB >= GetCompatableCount()) throw new CRainmanException(0, __FILE__, __LINE__, "IndexB %li is beyond %li or below 0", iIndexB, GetCompatableCount());

	std::map<long, char*>::iterator itrA = m_mapCompatableMods.begin();
	while(iIndexA) --iIndexA, ++itrA;
	std::map<long, char*>::iterator itrB = m_mapCompatableMods.begin();
	while(iIndexB) --iIndexB, ++itrB;

	char* sTmp = itrA->second;
	itrA->second = itrB->second;
	itrB->second = sTmp;
}

void CDoWModule::AddCompatable(const char* sName)
{
	long iNextID = 1;
	if(m_mapCompatableMods.size()) iNextID = m_mapCompatableMods.rbegin()->first + 1;
	char* sTmp = CHECK_MEM(mystrdup(sName));
	m_mapCompatableMods[iNextID] = sTmp;
}

void CDoWModule::RemoveCompatable(long iIndex)
{
	if(iIndex < 0 || iIndex >= GetCompatableCount()) throw new CRainmanException(0, __FILE__, __LINE__, "Index %li is beyond %li or below 0", iIndex, GetCompatableCount());
	std::map<long, char*>::iterator itr = m_mapCompatableMods.begin();
	while(iIndex) --iIndex, ++itr;
	delete[] itr->second;

	long iToEraseID, iActualErase;
	std::map<long, char*>::reverse_iterator ritr = m_mapCompatableMods.rbegin();
	iToEraseID = itr->first;
	iActualErase = ritr->first;

	char* sOld, *sTmp;
	if(ritr->first != iToEraseID)
	{
		sOld = ritr->second;
		do
		{
			++ritr;
			sTmp = ritr->second;
			ritr->second = sOld;
			sOld = sTmp;
		} while(ritr->first != iToEraseID);
	}

	m_mapCompatableMods.erase(iActualErase);
}

bool CDoWModule::IsDollarString(const char* s)
{
	if(!s || *s != '$') return false;
	++s;
	if(*s == 0) return false;
	while(*s)
	{
		if(*s < '0' || *s > '9') return false;
		++s;
	}
	return true;
}

bool CDoWModule::IsDollarString(const wchar_t* s)
{
	if(!s || *s != '$') return false;
	++s;
	if(*s == 0) return false;
	while(*s)
	{
		if(*s < '0' || *s > '9') return false;
		++s;
	}
	return true;
}

const wchar_t* CDoWModule::ResolveUCS(const char* sDollarString)
{
	if(!IsDollarString(sDollarString)) throw new CRainmanException(__FILE__, __LINE__, "sDollarString must be a dollar string");
	try
	{
		return ResolveUCS((unsigned long)atol(sDollarString + 1));
	}
	CATCH_THROW("Cannot resolve value as integer")
}

const wchar_t* CDoWModule::ResolveUCS(const wchar_t* sDollarString)
{
	if(!IsDollarString(sDollarString)) throw new CRainmanException(__FILE__, __LINE__, "sDollarString must be a dollar string");
	try
	{
		return ResolveUCS(wcstoul(sDollarString + 1, 0, 10));
	}
	CATCH_THROW("Cannot resolve value as integer")
}

const wchar_t* CDoWModule::ResolveUCS(unsigned long iStringID)
{
	for(std::vector<CUcsFile*>::iterator itr = m_vUcsFiles.begin(); itr != m_vUcsFiles.end(); ++itr)
	{
		const wchar_t *t;
		if(t = (*itr)->ResolveStringID(iStringID))
		{
			return t;
		}
	}

	for(std::vector<CDoWModule*>::iterator itr = m_vInheritedMods.begin(); itr != m_vInheritedMods.end(); ++itr)
	{
		const wchar_t *t;
		if((*itr) && (t = (*itr)->ResolveUCS(iStringID)))
		{
			return t;
		}
	}

	throw new CRainmanException(0, __FILE__, __LINE__, "$%lu no key!", iStringID);
}

long CDoWModule::GetUcsFileCount() const
{
	return (long)m_vUcsFiles.size();
}

const char* CDoWModule::GetUcsFileName(long iIndex) const
{
	if(iIndex < 0 || iIndex >= GetUcsFileCount()) return 0;
	return m_vUcsNames[iIndex];
}

CUcsFile* CDoWModule::GetUcsFile(long iIndex) const
{
	if(iIndex < 0 || iIndex >= GetUcsFileCount()) return 0;
	return m_vUcsFiles[iIndex];
}

unsigned long CDoWModule::_MakeFileSourcesHash(unsigned long iBase)
{
	long lval;
	// Head
	lval = 'HEAD';
	iBase = crc32(iBase, (const Bytef*)&lval, 4);
	if(m_sModFolder) iBase = crc32(iBase, (const Bytef*)m_sModFolder, strlen(m_sModFolder));

	// Data folders
	for(std::map<long, char*>::iterator itr = m_mapDataFolders.begin(); itr != m_mapDataFolders.end(); ++itr)
	{
		lval = 'DFOR';
		iBase = crc32(iBase, (const Bytef*)&lval, 4);
		iBase = crc32(iBase, (const Bytef*)&itr->first, 4);
		iBase = crc32(iBase, (const Bytef*)itr->second, strlen(itr->second));
	}

	// Data archives
	std::vector<CSgaFile*>::iterator itrsga = m_vSgas.begin();
	for(std::map<long, char*>::iterator itr = m_mapArchiveFiles.begin(); itr != m_mapArchiveFiles.end(); ++itr, ++itrsga)
	{
		lval = 'GSGA';
		iBase = crc32(iBase, (const Bytef*)&lval, 4);
		iBase = crc32(iBase, (const Bytef*)&itr->first, 4);
		iBase = crc32(iBase, (const Bytef*)itr->second, strlen(itr->second));
		iBase = crc32(iBase, (const Bytef*)*itrsga, 4);
	}

	// Required mods
	std::vector<CDoWModule*>::iterator itrmod = m_vInheritedMods.begin();
	for(std::map<long, char*>::iterator itr = m_mapRequiredMods.begin(); itr != m_mapRequiredMods.end(); ++itr, ++itrmod)
	{
		lval = 'RQUM';
		iBase = crc32(iBase, (const Bytef*)&lval, 4);
		iBase = crc32(iBase, (const Bytef*)&itr->first, 4);
		iBase = crc32(iBase, (const Bytef*)itr->second, strlen(itr->second));
		if((*itrmod)) iBase = (**itrmod)._MakeFileSourcesHash(iBase);
	}

	// Tail
	lval = 'TAIL';
	iBase = crc32(iBase, (const Bytef*)&lval, 4);
	return iBase;
}

void CDoWModule::RegisterNewUCS(const char* sFile, CUcsFile* pUcs)
{
	if(sFile == 0) throw new CRainmanException(__FILE__, __LINE__, "Invalid argument: sFile");
	if(pUcs == 0) throw new CRainmanException(__FILE__, __LINE__, "Invalid argument: pUcs");
	sFile = CHECK_MEM(mystrdup(sFile));
	m_vUcsFiles.push_back(pUcs);
	m_vUcsNames.push_back((char*)sFile);
}

long CDoWModule::GetEngineDataFolderCount() const
{
	return 1;
}

const char* CDoWModule::GetEngineDataFolder(long iIndex)
{
	if(iIndex == 0) return "Data";
	throw new CRainmanException(0, __FILE__, __LINE__, "Invalid index %li", iIndex);
}

long CDoWModule::GetEngineArchiveCount() const
{
	return m_bIsCohMod ? 2 : 1;
}

const char* CDoWModule::GetEngineArchive(long iIndex)
{
	if(iIndex == 0) return "Engine.sga";
	if(iIndex == 1 && m_bIsCohMod) return "RelicOnline.sga";
	throw new CRainmanException(0, __FILE__, __LINE__, "Invalid index %li", iIndex);
}

#endif

