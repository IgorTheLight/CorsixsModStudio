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

#include "CSgaCreator.h"
#include "Internal_Util.h"
#include "CMemoryStore.h"
#include "..\zLib/zlib.h"
#include "time.h"
extern "C" {
#include "md5.h"
}
#include <queue>
#include <algorithm>
#include "Exception.h"
#include "memdebug.h"

bool CSgaCreator::CInputFile::OpLT(CSgaCreator::CInputFile* oA, CSgaCreator::CInputFile* oB)
{
	return (strcmp(oA->sFullPath + oA->iNameOffset, oB->sFullPath + oB->iNameOffset) < 0);
}

CSgaCreator::CInputFile::~CInputFile()
{
	free(sFullPath);
}

bool CSgaCreator::CInputDirectory::OpLT(CSgaCreator::CInputDirectory* oA, CSgaCreator::CInputDirectory* oB)
{
	return (strcmp(oA->sNameFull, oB->sNameFull) < 0);
}

void CSgaCreator::CInputDirectory::FullCount(size_t& iDirCount, size_t& iFileCount)
{
	++iDirCount;
	for(std::vector<CSgaCreator::CInputFile*>::iterator itr = vFilesList.begin(); itr != vFilesList.end(); ++itr)
	{
		if(*itr) ++iFileCount;
	}
	for(std::vector<CSgaCreator::CInputDirectory*>::iterator itr = vDirsList.begin(); itr != vDirsList.end(); ++itr)
	{
		if(*itr) (*itr)->FullCount(iDirCount, iFileCount);
	}
}

CSgaCreator::CInputDirectory::~CInputDirectory()
{
	free(sNameFull);
	for(std::vector<CSgaCreator::CInputFile*>::iterator itr = vFilesList.begin(); itr != vFilesList.end(); ++itr)
	{
		delete *itr;
	}
	for(std::vector<CSgaCreator::CInputDirectory*>::iterator itr = vDirsList.begin(); itr != vDirsList.end(); ++itr)
	{
		delete *itr;
	}
}

CSgaCreator::CInputDirectory* CSgaCreator::_ScanDirectory(IDirectoryTraverser::IIterator* pDirectory, IFileStore* pStore, size_t iDirBaseL)
{
	if(pDirectory == 0) QUICK_THROW("No input directory")
	if(pStore == 0) QUICK_THROW("No input file store")

	CSgaCreator::CInputDirectory *pUs = CHECK_MEM(new CSgaCreator::CInputDirectory);
	const char* sDirectory;

	try
	{
		sDirectory = pDirectory->VGetDirectoryPath();
	}
	catch(CRainmanException *pE)
	{
		delete pUs;
		throw new CRainmanException(__FILE__, __LINE__, "Cannot get directory path", pE);
	}
	
	size_t iDirLen = strlen(sDirectory);
	if(iDirBaseL != 0)
	{
		if(sDirectory[iDirLen - 1] != '\\') pUs->sNameFull = strdup(sDirectory + iDirBaseL + 1);
		else
		{
			pUs->sNameFull = strdup(sDirectory + iDirBaseL);
			pUs->sNameFull[strlen(pUs->sNameFull) - 1] = 0;
		}
	}
	else
	{
		iDirBaseL = iDirLen;
		pUs->sNameFull = strdup(sDirectory + iDirBaseL);
	}
	Util_strtolower(pUs->sNameFull);
	try
	{
		while(pDirectory->VGetType() != IDirectoryTraverser::IIterator::T_Nothing)
		{
			if(pDirectory->VGetType() == IDirectoryTraverser::IIterator::T_Directory)
			{
				IDirectoryTraverser::IIterator *pChild;
				try
				{
					pChild = pDirectory->VOpenSubDir();
				}
				catch(CRainmanException* pE)
				{
					throw new CRainmanException(pE, __FILE__, __LINE__, "Cannot traverse sub-dir \'%s\'", pDirectory->VGetFullPath());
				}
				if(pChild != 0)
				{
					CSgaCreator::CInputDirectory *pChildIn = 0;
					try
					{
						pChildIn = _ScanDirectory(pChild, pStore, iDirBaseL);
					}
					catch(CRainmanException* pE)
					{
						delete pChild;
						throw new CRainmanException(pE, __FILE__, __LINE__, "Cannot scan sub-dir \'%s\'", pDirectory->VGetFullPath());
					}
					delete pChild;
					pUs->vDirsList.push_back(pChildIn);
				}
			}
			else
			{
				CSgaCreator::CInputFile* pFile = CHECK_MEM(new CSgaCreator::CInputFile);
				try
				{
					pFile->oLastWriteTime = pDirectory->VGetLastWriteTime();
					pFile->sFullPath = strdup(pDirectory->VGetFullPath());
					Util_strtolower(pFile->sFullPath);
				}
				CATCH_THROW("pDirectory problem")
				pFile->iNameOffset = iDirLen;
				if(pFile->sFullPath[iDirLen] == '\\') pFile->iNameOffset += 1;
				pFile->pFileStore = pStore;
				pUs->vFilesList.push_back(pFile);
			}
			switch(pDirectory->VNextItem())
			{
			case IDirectoryTraverser::IIterator::E_AtEnd:
				goto endwhile;
			default:
				break;
			}
		}
	}
	catch(CRainmanException *pE)
	{
		delete pUs;
		throw new CRainmanException(__FILE__, __LINE__, "(rethrow)", pE);
	}
endwhile:

	std::sort(pUs->vDirsList.begin(), pUs->vDirsList.end(), CInputDirectory::OpLT);
	std::sort(pUs->vFilesList.begin(), pUs->vFilesList.end(), CInputFile::OpLT);

	return pUs;
}

