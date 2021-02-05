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

#ifndef _C_MODULE_FILE_H_
#define _C_MODULE_FILE_H_

#include "gnuc_defines.h"
#include "CSgaFile.h"
#include "CUcsFile.h"
#include "CDoWFileView.h"
#include "CFileMap.h"
#include "CFileSystemStore.h"
#include "Api.h"
#include "Callbacks.h"

class RAINMAN_API CModuleFile : public IFileStore, public IDirectoryTraverser
{
public:
	CModuleFile();
	~CModuleFile();

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

	enum eModuleType
	{
		MT_DawnOfWar, //!< Dawn of war / WA / DC / SS .module file
		MT_CompanyOfHeroesEarly, //!< Company of Heroes SP pre-release beta / early versions of MP beta
		MT_CompanyOfHeroes //!< Company of Heroes MP beta / SP demo / release / Opposing Fronts
	};

	void Create(eModuleType ModuleType);

	/*!
		Resets the CModuleFile's state and parses the directives the the specified .module file.
		After calling this function, call ReloadResources() to actually load the UCSs, SGAs, etc.
		\return Returns no value, but throws a CRainmanException on error
	*/
	void LoadModuleFile(const char* sFileName, CALLBACK_ARG_OPT);
	void LoadSgaAsMod(const char* sFileName, CALLBACK_ARG_OPT);

	const static unsigned long RR_LocalisedText = 1; //!< Reload DoW UCS files / CoH UCS files
	const static unsigned long RR_DataFolders = 2; //!< Reload all data folders
	const static unsigned long RR_DataArchives = 4; //!< Reload all SGA archives
	const static unsigned long RR_RequiredMods = 8; //!< Reparse all required mods
	const static unsigned long RR_Engines = 16; //!< Reparse all engine settings
	const static unsigned long RR_DataGeneric = 32; //!< Reload the datageneric folder
	const static unsigned long RR_MapArchives = 64; //!< Reload all CoH map SGA packs
	const static unsigned long RR_All = RR_LocalisedText | RR_DataFolders | RR_DataArchives | RR_RequiredMods | RR_Engines | RR_DataGeneric | RR_MapArchives;

	/*!
		Unloads all loaded resources, and then reloads everything passed in iReloadWhat.
		If RequiredMods or Engines are re-parsed, then iReloadWhatRequiredMods and iReloadWhatEngines are used to specify
		what resources to load for these.
		\return Returns no value, but throws a CRainmanException on error
	*/
	void ReloadResources(unsigned long iReloadWhat = RR_All, unsigned long iReloadWhatRequiredMods = RR_All, unsigned long iReloadWhatEngines = RR_All, CALLBACK_ARG_OPT);

	// Get / Set mod settings

	const char* GetFileMapName		() const;
	const char* GetFileMapName		();
	void		SetLocale			(const char* sLocale);
	const char* GetLocale			() const;
	eModuleType GetModuleType		() const;
	void		SetMapPackRootFolder(const wchar_t* sFolder);
	const wchar_t* GetMapPackRootFolder() const;
	const char* GetApplicationPath	() const;

	// Get / Set mod directives

#define GET_SET_DIRECTIVE(type, name) type Get ## name() const; void Set ## name(type value)

	GET_SET_DIRECTIVE(const char*, Name);
	GET_SET_DIRECTIVE(const char*, ModFolder);
	GET_SET_DIRECTIVE(const char*, LocaleFolder);
	GET_SET_DIRECTIVE(const char*, Description);
	GET_SET_DIRECTIVE(const char*, TextureFe);
	GET_SET_DIRECTIVE(const char*, TextureIcon);
	GET_SET_DIRECTIVE(const char*, UiName);
	GET_SET_DIRECTIVE(const char*, DllName);
	GET_SET_DIRECTIVE(long, VersionMajor);
	GET_SET_DIRECTIVE(long, VersionMinor);
	GET_SET_DIRECTIVE(long, VersionRevision);

#undef GET_SET_DIRECTIVE

	class RAINMAN_API CFolderHandler
	{
	public:
		signed long GetNumber() const;
		void SetNumber(signed long iNumber);

		const char* GetName() const;
		void SetName(const char* sFileName);

	protected:
		CFolderHandler();
		~CFolderHandler();

		friend class CModuleFile;
		signed long m_iNumber;
		char* m_sName;
	};

	void NewFolder(signed long iNumber, const char* sName);
	size_t GetFolderCount();
	CFolderHandler* GetFolder(size_t iId);
	void DeleteFolder(size_t iId);

	friend void CModuleFile_ArchiveForEach(IDirectoryTraverser::IIterator*, void*);
	friend void CModuleFile_ArchiveForEachNoErrors(IDirectoryTraverser::IIterator*, void*);
	class RAINMAN_API CArchiveHandler
	{
	public:
		signed long GetNumber() const;
		void SetNumber(signed long iNumber);

