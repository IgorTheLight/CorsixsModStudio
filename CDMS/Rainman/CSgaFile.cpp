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

#include "CSgaFile.h"

#include <memory.h>
#include <string.h>
#include "..\zLib/zlib.h"
extern "C" {
#include "md5.h"
}
#include "memdebug.h"
#include <stdlib.h>
#include <search.h>
#include "Exception.h"

CSgaFile::CSgaFile(void)
{
	/*
		Constructor sets everything to 0 so that if the destructor is called immediatly,
		no memory is accidently freed.
	*/
	memset(&m_SgaHeader, 0, sizeof(_SgaFileHeader));
	m_pDataHeaderInfo = 0;

	m_pSgaToCs = 0;
	m_pSgaDirs = 0;
	m_pSgaDirExts = 0;
	m_pSgaDirHashMap = 0;
    m_pSgaFiles = 0;
	m_pSga4Files = 0;
	m_pSgaFileExts = 0;
	m_pFileStoreInputStream = 0;
	m_oSgaWriteTime = GetInvalidWriteTime();

	m_pFileStoreInputStream = 0;
}

CSgaFile::~CSgaFile(void)
{
	_Clean();
}

void CSgaFile::VCreateFolderIn(const char* sPath, const char* sNewFolderName)
{
	/*
		SGA files simply cannot be modified, but this method is reqiured by IDirectoryTraverser.
	*/
	throw new CRainmanException(__FILE__, __LINE__, "Cannot modify SGA files with this class; use CSgaCreator instead");
}

IFileStore::IStream *CSgaFile::GetInputStream()
{
	if(m_pFileStoreInputStream) return m_pFileStoreInputStream;
	throw new CRainmanException(__FILE__, __LINE__, "No file store given");
}

static char* mystrdup(const char* sStr)
{
	/*
		Equivalent to the standard library's strdup(), except it uses "new" instead of "malloc"
		(and thus "delete" instead of "free")
	*/
	char* s = new char[strlen(sStr) + 1];
	if(s == 0) return 0;
	strcpy(s, sStr);
	return s;
}

