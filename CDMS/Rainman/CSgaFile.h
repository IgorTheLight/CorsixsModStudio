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

#ifndef _C_SGA_FILE_H_
#define _C_SGA_FILE_H_

#include "gnuc_defines.h"
#include "IFileStore.h"
#include "IDirectoryTraverser.h"
#include "CMemoryStore.h"
#include "Api.h"
#include <wchar.h>
#include <vector>

//! SGA (Game data archive) file
/*!
	The SGA file is the archive file used in both DoW and CoH.
	The archive cannot be modified once loaded; the class CSgaCreator should be used
	to create SGA archives.

	The SGA format is documented at http://www.relic.com/rdn/wiki/DOWFileFormats/SGA

	\todo Tidy up the code and make it less of a VB6 port
*/
class RAINMAN_API CSgaFile : public IFileStore, public IDirectoryTraverser
{
public:
	CSgaFile(void);
	~CSgaFile(void);

	inline unsigned long GetVersion() { return m_SgaHeader.iVersion; }

	//! Load an archive from a stream
	/*!
		Loads an SGA archive starting from the current location in the stream
		\param[in] pStream The stream from which to load the archive. 
		\param[in] oWriteTime The last modification date of the archive file. Older (DoW) SGA archives do not store the last modification date of each file in the archive, and will use this value for each file.
		\return Returns no value, but will throw a CRainmanException on error
	*/
	void Load(IFileStore::IStream *pStream, tLastWriteTime oWriteTime = GetInvalidWriteTime());

	//! Get the input stream used for the archive
	/*!
		Gets the input stream passed to VInit(). However, if VInit() threw an exception then the value is not remembered.
		\return Returns the value passed, or 0 if the class has been reset. Does not throw a CRainmanException
		\sa VInit()
	*/
	IFileStore::IStream *GetInputStream();

	// ## IFileStore interface ##

	//! Initialize the file store interface
	/*!
		\param[in] pInputStream Must be a IFileStore::IStream* . This pointer is remembered, and then used whenever a file inside the SGA archive is opened - as such do not delete the pointer until you are finished using the SGA archive. You can use GetInputStream() to get this pointer and delete it right before you delete this class.
		\return Retuns no value but will throw a CRainmanException on error
		\sa IFileStore::VInit
	*/
	virtual void VInit(void* pInputStream = 0);

	//! Open an input/read stream
	/*!
		\sa IFileStore::VOpenStream
		\return Will actually return a CSgaFile::CStream* via runtime polymorphism.
	*/
	virtual IFileStore::IStream* VOpenStream(const char* sIdentifier);

	//! A input/read stream implementation
	/*!
		This class wraps a IFileStore::IStream* (CMemoryStore::CStream*) and deletes the data from memory afterwards.
		\sa IFileStore::IStream
	*/
	class RAINMAN_API CStream : public IFileStore::IStream
	{
	protected:
		friend class CSgaFile;
		CStream();
		IFileStore::IStream* m_pRawStream;
		char* m_pData;

	public:
		virtual ~CStream();

		virtual void VRead(unsigned long iItemCount, unsigned long iItemSize, void* pDestination);
		virtual void VSeek(long iPosition, IFileStore::IStream::SeekLocation SeekFrom = SL_Current);
		virtual long VTell();
	};

	// ## IDirectoryTraverser interface ##

	//! Iterate over the contents of a directory
	/*!
		\sa IDirectoryTraverser::VIterate()
		\return Will return a CSgaFile::CIterator* via runtime polymorphism
	*/
	virtual IDirectoryTraverser::IIterator* VIterate(const char* sPath);

	//! Get the number of entry points / SGA "Table of contents" available to iterate over
	/*!
		For SGA archives, entry points are table of contents entries.
		\sa IDirectoryTraverser::VGetEntryPointCount()
	*/
	virtual unsigned long VGetEntryPointCount();

	//! Get the name of a specified entry point / table of contents
	/*!
		For SGA archives, entry points are table of contents entries.
		Almost every DoW / CoH archive has a single entry point with a name of "Data"
		\sa IDirectoryTraverser::VGetEntryPoint()
	*/
	virtual const char* VGetEntryPoint(unsigned long iID);