		const char* GetFileName() const;
		void SetFileName(const char* sFileName);

		CSgaFile* GetFileHandle() const;
	protected:
		CArchiveHandler();
		~CArchiveHandler();

		friend class CModuleFile;
		friend void CModuleFile_ArchiveForEach(IDirectoryTraverser::IIterator*, void*);
		friend void CModuleFile_ArchiveForEachNoErrors(IDirectoryTraverser::IIterator*, void*);
		signed long m_iNumber;
		char* m_sName;
		CSgaFile* m_pHandle; //!< This is a "resource"
	};

	void NewArchive(signed long iNumber, const char* sName);
	size_t GetArchiveCount();
	CArchiveHandler* GetArchive(size_t iId);

	//! Puts the full file path of an archive into a user supplied buffer
	/*!
		\param sOutput If non-NULL, the file path will be placed into this buffer
		\return The length of the file path, in characters (including the terminating 0)
	*/
	size_t GetArchiveFullPath(size_t iId, char* sOutput);

	void DeleteArchive(size_t iId);

	class RAINMAN_API CRequiredHandler
	{
	public:
		signed long GetNumber() const;
		void SetNumber(signed long iNumber);

		const char* GetFileName() const;
		void SetFileName(const char* sFileName);

		const CModuleFile* GetModHandle() const;
		CModuleFile* GetModHandle();
	protected:
		CRequiredHandler();
		~CRequiredHandler();

		friend class CModuleFile;
		signed long m_iNumber;
		char* m_sName;
		CModuleFile* m_pHandle; //!< This is a "resource"
	};

	void NewRequired(signed long iNumber, const char* sName);
	size_t GetRequiredCount() const;
	CRequiredHandler* GetRequired(size_t iId);
	const CRequiredHandler* GetRequired(size_t iId) const;
	void DeleteRequired(size_t iId);

	class RAINMAN_API CCompatibleHandler
	{
	public:
		signed long GetNumber() const;
		void SetNumber(signed long iNumber);

		const char* GetFileName() const;
		void SetFileName(const char* sFileName);

	protected:
		CCompatibleHandler();
		~CCompatibleHandler();

		friend class CModuleFile;
		signed long m_iNumber;
		char* m_sName;
	};

	void NewCompatible(signed long iNumber, const char* sName);
	size_t GetCompatibleCount();
	CCompatibleHandler* GetCompatible(size_t iId);
	void DeleteCompatible(size_t iId);

	class RAINMAN_API CCohDataSource
	{
	public:
		const char* GetToc() const;
		void SetToc(const char* sToc);

		const char* GetOption() const;
		void SetOption(const char* sOption);

		signed long GetNumber() const;
		void SetNumber(signed long iNumber);

		const char* GetFolder() const;
		void SetFolder(const char* sFolder);

		void NewArchive(signed long iNumber, const char* sName);
		size_t GetArchiveCount();
		CArchiveHandler* GetArchive(size_t iId);
		void DeleteArchive(size_t iId);

		bool IsLoaded() const;

		inline bool IsFolderWritable() const { return m_bCanWriteToFolder; }

	protected:
		CCohDataSource();
		~CCohDataSource();

		friend class CModuleFile;
		signed long m_iNumber;
		char* m_sToc;
		char* m_sOption;
		char* m_sFolder;
		bool m_bIsLoaded;
		bool m_bCanWriteToFolder;
		std::vector<CArchiveHandler*> m_vArchives;
	};

	void NewDataSource(const char* sToc, const char* sOption, signed long iNumber);
	size_t GetDataSourceCount();
	CCohDataSource* GetDataSource(size_t iId);
	void DeleteDataSource(size_t iId);

	friend void CModuleFile_UcsForEach(IDirectoryTraverser::IIterator*, void*);
	class RAINMAN_API CUcsHandler //!< This is a "resource"
	{
	public:
		const char* GetFileName() const;
		void SetFileName(const char* sFileName);

		CUcsFile* GetUcsHandle();
		const CUcsFile* GetUcsHandle() const;
	protected:
		CUcsHandler();
		~CUcsHandler();

		friend class CModuleFile;
		friend void CModuleFile_UcsForEach(IDirectoryTraverser::IIterator*, void*);
		char* m_sName; //!< This is a "resource" (because CUcsHandler is)
		CUcsFile* m_pHandle; //!< This is a "resource" (because CUcsHandler is)
	};

	void NewUCS(const char* sName, CUcsFile* pUcs = 0);
	const wchar_t* ResolveUCS(const char* sDollarString);
	const wchar_t* ResolveUCS(const wchar_t* sDollarString);
	/*!
		\return Returns a valid pointer, or 0 if the key could not be found.
	*/
	const wchar_t* ResolveUCS(unsigned long iStringID);
	size_t GetUcsCount() const;
	CUcsHandler* GetUcs(size_t iId);
	const CUcsHandler* GetUcs(size_t iId) const;
	void DeleteUcs(size_t iId);