#ifdef RAINMAN_GNUC
static int CompareDirHash(const void *elem1, const void *elem2)
#else
static int __cdecl CompareDirHash(const void *elem1, const void *elem2)
#endif
{
	/*
		Sorting method to allow qsort() to sort the hash map

		Cannot do elem1 - elem2 even though it would appear to work:
		with elem1 of 3 and elem2 of 5, e1 - e2 = -2 = e1 < e2
		with elem1 of 13 and elem2 of 3,e1 - e2 = 10 = e1 > e2
		However:
		with elem1 of 2 and elem2 of 3, e1 - e2 = INT_MAX = e1 > e2 , which is incorrect
	*/
	if( (*(((const unsigned long*)elem1)) < (*((const unsigned long*)elem2))) ) return -1;
	if( (*(((const unsigned long*)elem1)) > (*((const unsigned long*)elem2))) ) return +1;
	return 0;
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

#endif

void CSgaFile::Load(IFileStore::IStream *pStream, tLastWriteTime oWriteTime)
{
	_Clean(); // Reset the class to a blank state

	if(pStream == 0) QUICK_THROW("No stream");
	m_oSgaWriteTime = oWriteTime;

	/*
		Load the header and verify it bit by bit (incase we were passed something which is not an SGA archive).
		While we do want to be leniant over file format specifics, we also do not want to crash when passed a
		file full of random data.
	*/
	m_SgaHeader.sIdentifier = new char[9];
	if(m_SgaHeader.sIdentifier == 0)
	{
		_Clean();
		QUICK_THROW("Failed to allocate memory");
	}
	memset(m_SgaHeader.sIdentifier, 0, 9);
	try
	{
		pStream->VRead(8, 1, m_SgaHeader.sIdentifier);
	}
	QCCT("Failed to read from stream")

	if(strcmp(m_SgaHeader.sIdentifier, "_ARCHIVE") != 0)
	{
		_Clean();
		throw new CRainmanException(0, __FILE__, __LINE__, "File identifier is \"%s\", should be \"_ARCHIVE\"", m_SgaHeader.sIdentifier);
	}

	try
	{
		pStream->VRead(1, sizeof(unsigned long), &m_SgaHeader.iVersion);
	}
	QCCT("Failed to read from stream")

	if(m_SgaHeader.iVersion != 2 && m_SgaHeader.iVersion != 4)
	{
		/*
			Only versions 2 and 4 are supported; IC may have used version 1 or 2 - I have not seen any IC SGAs.
			An early version of CoH or DoW:DC may have used version 3 - only Relic knows :/
		*/
		_Clean();
		throw new CRainmanException(0, __FILE__, __LINE__, "Version %lu is not supported (only 2 and 4 are)", m_SgaHeader.iVersion);
	}

	m_SgaHeader.iToolMD5 = new long[4];
	if(m_SgaHeader.iToolMD5 == 0)
	{
		_Clean();
		QUICK_THROW("Failed to allocate memory")
	}

	try
	{
		pStream->VRead(4, 4, m_SgaHeader.iToolMD5);
	}
	QCCT("Failed to read from stream")

	m_SgaHeader.sArchiveType = new wchar_t[65];
	if(m_SgaHeader.sArchiveType == 0)
	{
		_Clean();
		QUICK_THROW("Failed to allocate memory")
	}
	memset(m_SgaHeader.sArchiveType, 0, sizeof(wchar_t) * 65);

	try
	{
		pStream->VRead(64, 2, m_SgaHeader.sArchiveType);
	}
	QCCT("Failed to read from stream")

	m_SgaHeader.iMD5 = new long[4];
	if(m_SgaHeader.iMD5 == 0)
	{
		_Clean();
		QUICK_THROW("Failed to allocate memory")
	}
	try
	{
		pStream->VRead(4, sizeof(long), m_SgaHeader.iMD5);
		pStream->VRead(1, sizeof(long), &m_SgaHeader.iDataHeaderSize);
		pStream->VRead(1, sizeof(long), &m_SgaHeader.iDataOffset);

		if(m_SgaHeader.iVersion == 4) // v4 SGAs have an extra 4 byte value here
		{
			pStream->VRead(1, sizeof(long), &m_SgaHeader.iPlatform);
		}
	}
	QCCT("Failed to read from stream")

	if(m_SgaHeader.iVersion == 4 && m_SgaHeader.iPlatform != 1)
	{
		/*
			Version four SGA files store a "Platform" value. A value of 1 appears to mean win32, x86
			or little endian. Until we see a value of 2, we have no idea of what this means. As such
			we should throw an error, as it may change several things.
		*/
		_Clean();
		throw new CRainmanException(0, __FILE__, __LINE__, "Platform #%lu is not supported (only platform #1 is supported, so please show this to programmers)", m_SgaHeader.iPlatform);
	}

	/*
		File header has now been read; now read the data header in one foul swoop
		It is more efficient to read the data header as one item and have all of it in memory.
		This way, for example, file and folder names do not need two VSeek()s and byte by byte string reading
		as a pointer can simply be set to the location in memory.
	*/
	unsigned char *pDataHeader = new unsigned char[m_SgaHeader.iDataHeaderSize];
	if(pDataHeader == 0)
	{
		_Clean();
		throw new CRainmanException(0, __FILE__, __LINE__, "Cannot allocate %lu bytes for data header", m_SgaHeader.iDataHeaderSize);
	}

	try
	{
		pStream->VRead(m_SgaHeader.iDataHeaderSize, 1, (void*)pDataHeader );
	}
	catch(CRainmanException* pE)
	{
		delete[] pDataHeader;
		_Clean();
		throw new CRainmanException(pE, __FILE__, __LINE__, "Cannot read %lu bytes of data header", m_SgaHeader.iDataHeaderSize);
	}

	/*
		The final validation is checking the MD5 checksum matches. If this matches then there is a very high chance
		that the file is an SGA archive. The second MD5 is not checked though; as it invloves reading in the entire
		archive file, which takes a long time, and the first checksum almost definatly proves that it is an SGA archive.
		Relic may want to verify the second second checksum to make sure you haven't been fiddling with the archive, but
		I do not see anything wrong with that.
	*/
	MD5Context md5MainKey;
	MD5InitKey(&md5MainKey, "DFC9AF62-FC1B-4180-BC27-11CCE87D3EFF");
	
	MD5Update(&md5MainKey, pDataHeader, m_SgaHeader.iDataHeaderSize);
	unsigned char pMD5Main[17];
	pMD5Main[16] = 0;
	MD5Final(pMD5Main, &md5MainKey);
	int iMD5Match = strncmp((const char*)pMD5Main, (const char*)m_SgaHeader.iMD5, 16);
	if(iMD5Match != 0)
	{
		delete[] pDataHeader;
		_Clean();
		throw new CRainmanException(__FILE__, __LINE__, "Header MD5 does not match");
	}

	/*
		Allocate all the memory we need at once - might be more efficient?
		(and do some overflow checks)
	*/
	m_pDataHeaderInfo = (_SgaDataHeaderInfo*) pDataHeader;

	if( (sizeof(_SgaDirHash) * m_pDataHeaderInfo->iDirCount) < m_pDataHeaderInfo->iDirCount )
		{_Clean(); QUICK_THROW("Potential buffer overflow, aborting") }
	if( (sizeof(_SgaDirInfoExt) * m_pDataHeaderInfo->iDirCount) < m_pDataHeaderInfo->iDirCount )
		{_Clean(); QUICK_THROW("Potential buffer overflow, aborting") }
	if( (sizeof(_SgaFileInfoExt) * m_pDataHeaderInfo->iFileCount) < m_pDataHeaderInfo->iFileCount )
		{_Clean(); QUICK_THROW("Potential buffer overflow, aborting") }

	m_pSgaDirHashMap = new _SgaDirHash[m_pDataHeaderInfo->iDirCount];
	if(m_pSgaDirHashMap == 0) {_Clean(); QUICK_THROW("Failed to allocate memory") }

	m_pSgaDirExts = new _SgaDirInfoExt[m_pDataHeaderInfo->iDirCount];
	if(m_pSgaDirExts == 0) {_Clean(); QUICK_THROW("Failed to allocate memory") }
	for(unsigned short i = 0; i < m_pDataHeaderInfo->iDirCount; ++i)
	{
		m_pSgaDirExts[i].iParent = -1;
		m_pSgaDirExts[i].sName = 0;
		m_pSgaDirExts[i].sShortName = 0;
	}

	m_pSgaFileExts = new _SgaFileInfoExt[m_pDataHeaderInfo->iFileCount];
	if(m_pSgaFileExts == 0) {_Clean(); QUICK_THROW("Failed to allocate memory") }
	for(unsigned short i = 0; i < m_pDataHeaderInfo->iFileCount; ++i)
	{
		m_pSgaFileExts[i].iParent = -1;
		m_pSgaFileExts[i].sName = 0;
	}

	/*
		Read the ToCs.
		I still dont quite understand ToCs as I have been unable to find an SGA with more than 1 ToC.
		As such this code may well be horribly wrong. If you have experience with multi-ToC SGA files
		then please contact me and/or check the ToC related code.
	*/
	m_pSgaToCs = (_SgaToC*)(pDataHeader + m_pDataHeaderInfo->iToCOffset);

	for(unsigned short i = 0; i < m_pDataHeaderInfo->iToCCount; ++i)
	{
		for(unsigned short j = m_pSgaToCs[i].iStartDir; j != m_pSgaToCs[i].iEndDir; ++j)
		{
			m_pSgaDirExts[j].iParent = -(i+1);
		}

		for(unsigned short j = m_pSgaToCs[i].iStartFile; j != m_pSgaToCs[i].iEndFile; ++j)
		{
			m_pSgaFileExts[j].iParent = -(i+1);
		}
	}

	/*
		Read Directories.
		Relic's CoH code requires that directories and files be in alphabetic order,
		which suggests that they do a binary chop search using strcmp() / stricmp().
		I've gone for a modifed CRC hash table (modified to be case independant, but
		take the same time to do as a normal CRC) with binary chop searching. This
		should be faster than:
		a) going over each directory one by one (obvious reaons)
		b) binary chop with strcmp() / stricmp() (integer comparions are cheap - string compisons are not so)
	*/
	m_pSgaDirs = (_SgaDirInfo*)(pDataHeader + m_pDataHeaderInfo->iDirOffset);

    for(unsigned short i = 0; i < m_pDataHeaderInfo->iDirCount; ++i)
	{
		m_pSgaDirExts[i].sName = (char*)(pDataHeader + m_pDataHeaderInfo->iItemOffset + m_pSgaDirs[i].iNameOffset);

		char *sTmp = strrchr(m_pSgaDirExts[i].sName, '\\');
		if(sTmp) ++sTmp;
		m_pSgaDirExts[i].sShortName = sTmp ? sTmp : m_pSgaDirExts[i].sName;
		m_pSgaDirHashMap[i].iID = i;
		m_pSgaDirHashMap[i].iCRC = crc32_case_idt(crc32_case_idt(0L, Z_NULL, 0), (const Bytef*)m_pSgaDirExts[i].sName, (uInt)strlen(m_pSgaDirExts[i].sName));
        
        for(unsigned short j = m_pSgaDirs[i].iSubDirBegin; j < m_pSgaDirs[i].iSubDirEnd; ++j)
		{
            m_pSgaDirExts[j].iParent = i;
		}
        for(unsigned short j = m_pSgaDirs[i].iFileBegin; j < m_pSgaDirs[i].iFileEnd; ++j)
		{
            m_pSgaFileExts[j].iParent = i;
		}
	}
	qsort(m_pSgaDirHashMap, m_pDataHeaderInfo->iDirCount, sizeof(_SgaDirHash), &CompareDirHash);

	/*
		Read File Infos
	*/
	if(m_SgaHeader.iVersion == 2)
		m_pSgaFiles = (_SgaFileInfo*)(pDataHeader + m_pDataHeaderInfo->iFileOffset);
	else
		m_pSga4Files = (_SgaFileInfo4*)(pDataHeader + m_pDataHeaderInfo->iFileOffset);

    for(unsigned short i = 0; i < m_pDataHeaderInfo->iFileCount; ++i)
	{
		if(m_SgaHeader.iVersion == 2)
			m_pSgaFileExts[i].sName = (char*)(pDataHeader + m_pDataHeaderInfo->iItemOffset + m_pSgaFiles[i].iNameOffset);
		else
			m_pSgaFileExts[i].sName = (char*)(pDataHeader + m_pDataHeaderInfo->iItemOffset + m_pSga4Files[i].iNameOffset);
	}

	/*
		Check MD5 "tool" key
		This code is now commented out as it takes too long to do. The second MD5
		is not checked; it invloves reading in the entire archive file, which takes
		a long time, and the first checksum almost definatly proves that it is an
		SGA archive. Relic may want to verify the second second checksum to make sure
		you haven't been fiddling with the archive, but I do not see anything wrong with that.
	*/
	/*
	MD5InitKey(&md5ToolKey, "E01519D6-2DB7-4640-AF54-0A23319C56C3");
	pStream->VSeek(0, IFileStore::IStream::SL_End);
	long iDataLength = pStream->VTell() - iJumpBase;
	pStream->VSeek(iJumpBase, IFileStore::IStream::SL_Root);
	char* pDataFull = new char[1048576];
	while(iDataLength)
	{
		long iToRead = (iDataLength < 1048576) ? iDataLength : 1048576;
		pStream->VRead(1, iToRead, pDataFull);
		MD5Update(&md5ToolKey, (unsigned char*)pDataFull, iToRead);
		iDataLength -= iToRead;
	}
	delete[] pDataFull;
	unsigned char pMD5Tool[17];
	pMD5Tool[16] = 0;
	MD5Final(pMD5Tool, &md5ToolKey);
	memcpy((void*)((char*)pMD5FromHead + 0), &m_SgaHeader.iToolMD5[0], 4);
	memcpy((void*)((char*)pMD5FromHead + 4), &m_SgaHeader.iToolMD5[1], 4);
	memcpy((void*)((char*)pMD5FromHead + 8), &m_SgaHeader.iToolMD5[2], 4);
	memcpy((void*)((char*)pMD5FromHead + 12), &m_SgaHeader.iToolMD5[3], 4);
	iMD5Match = strncmp((const char*)pMD5FromHead, (const char*)pMD5Tool, 16);
	*/
	/* END */
}

#ifndef DOXY_NODOC
#undef QCCT
#endif

void CSgaFile::_Clean()
{
	// Header
	if(m_SgaHeader.sIdentifier) delete[] m_SgaHeader.sIdentifier;
	if(m_SgaHeader.iToolMD5) delete[] m_SgaHeader.iToolMD5;
	if(m_SgaHeader.sArchiveType) delete[] m_SgaHeader.sArchiveType;
	if(m_SgaHeader.iMD5) delete[] m_SgaHeader.iMD5;

	m_SgaHeader.sIdentifier = 0;
	m_SgaHeader.iVersion = 0;
	m_SgaHeader.iToolMD5 = 0;
	m_SgaHeader.sArchiveType = 0;
	m_SgaHeader.iMD5 = 0;
	m_SgaHeader.iDataHeaderSize = 0;
	m_SgaHeader.iDataOffset = 0;
	m_SgaHeader.iPlatform = 0;

	m_oSgaWriteTime = GetInvalidWriteTime();
	m_pFileStoreInputStream = 0;

	// Data header
	if(m_pSgaDirHashMap) delete[] m_pSgaDirHashMap;
	if(m_pSgaDirExts) delete[] m_pSgaDirExts;
	if(m_pSgaFileExts) delete[] m_pSgaFileExts;
	if(m_pDataHeaderInfo) delete[] (unsigned char*)m_pDataHeaderInfo;

	m_pDataHeaderInfo = 0;
	m_pSgaToCs = 0;
	m_pSgaDirs = 0;
	m_pSgaDirExts = 0;
	m_pSgaDirHashMap = 0;
    m_pSgaFiles = 0;
	m_pSga4Files = 0;
	m_pSgaFileExts = 0;
}

void CSgaFile::VInit(void* pInitData)
{
	m_pFileStoreInputStream = (IFileStore::IStream *)pInitData;
	m_bInited = m_pFileStoreInputStream ? true : false;
	if(!m_bInited) throw new CRainmanException(__FILE__, __LINE__, "No stream passed");
}

IFileStore::IStream* CSgaFile::VOpenStream(const char* sIdentifier)
{
	if(sIdentifier == 0) throw new CRainmanException(__FILE__, __LINE__, "Identifier required");
	if(m_pFileStoreInputStream == 0) throw new CRainmanException(__FILE__, __LINE__, "Input stream must have been set in VInit");
	if(m_pDataHeaderInfo == 0) throw new CRainmanException(__FILE__, __LINE__, "No data header loaded");

	size_t iDirNameLength = 0;
	unsigned short iDirID = -1;
	unsigned short iFileID = 0;

	/*
		Identify ToC
		Doesn't use a binary chop as there is usually only one ToC
	*/
	const char* sTmp = strchr(sIdentifier, '\\');
	if(sTmp == 0) throw new CRainmanException(0, __FILE__, __LINE__, "ToC could not be seperated - %s", sIdentifier);
	long iToCLength = (long)(sTmp - sIdentifier);
	long iToC = -1;
	if(iToCLength == 4 && m_SgaHeader.iVersion == 4)
	{
		iToC = 0;
	}
	else
	{
		for(unsigned short i = 0; i < m_pDataHeaderInfo->iToCCount; ++i)
		{
			/*
				Checks both length and string contents because if it only checked N characters of string
				then Data and Data2 could match each other. If strlen is included then they do not match.
			*/
			if(strlen(m_pSgaToCs[i].sAlias) == iToCLength && strnicmp(m_pSgaToCs[i].sAlias, sIdentifier, iToCLength) == 0)
			{
				iToC = i;
				break;
			}
		}
		if(iToC == -1) throw new CRainmanException(0, __FILE__, __LINE__, "ToC could not be found - %s", sIdentifier);
	}
	sIdentifier += (iToCLength + 1);
	const char* sFileNameBegin = strrchr(sIdentifier, '\\');

	/*
		Identify Correct Folder
		Binary chops the entire directory structure using the directory hash map. This is very fast to do.
		iBSL = Binary search lower-bound
		iBSM = Binary search middle
		iBSH = Binary search upper-bound
	*/
	unsigned short iBSL, iBSH, iBSM;
	if(sFileNameBegin)
	{
		iDirNameLength = sFileNameBegin - sIdentifier;
		unsigned long iCRC = crc32_case_idt(crc32_case_idt(0L, Z_NULL, 0), (const Bytef*)sIdentifier, (uInt)iDirNameLength);
		++sFileNameBegin;
		iBSL = m_pSgaToCs[iToC].iStartDir;
		iBSH = m_pSgaToCs[iToC].iEndDir;
		iBSM = (iBSL + iBSH) >> 1;
		while(iBSH > iBSL)
		{
			if(iCRC < m_pSgaDirHashMap[iBSM].iCRC)
			{
				iBSH = iBSM;
				iBSM = (iBSL + iBSH) >> 1;
			}
			else if(iCRC > m_pSgaDirHashMap[iBSM].iCRC)
			{
				iBSL = iBSM + 1;
				iBSM = (iBSL + iBSH) >> 1;
			}
			else
			{
				iDirID = m_pSgaDirHashMap[iBSM].iID;
				break;
			}
		}
		if(iDirID == -1)
		{
			throw new CRainmanException(0, __FILE__, __LINE__, "Directory could not be found - %s", sIdentifier);
		}
	}
	else
	{
		/*
			If no directory is present in the path name then the directory to use is the
			first one in the ToC (Again, not totally sure of ToC internals)
		*/
		iDirID = m_pSgaToCs[iToC].iStartDir;
		sFileNameBegin = sIdentifier;
	}

	/*
		Find file in folder
		Again, do a binary chop on file names inside the directory (this time no hashes though; as there are
		many less files in one directory than directories an an SGA (usually) ). stricmp() works well for files
		as filenames are usually fairly varied, hashes are much better for directories, as directory names are
		very similar (eg. "attrib\weapons\axis\mgs\heavy" "attrib\weapons\axis\mgs\light")
	*/
	iBSL = m_pSgaDirs[iDirID].iFileBegin;
	iBSH = m_pSgaDirs[iDirID].iFileEnd;
	iBSM = (iBSL + iBSH) >> 1;
	while(iBSH > iBSL)
	{
		int iNiRes = stricmp(m_pSgaFileExts[iBSM].sName, sFileNameBegin);
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
			iFileID = iBSM;
			goto gotfile;
		}
	}
	
	throw new CRainmanException(0, __FILE__, __LINE__, "File could not be found - %s", sIdentifier);
gotfile:

	/*
		Do the actual data reading and decompressing
	*/
	unsigned long iDataLength, iDataLengthCompressed, iDataOffset;
	if(m_SgaHeader.iVersion == 2)
	{
		iDataLength = m_pSgaFiles[iFileID].iDataLength;
		iDataLengthCompressed = m_pSgaFiles[iFileID].iDataLengthCompressed;
		iDataOffset = m_pSgaFiles[iFileID].iDataOffset;
	}
	else
	{
		iDataLength = m_pSga4Files[iFileID].iDataLength;
		iDataLengthCompressed = m_pSga4Files[iFileID].iDataLengthCompressed;
		iDataOffset = m_pSga4Files[iFileID].iDataOffset;
	}

	char *pData = CHECK_MEM(new char[iDataLengthCompressed]);

	signed long iPreDataSize;
	iPreDataSize = (m_SgaHeader.iVersion == 2) ? 264 : 260;
	try
	{
		m_pFileStoreInputStream->VSeek(m_SgaHeader.iDataOffset + iDataOffset - iPreDataSize, IFileStore::IStream::SL_Root);
	}
	catch(CRainmanException *pE)
	{
		delete[] pData;
		throw new CRainmanException(pE, __FILE__, __LINE__, "Cannot seek to %lu for \'%s\'", m_SgaHeader.iDataOffset + iDataOffset, sIdentifier);
	}

	char sName[256];
	unsigned long iPreDataDate = 0, iPreDataCrc;

	try
	{
		m_pFileStoreInputStream->VRead(256, 1, sName);
		if(iPreDataSize == 264) m_pFileStoreInputStream->VRead(4, 1, &iPreDataDate);
		m_pFileStoreInputStream->VRead(4, 1, &iPreDataCrc);
		m_pFileStoreInputStream->VRead(iDataLengthCompressed, 1, pData);
	}
	catch(CRainmanException *pE)
	{
		delete[] pData;
		throw new CRainmanException(pE, __FILE__, __LINE__, "Cannot read %lu bytes for \'%s\'", iDataLengthCompressed, sIdentifier);
	}

	unsigned long iRawCRC = crc32(crc32(0L, Z_NULL, 0), (const Bytef*)pData, iDataLengthCompressed);

	if(iDataLengthCompressed != iDataLength)
	{
		char* pDecompressedData = new char[iDataLength];
		if(pDecompressedData == 0)
		{
			delete[] pData;
			throw new CRainmanException(0, __FILE__, __LINE__, "Cannot allocate %lu bytes for decompressed \'%s\'", iDataLength, sIdentifier);
		}

		unsigned long iTmp = iDataLength;
		if(uncompress((Bytef*)pDecompressedData, (uLongf*)&iTmp, (Bytef*)pData, iDataLengthCompressed) != Z_OK)
		{
			delete[] pData;
			delete[] pDecompressedData;
			throw new CRainmanException(0, __FILE__, __LINE__, "Cannot decompress \'%s\'", sIdentifier);
		}

		delete[] pData;
		pData = pDecompressedData;
	}

	unsigned long iUncompressedCRC = crc32(crc32(0L, Z_NULL, 0), (const Bytef*)pData, iDataLength);

	CMemoryStore CMS;
	CMS.VInit();

	CStream *pOutStream = new CStream();
	if(pOutStream == 0)
	{
		delete[] pData;
		QUICK_THROW("Cannot allocate memory")
	}
	pOutStream->m_pData = pData;
	pOutStream->m_pRawStream = 0;

	try
	{
		pOutStream->m_pRawStream = CMS.VOpenStream(CMS.MemoryRange(pData, iDataLength));
	}
	catch(CRainmanException *pE)
	{
		delete pOutStream;
		throw new CRainmanException(__FILE__, __LINE__, "Cannot open memory stream", pE);
	}

	return pOutStream;
}

