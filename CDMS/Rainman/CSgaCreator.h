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

#ifndef _C_SGA_CREATOR_H_
#define _C_SGA_CREATOR_H_

#include "gnuc_defines.h"
#include "IFileStore.h"
#include "IDirectoryTraverser.h"
#include "Api.h"
#include <vector>

//! Creates DoW and CoH SGA archives
/*
	Currently has all static methods; may one day have non-static members,
	however having it all in one class makes it easy to organise.
*/
class RAINMAN_API CSgaCreator
{
public:

	//! Stores information about files that are to be packed
	class CInputFile
	{
	private:
		friend class CSgaCreator;
		size_t iNameOffset; //!< Where in sFullPath the actual file name begins
		tLastWriteTime oLastWriteTime;
		char* sFullPath; //!< The full path of the file in pFileStore
		IFileStore* pFileStore;

	public:
		static bool OpLT(CInputFile* oA, CInputFile* oB); //!< Is oA less than oB?
		~CInputFile();
	};

	//! Stores information about directories that are to be packed
	class CInputDirectory
	{
	private:
		friend class CSgaCreator;
		std::vector<CSgaCreator::CInputDirectory*> vDirsList; //!< Sub-directories
		std::vector<CSgaCreator::CInputFile*> vFilesList;
		char* sNameFull;

	public:
		static bool OpLT(CInputDirectory* oA, CInputDirectory* oB); //!< Is oA less than oB?
		void FullCount(size_t& iDirCount, size_t& iFileCount); //!< Count the number of directories and files
		~CInputDirectory();
	};

	//! Create a new SGA archive
	/*!
		\param[in] pDirectory The input directory to turn into an SGA
		\param[in] pStore The file store tied to pDirectory
		\param[in] sOutputFile The full name and path of the file to create
		\param[in] iVersion Set to 2 to create DoW compatible archives; set to 4 to create CoH compatible archives
		\return Returns no value but throws an= CRainmanException on error
	*/
	static void CreateSga(IDirectoryTraverser::IIterator* pDirectory, IFileStore* pStore, const char* sTocName, const char* sOutputFile, long iVersion = 4, const char* sArchiveTitle = 0, const char* sTocTitle = 0);

private:
	static CInputDirectory* _ScanDirectory(IDirectoryTraverser::IIterator* pDirectory, IFileStore* pStore, size_t iDirBaseL = 0);
};

#endif