	//! Create a new folder somewhere in the directory structure
	/*!
		SGA archives are not designed to be modified; use CSgaCreator to create a new SGA archive.
		\return Always throws a CRainmanException.
		\sa IDirectoryTraverser::VCreateFolderIn()
	*/
	virtual void VCreateFolderIn(const char* sPath, const char* sNewFolderName);

	//! An SGA directory contents iterator implementation
	/*!
		\sa IDirectoryTraverser::IIterator
	*/
	class RAINMAN_API CIterator : public IDirectoryTraverser::IIterator
	{
	protected:
		friend class CSgaFile;

		//! Constructor
		/*!
			\param[in] iDir The ID of the directory to iterate over
			\param[in] pSga A pointer to the Sga file which created the iterator
			\return Returns no value but may throw a CRainmanException
		*/
		CIterator(long iDir, CSgaFile* pSga);

		/*!
			The ID of the directory being iterated over
		*/
		long m_iParentDirectory;

		/*!
			Behaviour dependant on m_iTraversingWhat
		*/
		long m_iCurrentItem;

		/*!
			+0 implies m_iCurrentItem is a directory ID
			+1 implies m_iCurrentItem is a file ID
			-1 implies error / end of directory
		*/
		long m_iTraversingWhat;

		/*!
			The "parent" SGA file
		*/
		CSgaFile* m_pSga;

		/*!
			The full path of the directory being iterated, including a forward slash on the end
		*/
		char *m_sParentPath;

		/*!
			The full path of the current item; no forward slash on the end for directories
		*/
		char *m_sFullPath;

	public:
		virtual ~CIterator();

		//! Returns the type of the current object
		/*!
			\sa IDirectoryTraverser::IIterator::VGetType()
		*/
		virtual IDirectoryTraverser::IIterator::eTypes VGetType();

		//! Create an iterator to iterate over the current directory item
		/*!
			\sa IDirectoryTraverser::IIterator::VOpenSubDir()
			\return Will return a CSgaFile::CIterator* via runtime polymorphism
		*/
		virtual IDirectoryTraverser::IIterator* VOpenSubDir();

		//! Create a stream to read from the current file item
		/*!
			\sa IDirectoryTraverser::IIterator::VOpenFile()
			\return Will return a CSgaFile::CStream* via runtime polymorphism
		*/
		virtual IFileStore::IStream* VOpenFile();

		//! Get the name of the current item
		/*!
			\sa IDirectoryTraverser::IIterator::VGetName()
		*/
		virtual const char* VGetName();

		//! Get the full path of the current item
		/*!
			\sa IDirectoryTraverser::IIterator::VGetFullPath()
		*/
		virtual const char* VGetFullPath();

		//! Get the directory path of the current item (aka. the name of the directory being iterated)
		/*!
			\sa IDirectoryTraverser::IIterator::VGetDirectoryPath()
		*/
		virtual const char* VGetDirectoryPath();

		//! Get the time at which the current file item was last modified
		/*!
			With older (DoW) SGA archives that do not record a last modified time then this function will return
			the last modification date of the archive file itself (if passed), or throw a CRainmanException if one was
			not passed.
			\sa IDirectoryTraverser::IIterator::VGetLastWriteTime()
		*/
		virtual tLastWriteTime VGetLastWriteTime();

		//! Move onto the next item in the current directory
		/*!
			\sa IDirectoryTraverser::IIterator::VNextItem()
		*/
		virtual IDirectoryTraverser::IIterator::eErrors VNextItem();
	};

	//! Get the last modification date of a file
	/*!
		With older (DoW) SGA archives that do not record a last modified time then this function will return
		the last modification date of the archive file itself (if passed), or throw a CRainmanException if one was
		not passed.
		\sa IDirectoryTraverser::VGetLastWriteTime()
	*/
	tLastWriteTime VGetLastWriteTime(const char* sPath);

protected:
	friend class CIterator;