CSgaFile::CStream::CStream()
{
}

CSgaFile::CStream::~CStream()
{
	delete m_pRawStream;
	delete[] m_pData;
}

void CSgaFile::CStream::VRead(unsigned long iItemCount, unsigned long iItemSize, void* pDestination)
{
	try
	{
		m_pRawStream->VRead(iItemCount, iItemSize, pDestination);
	}
	CATCH_THROW("Error reading from raw stream")
}

void CSgaFile::CStream::VSeek(long iPosition, IFileStore::IStream::SeekLocation SeekFrom)
{
	try
	{
		m_pRawStream->VSeek(iPosition, SeekFrom);
	}
	CATCH_THROW("Error seeking on raw stream")
}

long CSgaFile::CStream::VTell()
{
	try
	{
		return m_pRawStream->VTell();
	}
	CATCH_THROW("Error telling on raw stream")
}

IDirectoryTraverser::IIterator* CSgaFile::VIterate(const char* sPath)
{
	if(!m_pDataHeaderInfo) throw new CRainmanException(__FILE__, __LINE__, "No data header present");

	const char* sPathRemember = sPath;
	const char* sTmp;
	unsigned long iPartLength;
	long iToC = -1;

	/*
		Identify ToC
		Doesn't use a binary chop as there is usually only one ToC
	*/
	sTmp = strchr(sPath, '\\');
	iPartLength = (sTmp ? (unsigned long)(sTmp - sPath) : (unsigned long)strlen(sPath) );
	if(iPartLength == 0) return 0;
	for(unsigned short i = 0; i < m_pDataHeaderInfo->iToCCount; ++i)
	{
		/*
			Checks both length and string contents because if it only checked N characters of string
			then Data and Data2 could match each other. If strlen is included then they do not match.
		*/
		if(strlen(m_pSgaToCs[i].sAlias) == iPartLength && strnicmp(m_pSgaToCs[i].sAlias, sPath, iPartLength) == 0)
		{
			iToC = i;
			break;
		}
	}
	if(iToC == -1) throw new CRainmanException(0, __FILE__, __LINE__, "Cannot iterate \'%s\' - ToC not found", sPath);
 
	sPath += iPartLength + (sTmp ? 1 : 0);
	sTmp = strchr(sPath, '\\');
	iPartLength = (sTmp ? (unsigned long)(sTmp - sPath) : (unsigned long)strlen(sPath) );

	/*
		Identifies the directory to iterate part by part.
		Doesn't use a hash table, so leads to more descriptive errors.
		Might be better to switch to hash table :/
	*/
	long iDir = m_pSgaToCs[iToC].iStartDir;
	while(iPartLength != 0)
	{
		// Identify sub folder
		long iSubDir = -1;
		for(unsigned short i = m_pSgaDirs[iDir].iSubDirBegin; i < m_pSgaDirs[iDir].iSubDirEnd; ++i)
		{
			if(strlen(m_pSgaDirExts[i].sShortName) == iPartLength && strnicmp(m_pSgaDirExts[i].sShortName, sPath, iPartLength) == 0)
			{
				iSubDir = i;
				break;
			}
		}
		if(iSubDir == -1) throw new CRainmanException(0, __FILE__, __LINE__, "Cannot iterate \'%s\' - cannot find \'%s\'", sPathRemember, sPath);
		iDir = iSubDir;

		sPath += iPartLength + (sTmp ? 1 : 0);
		sTmp = strchr(sPath, '\\');
		iPartLength = (sTmp ? (unsigned long)(sTmp - sPath) : (unsigned long)strlen(sPath) );
	}
	try
	{
		return CHECK_MEM(new CIterator(iDir, (CSgaFile*)this) );
	}
	catch(CRainmanException *pE)
	{
		throw new CRainmanException(pE, __FILE__, __LINE__, "Cannot iterate \'%s\' (%li) due to constructor error", sPathRemember, iDir);
	}
}

