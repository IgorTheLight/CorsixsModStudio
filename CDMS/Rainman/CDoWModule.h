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
#ifndef _C_DOW_MODULE_H_
#define _C_DOW_MODULE_H_

#include <vector>
#include <map>

#include "gnuc_defines.h"
#include "CFileSystemStore.h" // MODULEs are only in file system, not in SGAs
#include "CDoWFileView.h"
#include "CSgaFile.h"
#include "CUcsFile.h"
#include "zLib\zlib.h"
#include "Callbacks.h"
#include "Api.h"

//! Represents a Dawn of War modification (.module file)
/*!
	Represents the actual .module file, however it also loads up
	all of the mod's data folders, SGA archives and UCS files.
*/
class RAINMAN_API CDoWModule : public IFileStore, public IDirectoryTraverser
{
public:
	CDoWModule(void);
	~CDoWModule(void);

	//! Sets which locale to load
	/*!
		\param[in] sLocale Specifies the folder to use. Should be one of: Chinese, Czech
		, English, English_Chinese, French, German, Italian, Japanese, Korean, Korean adult
		, Polish, Russian, Slovak or Spanish
		\return throws exception on failure, no return otherwise
		\note Will only work if no mod is currently loaded (eg. Before you call New() or Load() )
		\see http://www.relic.com/rdn/wiki/DOWUCS
		\see GetLocale
		\see CUcsFile
	*/
	void SetLocale(const char* sLocale); // will only work if no mod is loaded.

	//! Retreives the loaded locale
	/*!
		\return NULL in error, or a pointer to the locale string
		\see SetLocale()
	*/
	const char* GetLocale() const;

	//! Creates a new module
	/*!
		\return throws exception on failure, no return otherwise
	*/
	void New();

	//! Loads a mod from disk
	/*!
		Loads and parses the specified .module file from disk and then loads
		up the mod's data folders, SGA archives, UCS files and any other
		mods that it requires
		\param[in] sFile The .module file to load from disk
		\return throws exception on failure, no return otherwise
	*/
	void Load(const char* sFile, CALLBACK_ARG_OPT );

	//! Saves a mod to disk
	/*!
		Saves a .module file to disk. It does NOT save any of the mod's files
		or archives to disk though.
		\param[in] sFile The .module file to save to disk
		\return throws exception on failure, no return otherwise
	*/
	void Save(const char* sFile);

	//! Recreates the representation of data folders and SGA archives
	/*!
		Use after you make changes to data folders, data archives or
		required mods.
		\param[in] sRefFile The name of any file in the DoW folder
		\return throws exception on failure, no return otherwise
		\bug old inherited mods are not unloaded
		\todo fix engine files loading code
		\todo needs to update hash + other MAJOR updates
	*/
	//void RebuildFileview(const char* sRefFile);

	//! A 32 bit number representing the data folders and SGA archives
	/*!
		
	*/
	unsigned long GetFileviewHash() const;

	const char* GetFilesModName() const;
	static const char* GetEngineModName();

	// IFileStore Interface
	virtual void VInit(void* pUnused = 0);
	virtual IStream* VOpenStream(const char* sFile);
	virtual IOutputStream* VOpenOutputStream(const char* sIdentifier, bool bEraseIfPresent);

	// IDirectoryTraverser Interface
	virtual IDirectoryTraverser::IIterator* VIterate(const char* sPath);
	virtual unsigned long VGetEntryPointCount();
	virtual const char* VGetEntryPoint(unsigned long iID);
	virtual void VCreateFolderIn(const char* sPath, const char* sNewFolderName);
	virtual tLastWriteTime VGetLastWriteTime(const char* sPath);

	// Mod information accessors
	long GetSgaOutputVersion();
	const char* GetUIName() const;
	const char* GetDescription() const;
	const char* GetDllName() const;
	const char* GetModFolder() const;
	const char* GetTextureFE() const;
	const char* GetTextureIcon() const;
	long GetVersionMajor() const;
	long GetVersionMinor() const;
	long GetVersionRevision() const;