	//! The file header is at the start of the SGA archive
	struct _SgaFileHeader
	{
		char* sIdentifier; //!< 8 bytes "_ARCHIVE"
		unsigned long iVersion; //!< DoW is v2 , CoH is v4
		long* iToolMD5; //!< First MD5
		wchar_t* sArchiveType; //!< Unicode string (128 bytes - 64 * wchar)
		long* iMD5; //!< Second MD5
		unsigned long iDataHeaderSize;
		unsigned long iDataOffset;
		unsigned long iPlatform; //!< v4 only; 1 = win32
	};

	#pragma pack( push, sga_formats, 1 )
	struct _SgaDataHeaderInfo
	{
		unsigned long iToCOffset; //!< Where in the data header ToCs start
		unsigned short int iToCCount;
		unsigned long iDirOffset; //!< Where in the data header directories start
		unsigned short int iDirCount;
		unsigned long iFileOffset; //!< Where in the data header files start
		unsigned short int iFileCount;
		unsigned long iItemOffset; //!< Where in the data header strings start
		unsigned short int iItemCount;
	};

	struct _SgaToC
	{
		char sAlias[64];
		char sBaseDirName[64];
		short unsigned int iStartDir;
		short unsigned int iEndDir;
		short unsigned int iStartFile;
		short unsigned int iEndFile;
		unsigned long iFolderOffset; //!< Unknown
	};

	struct _SgaDirInfo
	{
		unsigned long iNameOffset;
		short unsigned int iSubDirBegin;
		short unsigned int iSubDirEnd;
		short unsigned int iFileBegin;
		short unsigned int iFileEnd;
	};

	struct _SgaFileInfo
	{
		unsigned long iNameOffset;
		unsigned long iFlags; //!< 0x00 = uncompressed, 0x10 = zlib large file, 0x20 = zlib small file (< 4kb)
		unsigned long iDataOffset;
		unsigned long iDataLengthCompressed;
		unsigned long iDataLength;
	};

	struct _SgaFileInfo4
	{
		unsigned long iNameOffset;
		unsigned long iDataOffset;
		unsigned long iDataLengthCompressed;
		unsigned long iDataLength;
		unsigned long iModificationTime;
		unsigned short iFlags;
	};

	#pragma pack( pop, sga_formats)

	struct _SgaDirInfoExt
	{
		/*!
			Positive values refer to directories, negative values refer to Table of contents <br>
			eg. 4 = parent is directory #4 <br>
			eg. -1 = parent is ToC #1

			IDs are 16 bit unsigned int, so 32 bit signed int will allow that and negative numbers.
		*/
		signed long iParent;
		char* sShortName;
		char* sName;
	};

	struct _SgaFileInfoExt
	{
		signed long iParent;
		char* sName;
	};

	//! Directory Hash map entry
	/*!
		Access times become alot lower when using a hash map to resolve a directory path/name to an ID.
		The CRC is case insensitive and thus uses a slightly modified version of the CRC algorithm (which is just as fast as a normal CRC).
	*/
	struct _SgaDirHash
	{
		unsigned long iCRC;
		unsigned short int iID;
	};

	_SgaFileHeader m_SgaHeader; //!< The file header
	//! The data header info
	/*!
		This pointer is to the start of the data header, which is one large block of memory.
		As such, deleting this pointer will free the entire data header.
	*/
	_SgaDataHeaderInfo* m_pDataHeaderInfo;
	_SgaToC* m_pSgaToCs; //!< Array of ToC entries
	_SgaDirInfo* m_pSgaDirs; //!< Array of directory entries
	_SgaDirInfoExt* m_pSgaDirExts; //!< Array of directory extra information
	_SgaDirHash* m_pSgaDirHashMap; //!< Hash map for quickly resolving directory name -> directory ID
    _SgaFileInfo* m_pSgaFiles; //!< An array of file entries for v2 SGAs
	_SgaFileInfo4* m_pSga4Files; //!< An array of file entries for v4 SGAs
	_SgaFileInfoExt* m_pSgaFileExts; //!< Array of file extra information
	tLastWriteTime m_oSgaWriteTime; //!< The last modification date of the SGA file

	//! Free all memory used by the class and reset the class to a blank state
	/*!
		\return Returns no value and never throws a CRainmanException
	*/
	void _Clean();

	IFileStore::IStream *m_pFileStoreInputStream; //!< The input stream passed by VInit()
};

#endif