unsigned long CSgaFile::VGetEntryPointCount()
{
	if(!m_pDataHeaderInfo) throw new CRainmanException(__FILE__, __LINE__, "No data header present");
	return (unsigned long)m_pDataHeaderInfo->iToCCount;
}

const char* CSgaFile::VGetEntryPoint(unsigned long iID)
{
	if(!m_pDataHeaderInfo) throw new CRainmanException(__FILE__, __LINE__, "No data header present");
	if(iID < 0 || iID >= (unsigned long)m_pDataHeaderInfo->iToCCount) throw new CRainmanException(0, __FILE__, __LINE__, "ID %lu is outside of range 0 -> %lu", iID, m_pDataHeaderInfo->iToCCount);
	return m_pSgaToCs[iID].sAlias;
}

CSgaFile::CIterator::CIterator(long iDir, CSgaFile* pSga)
{
	m_iParentDirectory = iDir;
	m_pSga = pSga;
	m_sParentPath = 0;
	m_sFullPath = 0;

	if(m_iParentDirectory < 0)
	{
		m_iParentDirectory = pSga->m_pSgaToCs[-1-m_iParentDirectory].iStartDir;
	}

	m_iTraversingWhat = 0;
	m_iCurrentItem = pSga->m_pSgaDirs[m_iParentDirectory].iSubDirBegin;

	if(m_iCurrentItem == pSga->m_pSgaDirs[m_iParentDirectory].iSubDirEnd)
	{
		m_iTraversingWhat = 1;
		m_iCurrentItem = pSga->m_pSgaDirs[m_iParentDirectory].iFileBegin;

		if(m_iCurrentItem == pSga->m_pSgaDirs[m_iParentDirectory].iFileEnd)
		{
			m_iTraversingWhat = -1;
		}
	}

	long iToC = -1;
	for(short i = 0; i < m_pSga->m_pDataHeaderInfo->iToCCount; ++i)
	{
		if(m_pSga->m_pSgaToCs[i].iStartDir <= iDir && m_pSga->m_pSgaToCs[i].iEndDir > iDir)
		{
			iToC = i;
			break;
		}
	}
	if(iToC == -1) throw new CRainmanException(0, __FILE__, __LINE__, "Directory %li does not fit into any ToC", iDir);

	m_sParentPath = CHECK_MEM(new char[strlen(m_pSga->m_pSgaToCs[iToC].sAlias) + 4 + strlen(m_pSga->m_pSgaDirExts[iDir].sName) + 3]);

	/*
		String together the full name of the directory being iterated;
		Put on the ToC name followed by a forward slash.
		If parent is less than zero, then the directory is a ToC directory, and needs nothing after the ToC name
		Otherwise, the directory has a name, which is appended on, and a forward slash added
	*/
	*m_sParentPath = 0;
	strcpy(m_sParentPath, m_pSga->m_SgaHeader.iVersion == 4 ? "Data" : m_pSga->m_pSgaToCs[iToC].sAlias);
	strcat(m_sParentPath, "\\");
	if(m_pSga->m_pSgaDirExts[iDir].iParent >= 0)
	{
		strcat(m_sParentPath, m_pSga->m_pSgaDirExts[iDir].sName);
		strcat(m_sParentPath, "\\");
	}

	if(m_iTraversingWhat == 0)
	{
		m_sFullPath = new char[strlen(m_sParentPath) + strlen(m_pSga->m_pSgaDirExts[m_iCurrentItem].sShortName) + 1];
		if(m_sFullPath == 0)
		{
			delete[] m_sParentPath;
			m_sParentPath = 0;
			throw new CRainmanException(__FILE__, __LINE__, "Memory allocate error");
		}
		strcpy(m_sFullPath, m_sParentPath);
		strcat(m_sFullPath, m_pSga->m_pSgaDirExts[m_iCurrentItem].sShortName);
	}
	else if(m_iTraversingWhat == 1)
	{
		m_sFullPath = new char[strlen(m_sParentPath) + strlen(m_pSga->m_pSgaFileExts[m_iCurrentItem].sName) + 1];
		if(m_sFullPath == 0)
		{
			delete[] m_sParentPath;
			m_sParentPath = 0;
			throw new CRainmanException(__FILE__, __LINE__, "Memory allocate error");
		}
		strcpy(m_sFullPath, m_sParentPath);
		strcat(m_sFullPath, m_pSga->m_pSgaFileExts[m_iCurrentItem].sName);
	}
}