void CSgaCreator::CreateSga(IDirectoryTraverser::IIterator* pDirectory, IFileStore* pStore, const char* _sTocName, const char* sOutputFile, long iVersion, const char* sArchiveTitle, const char* sTocTitle)
{
	if(iVersion != 2 && iVersion != 4) throw new CRainmanException(0, __FILE__, __LINE__, "Version %li not supported", iVersion);
	CSgaCreator::CInputDirectory *pInput = 0;
	try
	{
		pInput = _ScanDirectory(pDirectory, pStore);
	}
	catch(CRainmanException* pE)
	{
		throw new CRainmanException(pE, __FILE__, __LINE__, "ScanDirectory failed");
	}

	FILE *fOut = 0;
	CMemoryStore::COutStream* pDataHeader = 0;
	IFileStore::IStream *pFileStream = 0;
	char *pFileBuffer = 0, *pCompressedBuffer = 0;
	try
	{
		unsigned long iTmp = 0;
		unsigned short iSTmp = 0;
		fOut = fopen(sOutputFile, "w+b");
		if(fOut == 0) throw new CRainmanException(0, __FILE__, __LINE__, "Cold not open \'%s\' (does the directory exist, and do you have permission to create files there?)", sOutputFile);

		// File signature
		if(fwrite("_ARCHIVE", 1, 8, fOut) != 8) throw new CRainmanException(__FILE__, __LINE__, "Write operation failed");
		// Version
		if(fwrite(&iVersion, sizeof(long), 1, fOut) != 1) throw new CRainmanException(__FILE__, __LINE__, "Write operation failed");
		// MD5
		if(fwrite("-BUFFER FOR MD5-", 1, 16, fOut) != 16) throw new CRainmanException(__FILE__, __LINE__, "Write operation failed");
		// Archive UI name
		wchar_t sUiName[65];
		memset(sUiName, 0, 65 * sizeof(wchar_t));
		if(sArchiveTitle)
		{
			size_t i = 0;
			do
			{
				sUiName[i] = sArchiveTitle[i];
				++i;
			} while(sArchiveTitle[i] && i < 65);
		}
		else
		{
			wcscpy(sUiName, L"Made with Corsix\'s Rainman");
		}
		if(fwrite(sUiName, sizeof(wchar_t), 64, fOut) != 64) throw new CRainmanException(__FILE__, __LINE__, "Write operation failed");
		// MD5
		if(fwrite("-BUFFER FOR MD5-", 1, 16, fOut) != 16) throw new CRainmanException(__FILE__, __LINE__, "Write operation failed");
		// DataHeaderSize / DataOffset
		if(fwrite(&iTmp, sizeof(long), 1, fOut) != 1 || fwrite(&iTmp, sizeof(long), 1, fOut) != 1) throw new CRainmanException(__FILE__, __LINE__, "Write operation failed");
		// Platform [V4 only]
		if(iVersion == 4)
		{
			iTmp = 1;
			if(fwrite(&iTmp, sizeof(long), 1, fOut) != 1) throw new CRainmanException(__FILE__, __LINE__, "Write operation failed");
		}

		const unsigned long iTocOutLength = 138;
		const unsigned long iDirOutLength = 12;
		unsigned long iFileOutLength;
		long iNamesStartLocation, iNamesLength;
		long iFilesLoc;
		std::vector<CInputFile*> vFilesList;
		size_t iDirCount = 0, iFileCount = 0;

		// Data header buffer
		CMemoryStore oMemoryStore;
		try
		{
			pDataHeader = (CMemoryStore::COutStream*)oMemoryStore.VOpenOutputStream(0, false);

			// TOC offset & count
			iTmp = 24;
			pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned long), &iTmp);
			iSTmp = 1;
			pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned short), &iSTmp);

			// Directory offset & count
			pInput->FullCount(iDirCount, iFileCount);
			iTmp = 24 + iTocOutLength;
			pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned long), &iTmp);
			iSTmp = (unsigned short)iDirCount;
			pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned short), &iSTmp);

			// File offset & count
			iTmp = 24 + iTocOutLength + (iDirOutLength * iDirCount);
			pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned long), &iTmp);
			iSTmp = (unsigned short)iFileCount;
			pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned short), &iSTmp);
			iFileOutLength = (iVersion == 4 ? 22 : 20);

			// Names offset & count
			iTmp = 24 + iTocOutLength + (iDirOutLength * iDirCount) + (iFileOutLength * iFileCount);
			pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned long), &iTmp);
			iSTmp = (unsigned short)(iFileCount + iDirCount);
			pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned short), &iSTmp);

			// TOC Entry
			char sTocName[65];
			memset(sTocName, 0, 65);
			strncpy(sTocName, _sTocName, strlen(_sTocName) > 64 ? 64 : strlen(_sTocName));
			Util_strtolower(sTocName);
			pDataHeader->VWrite(64, (unsigned long)sizeof(char), sTocName);

			if(sTocTitle)
			{
				memset(sTocName, 0, 65);
				strncpy(sTocName, sTocTitle, strlen(sTocTitle) > 64 ? 64 : strlen(sTocTitle));
				Util_strtolower(sTocName);
			}
			else
			{
				struct tm *newtime;
				time_t ltime;
				time( &ltime );
				newtime = gmtime( &ltime );
				sprintf(sTocName, "archive_%i_%i_%i_%i_%i_%i", newtime->tm_year + 1900, newtime->tm_mon, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec);
			}

			pDataHeader->VWrite(64, (unsigned long)sizeof(char), sTocName);
			iSTmp = 0;
			pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned short), &iSTmp);
			iSTmp = (short)iDirCount;
			pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned short), &iSTmp);
			iSTmp = 0;
			pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned short), &iSTmp);
			iSTmp = (short)iFileCount;
			pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned short), &iSTmp);
			iSTmp = 0;
			pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned short), &iSTmp);

			// Dir header space
			for(size_t i = 0; i < iDirCount; ++i) pDataHeader->VWrite(12, (unsigned long)sizeof(char), "DIR H BUFFER");

			// File header space
			for(size_t i = 0; i < iFileCount; ++i) pDataHeader->VWrite(iFileOutLength, (unsigned long)sizeof(char), "[-FILE HEADER BUFFER-]");

			// Dir headers properly
			iNamesStartLocation = pDataHeader->VTell();
			iNamesLength = 0;
			pDataHeader->VSeek(24 + iTocOutLength, IFileStore::IStream::SL_Root);

			std::queue<CInputDirectory*> qDirsTodo;
			qDirsTodo.push(pInput);
			vFilesList.reserve(iFileCount);
			size_t iDirC = 1, iFileC = 0;
			while(qDirsTodo.size())
			{
				CInputDirectory *pDir = qDirsTodo.front();
				qDirsTodo.pop();

				// Name offset
				pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned long), &iNamesLength);

				// Name
				long iRestorePos = pDataHeader->VTell();
				pDataHeader->VSeek(iNamesStartLocation + iNamesLength, IFileStore::IStream::SL_Root);
				size_t iNameL = strlen(pDir->sNameFull) + 1;
				pDataHeader->VWrite((unsigned long)iNameL, (unsigned long)sizeof(char), pDir->sNameFull);
				iNamesLength += (long)iNameL;
				pDataHeader->VSeek(iRestorePos, IFileStore::IStream::SL_Root);

				// Start/End directory
				iSTmp = (short)iDirC;
				pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned short), &iSTmp);
				iDirC += pDir->vDirsList.size();
				iSTmp = (short)iDirC;
				pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned short), &iSTmp);

				// Start/End file
				iSTmp = (short)iFileC;
				pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned short), &iSTmp);
				iFileC += pDir->vFilesList.size();
				iSTmp = (short)iFileC;
				pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned short), &iSTmp);

				// Identify child folders/files
				for(std::vector<CInputDirectory*>::iterator itr = pDir->vDirsList.begin(); itr != pDir->vDirsList.end(); ++itr)
				{
					qDirsTodo.push(*itr);
				}
				for(std::vector<CInputFile*>::iterator itr = pDir->vFilesList.begin(); itr != pDir->vFilesList.end(); ++itr)
				{
					vFilesList.push_back(*itr);
				}
			}

			// File headers properly phase 1
			iFilesLoc = pDataHeader->VTell();
			for(std::vector<CInputFile*>::iterator itr = vFilesList.begin(); itr != vFilesList.end(); ++itr, iFilesLoc += iFileOutLength)
			{
				pDataHeader->VSeek(iFilesLoc, IFileStore::IStream::SL_Root);

				// Name offset
				pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned long), &iNamesLength);
				
				// Name
				pDataHeader->VSeek(iNamesStartLocation + iNamesLength, IFileStore::IStream::SL_Root);
				size_t iNameL = strlen( ((**itr).sFullPath + (**itr).iNameOffset) ) + 1;
				pDataHeader->VWrite((unsigned long)iNameL, (unsigned long)sizeof(char), ((**itr).sFullPath + (**itr).iNameOffset));
				iNamesLength += (long)iNameL;
			}
		}
		catch(CRainmanException *pE)
		{
			throw new CRainmanException(__FILE__, __LINE__, "Memory operation failed", pE);
		}

		// Get output file ready for writing file data
		if(fwrite(pDataHeader->GetData(), 1, pDataHeader->GetDataLength(), fOut) != pDataHeader->GetDataLength()) throw new CRainmanException(__FILE__, __LINE__, "Write operation failed");

		// File headers properly phase 2
		iFilesLoc = 24 + iTocOutLength + (iDirOutLength * iDirCount) + 4;
		long iDataLengthTotal = 0;
		long iPreDataSize = 0;
		if(iVersion == 4) iPreDataSize = 260;
		for(std::vector<CInputFile*>::iterator itr = vFilesList.begin(); itr != vFilesList.end(); ++itr, iFilesLoc += iFileOutLength)
		{
			try
			{
				pFileStream = (**itr).pFileStore->VOpenStream((**itr).sFullPath);
			}
			catch(CRainmanException* pE)
			{
				throw new CRainmanException(pE, __FILE__, __LINE__, "Could not open \'%s\'", (**itr).sFullPath);
			}
			long iInputFileLength;
			try
			{
				pFileStream->VSeek(0, IFileStore::IStream::SL_End);
				iInputFileLength = pFileStream->VTell();
				pFileStream->VSeek(0, IFileStore::IStream::SL_Root);
				pFileBuffer = new char[iInputFileLength];
				pFileStream->VRead(iInputFileLength, 1, pFileBuffer);
				delete pFileStream;
				pFileStream = 0;
			}
			catch(CRainmanException* pE)
			{
				throw new CRainmanException(__FILE__, __LINE__, "File operation failed", pE);
			}
			unsigned long iUncompressedCRC = crc32(crc32(0L, Z_NULL, 0), (const Bytef*)pFileBuffer, iInputFileLength);
			unsigned long iCompBuffSize = compressBound(iInputFileLength);
			pCompressedBuffer = new char[iCompBuffSize];
			if(compress2((Bytef*)pCompressedBuffer, &iCompBuffSize, (const Bytef*)pFileBuffer, iInputFileLength, Z_BEST_COMPRESSION) != Z_OK) throw new CRainmanException(__FILE__, __LINE__, "Compression error");
			long iFlags;
			if(iCompBuffSize < (unsigned long)iInputFileLength)
			{
				if(iVersion == 4)
				{
					/* if(iInputFileLength < 2048) iFlags = 0;
					else */ iFlags = 0x100; //(iInputFileLength > 8192 ? 0x100 : 0x200);
				}
				else
				{
					iFlags = (iInputFileLength < 4096 ? 0x20 : 0x10);
				}
			}
			else
			{
				iFlags = 0;
			}

			if(iFlags == 0)
			{
				delete[] pCompressedBuffer;
				pCompressedBuffer = pFileBuffer;
				iCompBuffSize = iInputFileLength;
			}
			else
			{
				delete[] pFileBuffer;
			}
			pFileBuffer = 0;

			iDataLengthTotal += iPreDataSize;
			try
			{
				pDataHeader->VSeek(iFilesLoc, IFileStore::IStream::SL_Root);
				if(iVersion == 4)
				{
					// Data offset
					pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned long), &iDataLengthTotal);

					// Size compressed / decompressed
					pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned long), &iCompBuffSize);
					pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned long), &iInputFileLength);
					
					// Time
					iTmp = (unsigned long)(**itr).oLastWriteTime;
					pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned long), &iTmp);

					// Flags
					iSTmp = (unsigned short)iFlags;
					pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned short), &iSTmp);
				}
				else
				{
					// Flags
					pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned short), &iFlags);

					// Data offset
					pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned long), &iDataLengthTotal);

					// Size compressed / decompressed
					pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned long), &iCompBuffSize);
					pDataHeader->VWrite(1, (unsigned long)sizeof(unsigned long), &iInputFileLength);
				}
			}
			catch(CRainmanException* pE)
			{
				throw new CRainmanException(__FILE__, __LINE__, "Memory operation failed", pE);
			}

			if(iVersion == 4)
			{
				char sPreDataName[256];
				memset(sPreDataName, 0, 256);
				strcpy(sPreDataName, (**itr).sFullPath + (**itr).iNameOffset);
				sPreDataName[255] = 0;

				if(fwrite(sPreDataName, 1, 256, fOut) != 256) throw new CRainmanException(__FILE__, __LINE__, "Write operation failed");
				if(fwrite(&iUncompressedCRC, sizeof(long), 1, fOut) != 1) throw new CRainmanException(__FILE__, __LINE__, "Write operation failed");
			}
			if(fwrite(pCompressedBuffer, 1, iCompBuffSize, fOut) != iCompBuffSize) throw new CRainmanException(__FILE__, __LINE__, "Write operation failed");
			iDataLengthTotal += iCompBuffSize;
			delete[] pCompressedBuffer;
			pCompressedBuffer = 0;
		}

		// Header MD5
		MD5Context md5DataKey;
		unsigned char sMD5[16];
		MD5InitKey(&md5DataKey, "DFC9AF62-FC1B-4180-BC27-11CCE87D3EFF");
		MD5Update(&md5DataKey, (const unsigned char*)pDataHeader->GetData(), pDataHeader->GetDataLength());
		MD5Final(sMD5, &md5DataKey);
		if(fseek(fOut, 156, SEEK_SET) != 0) throw new CRainmanException(__FILE__, __LINE__, "Seek operation failed");
		if(fwrite(sMD5, 1, 16, fOut) != 16) throw new CRainmanException(__FILE__, __LINE__, "Write operation failed");

		// Data header size / offset
		iTmp = pDataHeader->GetDataLength();
		if(fwrite(&iTmp, sizeof(long), 1, fOut) != 1) throw new CRainmanException(__FILE__, __LINE__, "Write operation failed");
		iTmp += (iVersion == 4 ? 0xB8 : 0xB4);
		if(fwrite(&iTmp, sizeof(long), 1, fOut) != 1) throw new CRainmanException(__FILE__, __LINE__, "Write operation failed");

		// Write data header properly
		if(iVersion == 4)
		{
			if(fseek(fOut, 4, SEEK_CUR) != 0) throw new CRainmanException(__FILE__, __LINE__, "Seek operation failed");
		}
		if(fwrite(pDataHeader->GetData(), 1, pDataHeader->GetDataLength(), fOut) != pDataHeader->GetDataLength()) throw new CRainmanException(__FILE__, __LINE__, "Write operation failed");

		// Data MD5
		fflush(fOut);
		MD5InitKey(&md5DataKey, "E01519D6-2DB7-4640-AF54-0A23319C56C3");
		fseek(fOut, (iVersion == 4) ? 184 : 180, SEEK_SET);
		char* sBuffer = new char[1048576];
		while(!feof(fOut))
		{
			MD5Update(&md5DataKey, (const unsigned char*)sBuffer, (unsigned int)fread(sBuffer, 1, 1048576, fOut));
		}
		delete[] sBuffer;
		MD5Final(sMD5, &md5DataKey);
		if(fseek(fOut, 12, SEEK_SET) != 0) throw new CRainmanException(__FILE__, __LINE__, "Seek operation failed");
		if(fwrite(sMD5, 1, 16, fOut) != 16) throw new CRainmanException(__FILE__, __LINE__, "Write operation failed");
	}
	catch(CRainmanException *pE)
	{
		delete pInput;
		if(fOut) fclose(fOut);
		if(pDataHeader) delete pDataHeader;
		if(pFileStream) delete pFileStream;
		if(pFileBuffer) delete[] pFileBuffer;
		if(pCompressedBuffer) delete[] pCompressedBuffer;
		throw pE;
	}
	delete pInput;
	if(fOut) fclose(fOut);
	if(pDataHeader) delete pDataHeader;
	if(pFileStream) delete pFileStream;
	if(pFileBuffer) delete[] pFileBuffer;
	if(pCompressedBuffer) delete[] pCompressedBuffer;
}