	size_t GetEngineCount() const;
	CModuleFile* GetEngine(size_t iId);
	const CModuleFile* GetEngine(size_t iId) const;

	long GetSgaOutputVersion();

	inline CFileMap* GetFileMap() { return m_pNewFileMap; }
	inline CFileSystemStore* GetFileSystem() { return m_pFSS; }
	inline bool IsFauxModule() { return m_bIsFauxModule; }

protected:
	// Functions
	void _Clean();
	void _CleanResources();
	/*!
		Will duplicate a string, replacing the following: <br>
		%LOCALE% -> Locale\{sLocale} <br>
		%TEXTURE-LEVEL% -> Full <br>
		%SOUND-LEVEL% -> Full <br>
		%MODEL-LEVEL% -> High <br>
		If sAlsoAppend is specified then that will be appended to the result.
		\return Returns a valid string, or 0 on error
	*/
	char* _DawnOfWarRemoveDynamics(const char* sStr, const char* sAlsoAppend = 0);
	void _DoLoadFolder(const char* sFullPath, bool bIsDefaultWrite, unsigned short iNum, const char* sTOC, const char *sUiName = 0, CALLBACK_ARG_OPT, bool* bIsWritable = 0);
	void _DoLoadDataGeneric(CALLBACK_ARG_OPT);
	void _DoLoadArchive(const char* sFullPath, CSgaFile** pSga, unsigned short iNum, const char* sUiName, CALLBACK_ARG_OPT);
	void _DoLoadCohEngine(const char* sFolder, const char* sUiName, unsigned long iLoadWhat, unsigned short iNum, CALLBACK_ARG_OPT);
	bool _IsToBeLoaded(CCohDataSource* pDataSource);

	// Module settings
	eModuleType m_eModuleType;
	char* m_sLocale; // This is NOT cleaned by _Clean()
	char* m_sApplicationPath;
	char* m_sFilename;
	char* m_sFileMapName;
	CModuleFile* m_pParentModule;
	unsigned short int m_iFileMapModNumber;
	wchar_t* m_sScenarioPackRootFolder;
	char* m_saScenarioPackRootFolder;
	bool m_bIsFauxModule;

	// Simple fields in .module files
	char* m_sUiName; // MT_DawnOfWar & MT_CompanyOfHeroes
	char* m_sName; // MT_CompanyOfHeroesEarly & MT_CompanyOfHeroes
	char* m_sDescription; // MT_DawnOfWar & MT_CompanyOfHeroes & MT_CompanyOfHeroesEarly
	char* m_sDllName; // MT_DawnOfWar & MT_CompanyOfHeroes & MT_CompanyOfHeroesEarly
	char* m_sModFolder; // MT_DawnOfWar & MT_CompanyOfHeroes & MT_CompanyOfHeroesEarly
	signed long m_iModVersionMajor; // MT_DawnOfWar & MT_CompanyOfHeroes & MT_CompanyOfHeroesEarly
	signed long m_iModVersionMinor; // MT_DawnOfWar & MT_CompanyOfHeroes & MT_CompanyOfHeroesEarly (Optional)
	signed long m_iModVersionRevision; // MT_DawnOfWar & MT_CompanyOfHeroes & MT_CompanyOfHeroesEarly (Optional)
	char* m_sTextureFe; // MT_DawnOfWar
	char* m_sTextureIcon; // MT_DawnOfWar
	char* m_sPatcherUrl; // MT_CompanyOfHeroesEarly
	char* m_sLocFolder; // MT_CompanyOfHeroes
	char* m_sScenarioPackFolder; // MT_CompanyOfHeroes

	// Object collections for MT_DawnOfWar & MT_CompanyOfHeroesEarly
	std::vector<CFolderHandler*> m_vFolders;
	std::vector<CArchiveHandler*> m_vArchives;

	// Object collections for MT_DawnOfWar
	std::vector<CRequiredHandler*> m_vRequireds;
	std::vector<CCompatibleHandler*> m_vCompatibles;

	// Object collections for MT_CompanyOfHeroes
	std::vector<CCohDataSource*> m_vDataSources;

	// Object collections for all
	std::vector<CModuleFile*> m_vEngines; //!< This is a "resource"
	std::vector<CUcsHandler*> m_vLocaleTexts; //!< This is a "resource"

	// Internal usefuls
	CFileSystemStore* m_pFSS;
	char* m_sCohThisModFolder;

	// Resource holders
	CDoWFileView* m_pFileMap;
	CFileMap* m_pNewFileMap;
};

#endif