CSgaFile::CIterator::~CIterator()
{
	delete[] m_sParentPath;
	delete[] m_sFullPath;
}

tLastWriteTime CSgaFile::VGetLastWriteTime(const char*)
{
	//! \todo Update this method to work with version 4 SGA files
	if(IsValidWriteTime(m_oSgaWriteTime)) return m_oSgaWriteTime;
	throw new CRainmanException(__FILE__, __LINE__, "No modification date present");
}

tLastWriteTime CSgaFile::CIterator::VGetLastWriteTime()
{
	//! \todo Update this method to work with version 4 SGA files
	if(IsValidWriteTime(m_pSga->m_oSgaWriteTime)) return m_pSga->m_oSgaWriteTime;
	throw new CRainmanException(__FILE__, __LINE__, "No modification date present");
}

IDirectoryTraverser::IIterator::eTypes CSgaFile::CIterator::VGetType()
{
	if(m_iTraversingWhat == 0) return IDirectoryTraverser::IIterator::T_Directory;
	if(m_iTraversingWhat == 1) return IDirectoryTraverser::IIterator::T_File;
	return IDirectoryTraverser::IIterator::T_Nothing;
}

IDirectoryTraverser::IIterator* CSgaFile::CIterator::VOpenSubDir()
{
	if(m_iTraversingWhat == 0)
	{
		try
		{
			return new CIterator(m_iCurrentItem, m_pSga);
		}
		CATCH_THROW("Cannot iterate over sub directory")
	}
	QUICK_THROW("Current item is not a directory")
}