	void SetUIName(const char* sValue);
	void SetDescription(const char* sValue);
	void SetDllName(const char* sValue);
	void SetModFolder(const char* sValue);
	void SetTextureFE(const char* sValue);
	void SetTextureIcon(const char* sValue);
	void SetVersionMajor(long iValue);
	void SetVersionMinor(long iValue);
	void SetVersionRevision(long iValue);

	long GetDataFolderCount() const;
	const char* GetDataFolder(long iIndex);
	long GetDataFolderID(long iIndex);
	void SwapDataFolders(long iIndexA, long iIndexB);
	void AddDataFolder(const char* sName);
	void RemoveDataFolder(long iIndex);

	long GetArchiveCount() const;
	const char* GetArchive(long iIndex);
	long GetArchiveID(long iIndex);
	CSgaFile* GetArchiveHandle(long iIndex) const;
	void SwapArchives(long iIndexA, long iIndexB);
	void AddArchive(const char* sName);
	void RemoveArchive(long iIndex);

	long GetRequiredCount() const;
	const char* GetRequired(long iIndex);
	long GetRequiredID(long iIndex);
	CDoWModule* GetRequiredHandle(long iIndex) const;
	void SwapRequireds(long iIndexA, long iIndexB);
	void AddRequired(const char* sName);
	void RemoveRequired(long iIndex);

	long GetCompatableCount() const;
	const char* GetCompatable(long iIndex);
	long GetCompatableID(long iIndex);
	void SwapCompatables(long iIndexA, long iIndexB);
	void AddCompatable(const char* sName);
	void RemoveCompatable(long iIndex);

	// Engine stuff
	long GetEngineDataFolderCount() const;
	const char* GetEngineDataFolder(long iIndex);

	long GetEngineArchiveCount() const;
	const char* GetEngineArchive(long iIndex);

	// UCS Stuff
	static bool IsDollarString(const char* sString);
	static bool IsDollarString(const wchar_t* sString);
	const wchar_t* ResolveUCS(const char* sDollarString);
	const wchar_t* ResolveUCS(const wchar_t* sDollarString);
	const wchar_t* ResolveUCS(unsigned long iStringID);
	long GetUcsFileCount() const;
	const char* GetUcsFileName(long iIndex) const;
	CUcsFile* GetUcsFile(long iIndex) const;
	void RegisterNewUCS(const char* sFile, CUcsFile* pUcs);

protected:
	char* m_sUIName; // DoW
	char* m_sName; // CoH
	char* m_sDescription; // Both
	char* m_sDllName; // Both
	char* m_sModFolder; // Both
	char* m_sPatcherUrl; // CoH
	char* m_sModFileName;
	long m_iModVersionMajor, m_iModVersionMinor, m_iModVersionRevision; // DoW: A.B.C   CoH: A.B
	char* m_sTextureFE; // DoW
	char* m_sTextureIcon; // DoW
	bool m_bInvalidVersionNumber;
	bool m_bLoadFSToCustom;
	bool m_bNoEngine;

	bool m_bIsCohMod;
	long m_iSgaOutVer;

	std::map<long, char*> m_mapDataFolders; // DoW
	std::map<long, char*> m_mapArchiveFiles; // DoW
	std::map<long, char*> m_mapRequiredMods; // DoW [CoH method unknown]
	std::map<long, char*> m_mapCompatableMods; // DoW [CoH method unknown]

	// Filesystem stuff
	CDoWFileView* m_pFiles;
	CFileSystemStore* m_pFSO;
	std::vector<CSgaFile*> m_vSgas;
	std::vector<CDoWModule*> m_vInheritedMods;
	unsigned long m_iFileViewState;
	void _TryLoadDataGeneric(const char* sModName, const char* sDoWPath, bool bReq, const char* sModFSName);

	// UCS Stuff
	std::vector<CUcsFile*> m_vUcsFiles;
	std::vector<char*> m_vUcsNames;

	// Random
	void _Clean();
	char* m_sLocale;
	unsigned long _MakeFileSourcesHash(unsigned long iBase = crc32(0,0,0));
};

#endif
#endif