IFileStore::IStream* CSgaFile::CIterator::VOpenFile()
{
	if(m_iTraversingWhat == 1)
	{
		try
		{
			return m_pSga->VOpenStream(m_sFullPath);
		}
		catch(CRainmanException *pE)
		{
			throw new CRainmanException(pE, __FILE__, __LINE__, "Unable to open \'%s\'", m_sFullPath);
		}
	}
	QUICK_THROW("Current item is not a file")
}

const char* CSgaFile::CIterator::VGetName()
{
	if(m_iTraversingWhat == 0)
	{
		return CHECK_STR(m_pSga->m_pSgaDirExts[m_iCurrentItem].sShortName);
	}
	else if(m_iTraversingWhat == 1)
	{
		return CHECK_STR(m_pSga->m_pSgaFileExts[m_iCurrentItem].sName);
	}
	QUICK_THROW("Nothing to get the name of")
}

const char* CSgaFile::CIterator::VGetFullPath()
{
	return CHECK_STR(m_sFullPath);
}

const char* CSgaFile::CIterator::VGetDirectoryPath()
{
	return CHECK_STR(m_sParentPath);
}

IDirectoryTraverser::IIterator::eErrors CSgaFile::CIterator::VNextItem()
{
	if(m_iTraversingWhat == 0)
	{
		++m_iCurrentItem;

		if(m_iCurrentItem == m_pSga->m_pSgaDirs[m_iParentDirectory].iSubDirEnd)
		{
			m_iTraversingWhat = 1;
			m_iCurrentItem = m_pSga->m_pSgaDirs[m_iParentDirectory].iFileBegin;

			if(m_iCurrentItem == m_pSga->m_pSgaDirs[m_iParentDirectory].iFileEnd)
			{
				m_iTraversingWhat = -1;
				return IDirectoryTraverser::IIterator::E_AtEnd;
			}
		}

		goto work_out_name;
	}
	else
	{
		++m_iCurrentItem;

		if(m_iCurrentItem == m_pSga->m_pSgaDirs[m_iParentDirectory].iFileEnd)
		{
			m_iTraversingWhat = -1;
			return IDirectoryTraverser::IIterator::E_AtEnd;
		}

		goto work_out_name;
	}

	QUICK_THROW("Already at end / error")

work_out_name:
	if(m_iTraversingWhat == 0)
	{
		delete[] m_sFullPath;
		m_sFullPath = 0;
		m_sFullPath = CHECK_MEM(new char[strlen(m_sParentPath) + strlen(m_pSga->m_pSgaDirExts[m_iCurrentItem].sShortName) + 1]);
		strcpy(m_sFullPath, m_sParentPath);
		strcat(m_sFullPath, m_pSga->m_pSgaDirExts[m_iCurrentItem].sShortName);
		return IDirectoryTraverser::IIterator::E_OK;
	}
	else
	{
		delete[] m_sFullPath;
		m_sFullPath = 0;
		m_sFullPath = CHECK_MEM(new char[strlen(m_sParentPath) + strlen(m_pSga->m_pSgaFileExts[m_iCurrentItem].sName) + 1]);
		strcpy(m_sFullPath, m_sParentPath);
		strcat(m_sFullPath, m_pSga->m_pSgaFileExts[m_iCurrentItem].sName);
		return IDirectoryTraverser::IIterator::E_OK;
	}
}

