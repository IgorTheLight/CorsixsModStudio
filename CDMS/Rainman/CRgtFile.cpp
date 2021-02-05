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

#include "CRgtFile.h"
#include "..\zLib/zlib.h"
#include "Exception.h"
#include "memdebug.h"
#include <time.h>

CRgtFile::CRgtFile()
{
	m_pChunky = 0;
	m_eFormat = IF_None;

	m_iWidth = 0;
	m_iHeight = 0;
	m_iDataLength = 0;

	m_iMipCount = 0;
	m_iMipCurrent = 0;
	m_iDxtCompression = 0;

	m_pData = 0;

	m_fnCompress = 0;
	m_fnDecompress = 0;
}

bool CRgtFile::isDXTCodecPresent()
{
	return (m_fnCompress && m_fnDecompress);
}

void CRgtFile::setDXTCodec(tfnDXTCodec fnCompress, tfnDXTCodec fnDecompress)
{
	m_fnCompress = fnCompress;
	m_fnDecompress = fnDecompress;
}

CRgtFile::~CRgtFile()
{
	_Clean();
}

CRgtFile::eImageFormats CRgtFile::GetImageFormat()
{
	return m_eFormat;
}

void CRgtFile::_MakeFileBurnInfo(CChunkyFile* pChunkyFile)
{
	CChunkyFile::CChunk* pChunk = pChunkyFile->AppendNew("FBIF", CChunkyFile::CChunk::T_Data);
	pChunk->SetVersion(2);
	pChunk->SetDescriptor("FileBurnInfo");

	CMemoryStore::COutStream *pOutStream = CMemoryStore::OpenOutputStreamExt();
	unsigned long iN;
	iN = 25; pOutStream->VWrite(1, sizeof(long), &iN);
	pOutStream->VWrite(25, 1, "generic-image to data-rgt");
	iN = 1; pOutStream->VWrite(1, sizeof(long), &iN);
	iN = 32; pOutStream->VWrite(1, sizeof(long), &iN);
	pOutStream->VWrite(32, 1, "Rainman Library (www.corsix.org)");

	struct tm *pTm;
	const char* sMonths[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
	char sDataBuffer[32];
    time_t pTime;
    time(&pTime);
    pTm = localtime(&pTime);

	sprintf(sDataBuffer, "%s %.2i, %.4i, %.2i:%.2i:%.2i", sMonths[pTm->tm_mon], pTm->tm_mday, (int)(pTm->tm_year + 1900), pTm->tm_hour, pTm->tm_min, pTm->tm_sec );

	iN = (unsigned long)strlen(sDataBuffer); pOutStream->VWrite(1, sizeof(long), &iN);
	pOutStream->VWrite(iN, 1, sDataBuffer);
	iN = 0; pOutStream->VWrite(1, sizeof(long), &iN);

	pChunk->SetData(pOutStream);
	delete pOutStream;
}

void CRgtFile::_MakeDataAttr(CChunkyFile::CChunk* pParentChunk, long iWidth, long iHeight, long iUnknown)
{
	CChunkyFile::CChunk* pChunk = pParentChunk->AppendNew("ATTR", CChunkyFile::CChunk::T_Data);

	pChunk->SetVersion(1);

	CMemoryStore::COutStream *pOutStream = CMemoryStore::OpenOutputStreamExt();
	pOutStream->VWrite(1, sizeof(long), &iUnknown);
	pOutStream->VWrite(1, sizeof(long), &iWidth);
	pOutStream->VWrite(1, sizeof(long), &iHeight);

	pChunk->SetData(pOutStream);
	delete pOutStream;
}

void CRgtFile::_MakeDataData(CChunkyFile::CChunk* pParentChunk, long iVal1, long iVal2, int iType)
{
	CChunkyFile::CChunk* pChunk = pParentChunk->AppendNew("DATA", CChunkyFile::CChunk::T_Data);

	pChunk->SetVersion(iType);
	pChunk->SetUnknown1(1);

	CMemoryStore::COutStream *pOutStream = CMemoryStore::OpenOutputStreamExt();
	pOutStream->VWrite(1, sizeof(long), &iVal1);
	if(iType == 3) pOutStream->VWrite(1, sizeof(long), &iVal2);

	pChunk->SetData(pOutStream);
	delete pOutStream;
}

void CRgtFile::_MakeDataInfo(CChunkyFile::CChunk* pParentChunk, long iWidth, long iHeight, char iFinalByte)
{
	CChunkyFile::CChunk* pChunk = pParentChunk->AppendNew("INFO", CChunkyFile::CChunk::T_Data);

	pChunk->SetVersion(2);

	CMemoryStore::COutStream *pOutStream = CMemoryStore::OpenOutputStreamExt();

	pOutStream->VWrite(1, sizeof(long), &iWidth);
	pOutStream->VWrite(1, sizeof(long), &iHeight);
	long N;
	N = 1; pOutStream->VWrite(1, sizeof(long), &N);
	N = 2; pOutStream->VWrite(1, sizeof(long), &N);
	N = 2; pOutStream->VWrite(1, sizeof(long), &N);
	N = 0; pOutStream->VWrite(1, sizeof(long), &N);
	pOutStream->VWrite(1, 1, &iFinalByte);

	pChunk->SetData(pOutStream);
	delete pOutStream;
}

void CRgtFile::_MakeDxtcDataTexFormat(CChunkyFile::CChunk* pParentChunk, unsigned long iWidth, unsigned long iHeight, int iDxtN)
{
	CChunkyFile::CChunk* pChunk = pParentChunk->AppendNew("TFMT", CChunkyFile::CChunk::T_Data);
	pChunk->SetVersion(1);
	pChunk->SetUnknown1(-1);
	CMemoryStore::COutStream *pOutStream = CMemoryStore::OpenOutputStreamExt();

	long N;
	pOutStream->VWrite(1, sizeof(long), &iWidth);
	pOutStream->VWrite(1, sizeof(long), &iHeight);
	N = 1; pOutStream->VWrite(1, sizeof(long), &N);
	N = 2; pOutStream->VWrite(1, sizeof(long), &N);

	switch(iDxtN)
	{
	case 1:
		N = 0xD;
		break;
	case 3:
		N = 0xE;
		break;
	case 5:
		N = 0xF;
		break;
	default:
		N = 0; // unknown
		break;
	};
	pOutStream->VWrite(1, sizeof(long), &N);
	N = 0; pOutStream->VWrite(1, sizeof(long), &N);
	char c = 1; pOutStream->VWrite(1, 1, &c);

	pChunk->SetData(pOutStream);
	delete pOutStream;
}

void CRgtFile::_MakeDxtcData(CMemoryStore::COutStream* pTMAN, CMemoryStore::COutStream* pTDAT, IFileStore::IStream* pDDS, unsigned long iWidth, unsigned long iHeight, int iDxtN, bool bMipMaps, int iThisMipLevel)
{
	// Read image from DDS file
	size_t iDataSizeUncompressed;
	iDataSizeUncompressed = std::max((size_t) 1, (size_t)(iWidth / 4)) * std::max((size_t) 1, (size_t)(iHeight / 4)) * (size_t)(iDxtN == 1 ? 8 : 16);
	unsigned char* pData = new unsigned char[iDataSizeUncompressed + 16];
	unsigned long* iData = (unsigned long*)pData;
	iData[0] = iThisMipLevel;
	iData[1] = iWidth;
	iData[2] = iHeight;
	iData[3] = (unsigned long)iDataSizeUncompressed;

	try
	{
		pDDS->VRead((unsigned long)iDataSizeUncompressed , 1, pData + 16);
	}
	catch(CRainmanException *pE)
	{
		delete[] pData;
		throw new CRainmanException(pE, __FILE__, __LINE__, "Error loading image %lux%lu", iWidth, iHeight);
	}

	iDataSizeUncompressed += 16;

	// Do mip map of this level + 1
	// DXTC files have largest image ----> smallest image
	// RGT files have smallest image ----> largest image
	// thus we read the data from the DDS, then do mip level, then do output, as it reverses the order
	if(bMipMaps && (iWidth > 1 || iHeight > 1) )
	{
		try
		{
			_MakeDxtcData(pTMAN, pTDAT, pDDS, std::max(iWidth / 2, (unsigned long) 1), std::max(iHeight / 2, (unsigned long) 1), iDxtN, true, iThisMipLevel + 1);
		}
		catch(CRainmanException *pE)
		{
			delete[] pData;
			throw new CRainmanException(pE, __FILE__, __LINE__, "Error doing mip-map of %lux%lu", iWidth, iHeight);
		}
	}

	// Try compressing ourselves
	unsigned long iCompressedSize = compressBound((uLong)iDataSizeUncompressed);
	unsigned char* pCompressedData = new unsigned char[iCompressedSize];
	if(compress2((Bytef*)pCompressedData, &iCompressedSize, (const Bytef*)pData, (uLong)iDataSizeUncompressed, Z_BEST_COMPRESSION) != Z_OK)
	{
		delete[] pCompressedData;
		delete[] pData;
		throw new CRainmanException(__FILE__, __LINE__, "Compression error");
	}

	
	if(iCompressedSize < iDataSizeUncompressed)
	{
		delete[] pData;
		pData = pCompressedData;
	}
	else
	// no compression...
	{
		delete[] pCompressedData;
		iCompressedSize = (unsigned long)iDataSizeUncompressed;
	}

	// Write TMAN
	unsigned long N;
	if(pTMAN->GetDataLength() == 0)
	{
		N = iThisMipLevel + 1; // mip-level count
		pTMAN->VWrite(1, sizeof(long), &N);
	}

	N = (unsigned long)iDataSizeUncompressed;
	pTMAN->VWrite(1, sizeof(long), &N);
	pTMAN->VWrite(1, sizeof(long), &iCompressedSize);

	// Write TDAT
	//N = iThisMipLevel;
	//pTDAT->VWrite(1, sizeof(long), &N);
	//pTDAT->VWrite(1, sizeof(long), &iWidth);
	//pTDAT->VWrite(1, sizeof(long), &iHeight);
	//pTDAT->VWrite(1, sizeof(long), &iCompressedSize);
	pTDAT->VWrite(iCompressedSize, 1, pData);
	
	// Cleanup
	delete[] pData;
}

void CRgtFile::LoadTGA(IFileStore::IStream *pFile, bool bMakeMips, bool* pIs32Bit)
{
	_Clean();

	// Make what chunkyness can be made without opening the DDS
	m_pChunky = CHECK_MEM(new CChunkyFile);
	m_pChunky->New(3);

	_MakeFileBurnInfo(m_pChunky);

	CChunkyFile::CChunk* pFoldChunk = m_pChunky->AppendNew("TSET", CChunkyFile::CChunk::T_Folder), *pTXTR;
	pFoldChunk->SetVersion(1);
	pFoldChunk->SetDescriptor("\\ww2\\datageneric\\bia\\ww2\\art\\folder\\aaaaa\\bbbb\\cccccccc\\ddddddd\\eeeeeee\\fffff\\gggggggggggggg");

	if(bMakeMips) _MakeDataData(pFoldChunk);
	else _MakeDataData(pFoldChunk, 0, 0, 2);

	pTXTR = pFoldChunk = pFoldChunk->AppendNew("TXTR", CChunkyFile::CChunk::T_Folder);
	pFoldChunk->SetVersion(1);
	pFoldChunk->SetDescriptor("tool:C:\\WW2\\DataGeneric\\BIA\\WW2\\Art\\folder\\Aaaaa\\Bbbb\\Cccccccc\\Ddddddd\\Eeeeeee\\Fffff\\GGGgGGgGgggggg.hhh");

	// Load up some TGA header
	char iTga_IdLen, iTga_ColourMapT, iTga_DataT, iTga_ColourMapD, iTga_BPP, iTga_Flags;
	short int iTga_ColourMapO, iTga_ColourMapL, iTga_XO, iTga_YO, iTga_W, iTga_H;

	try
	{
		pFile->VRead(1, 1, &iTga_IdLen);
		pFile->VRead(1, 1, &iTga_ColourMapT);
		pFile->VRead(1, 1, &iTga_DataT);
		pFile->VRead(1, 2, &iTga_ColourMapO);
		pFile->VRead(1, 2, &iTga_ColourMapL);
		pFile->VRead(1, 1, &iTga_ColourMapD);
		pFile->VRead(1, 2, &iTga_XO);
		pFile->VRead(1, 2, &iTga_YO);
		pFile->VRead(1, 2, &iTga_W);
		pFile->VRead(1, 2, &iTga_H);
		pFile->VRead(1, 1, &iTga_BPP);
		pFile->VRead(1, 1, &iTga_Flags);
	}
	catch(CRainmanException* pE)
	{
		throw new CRainmanException(__FILE__, __LINE__, "Error reading TGA header", pE);
	}

	if(iTga_DataT != 2
		|| ( iTga_BPP != 24 && iTga_BPP != 32 )
		|| ( iTga_Flags & 0xC0 )
		/*|| ( (iTga_Flags & 0xF) != (iTga_BPP - 24) )*/ // Some silly programs don't set the attribute bit count. fools.
		)
	{
		throw new CRainmanException(__FILE__, __LINE__, "TGA file must be uncompressed, non-interleaved 24 bit RGB or 32 bit RGBA data");
	}
	if(pIs32Bit) *pIs32Bit = (iTga_BPP == 32);

	try
	{
		pFile->VSeek(iTga_IdLen);
		if(iTga_ColourMapT) pFile->VSeek(iTga_ColourMapL * (iTga_ColourMapD >> 3));
	}
	catch(CRainmanException* pE)
	{
		throw new CRainmanException(__FILE__, __LINE__, "Error seeking over TGA predata", pE);
	}

	// Make some more chunkiness
	_MakeDataInfo(pFoldChunk, iTga_W, iTga_H, bMakeMips ? 1 : 0);

	pFoldChunk = pFoldChunk->AppendNew("IMAG", CChunkyFile::CChunk::T_Folder);
	pFoldChunk->SetVersion(1);

	_MakeDataAttr(pFoldChunk, iTga_W, iTga_H);

	// Write data
	CChunkyFile::CChunk* pChunkData = pFoldChunk->AppendNew("DATA", CChunkyFile::CChunk::T_Data);
	pChunkData->SetVersion(1);
	CMemoryStore::COutStream *pOutData = CMemoryStore::OpenOutputStreamExt();

	unsigned char* pDatIn = new unsigned char[(iTga_BPP >> 3) * iTga_W * iTga_H];
	pFile->VRead(iTga_W * iTga_H, iTga_BPP / 8, pDatIn);

	if(iTga_BPP == 32)
	{
		pOutData->VWrite(iTga_W * iTga_H, iTga_BPP / 8, pDatIn);
		if(bMakeMips) _MakeTgaImagMips(pTXTR, iTga_W, iTga_H, pDatIn);
	}
	else
	{
		unsigned long iRgbaLen;
		unsigned char* pRgba = TranscodeData(pDatIn, (iTga_BPP >> 3) * iTga_W * iTga_H, iTga_BPP / 8, 4, iRgbaLen);
		pOutData->VWrite(iRgbaLen, 1, pRgba);
		if(bMakeMips) _MakeTgaImagMips(pTXTR, iTga_W, iTga_H, pRgba);
		delete[] pRgba;
	}

	delete[] pDatIn;
	pChunkData->SetData(pOutData);
	delete pOutData;

	// All done
	_LoadFromChunky();
}

unsigned char* CRgtFile::DownsizeData(unsigned char* pRGBAData, unsigned long& iWidth, unsigned long & iHeight)
{
	unsigned long iRowSkip = 0;
	if(iHeight & 1) iHeight -= 1;
	if(iWidth & 1) iWidth -= 1, iRowSkip = 4;

	if((iWidth * iHeight) < 4) return 0;

	unsigned char* pMippedData = new unsigned char[iWidth * iHeight], *pMipOut, *pInA, *pInB;
	pMipOut = pMippedData;
	pInA = pRGBAData;
	pInB = pRGBAData + (iWidth * 4) + iRowSkip;
	for(unsigned long iY = 0; iY < iHeight; iY += 2)
	{
		for(unsigned long iX = 0; iX < iWidth; iX += 2)
		{
			pMipOut[0] = ((pInA[0] + pInA[4] + pInB[0] + pInB[4]) >> 2);
			pMipOut[1] = ((pInA[1] + pInA[5] + pInB[1] + pInB[5]) >> 2);
			pMipOut[2] = ((pInA[2] + pInA[6] + pInB[2] + pInB[6]) >> 2);
			pMipOut[3] = ((pInA[3] + pInA[7] + pInB[3] + pInB[7]) >> 2);

			pMipOut += 4;
			pInA += 8;
			pInB += 8;
		}

		pInA += iRowSkip + (iWidth * 4) + iRowSkip;
		pInB += iRowSkip + (iWidth * 4) + iRowSkip;
	}

	iHeight >>= 1;
	iWidth >>= 1;
	return pMippedData;
}

void CRgtFile::_MakeTgaImagMips(CChunkyFile::CChunk* pTXTR, unsigned long iWidth, unsigned long iHeight, unsigned char* pRGBAData)
{
	pRGBAData = DownsizeData(pRGBAData, iWidth, iHeight);
	if(pRGBAData == 0) return;
	
	CChunkyFile::CChunk *pIMAG = pTXTR->InsertBefore(2, "IMAG", CChunkyFile::CChunk::T_Folder);
	pIMAG->SetVersion(1);

	_MakeDataAttr(pIMAG, iWidth, iHeight);

	// Write data
	CChunkyFile::CChunk* pChunkData = pIMAG->AppendNew("DATA", CChunkyFile::CChunk::T_Data);
	pChunkData->SetVersion(1);
	CMemoryStore::COutStream *pOutData = CMemoryStore::OpenOutputStreamExt();
	pOutData->VWrite(iWidth * iHeight, 4, pRGBAData);
	pChunkData->SetData(pOutData);
	delete pOutData;

	_MakeTgaImagMips(pTXTR, iWidth, iHeight, pRGBAData);
	delete[] pRGBAData;
}

void CRgtFile::LoadDDS(IFileStore::IStream *pFile)
{
	_Clean();

	// Make what chunkyness can be made without opening the DDS
	m_pChunky = CHECK_MEM(new CChunkyFile);
	m_pChunky->New(3);

	_MakeFileBurnInfo(m_pChunky);

	CChunkyFile::CChunk* pFoldChunk = m_pChunky->AppendNew("TSET", CChunkyFile::CChunk::T_Folder);
	pFoldChunk->SetVersion(1);
	pFoldChunk->SetDescriptor("\\ww2\\datageneric\\bia\\ww2\\art\\folder\\aaaaa\\bbbb\\cccccccc\\ddddddd\\eeeeeee\\fffff\\gggggggggggggg");

	_MakeDataData(pFoldChunk);

	pFoldChunk = pFoldChunk->AppendNew("TXTR", CChunkyFile::CChunk::T_Folder);
	pFoldChunk->SetVersion(1);
	pFoldChunk->SetDescriptor("tool:C:\\WW2\\DataGeneric\\BIA\\WW2\\Art\\folder\\Aaaaa\\Bbbb\\Cccccccc\\Ddddddd\\Eeeeeee\\Fffff\\GGGgGGgGgggggg.hhh");

	pFoldChunk = pFoldChunk->AppendNew("DXTC", CChunkyFile::CChunk::T_Folder);
	pFoldChunk->SetVersion(3);

	// Parse DDS header
	char sDDS_Magic[4];
	unsigned long iDDS_HeaderLen, iDDS_Flags, iDDS_Height, iDDS_Width, iDDS_PitchOrLinearSize, iDDS_Depth, iDDS_MipCount;
	pFile->VRead(4, 1, sDDS_Magic);
	if(strncmp(sDDS_Magic, "DDS ", 4) != 0)
	{
		_Clean();
		throw new CRainmanException(0, __FILE__, __LINE__, "Not a valid DDS file (\"%.4s\")", sDDS_Magic);
	}
	pFile->VRead(1, sizeof(long), &iDDS_HeaderLen);
	if(iDDS_HeaderLen != 124)
	{
		_Clean();
		throw new CRainmanException(0, __FILE__, __LINE__, "Not a valid DDS file (L %lu)", iDDS_HeaderLen);
	}
	pFile->VRead(1, sizeof(long), &iDDS_Flags);
	if( ((iDDS_Flags & 1) == 0) || ((iDDS_Flags & 2) == 0) || ((iDDS_Flags & 4) == 0) )
	{
		_Clean();
		throw new CRainmanException(0, __FILE__, __LINE__, "Unsupported DDS File; flags must contain at least DDSD_CAPS, DDSD_WIDTH and DDSD_HEIGHT (%lu)", iDDS_Flags);
	}
	// N = 1 | 2 | 4 | 0x1000 | (bMipLevels ? 0x20000 : 0) | 0x80000; // DDSD_CAPS, DDSD_WIDTH, DDSD_HEIGHT, DDSD_PIXELFORMAT, DDSD_MIPMAPCOUNT, DDSD_LINEARSIZE
	pFile->VRead(1, sizeof(long), &iDDS_Height);
	pFile->VRead(1, sizeof(long), &iDDS_Width);
	pFile->VRead(1, sizeof(long), &iDDS_PitchOrLinearSize);
	pFile->VRead(1, sizeof(long), &iDDS_Depth);
	pFile->VRead(1, sizeof(long), &iDDS_MipCount);
	pFile->VSeek(11 * sizeof(long));

	unsigned long iDDSPF_Size, iDDSPF_Flags;
	char sDDSPF_FourCC[5];
	sDDSPF_FourCC[4] = 0;

	pFile->VRead(1, sizeof(unsigned long), &iDDSPF_Size);
	if(iDDSPF_Size != 32)
	{
		_Clean();
		throw new CRainmanException(0, __FILE__, __LINE__, "Pixel format invalid; length is %lu", iDDSPF_Size);
	}

	pFile->VRead(1, sizeof(unsigned long), &iDDSPF_Flags);
	if( ((iDDSPF_Flags & 4) == 0)  )
	{
		_Clean();
		throw new CRainmanException(0, __FILE__, __LINE__, "Pixel format invalid; flags (%lu) must contain at least DDPF_FOURCC", iDDSPF_Flags);
	}

	pFile->VRead(4, 1, sDDSPF_FourCC);
	if(sDDSPF_FourCC[0] == 'D' && sDDSPF_FourCC[1] == 'X' && sDDSPF_FourCC[2] == 'T' && (sDDSPF_FourCC[3] == '1' || sDDSPF_FourCC[3] == '3' || sDDSPF_FourCC[3] == '5') )
	{
		// valid pixel format;
	}
	else
	{
		_Clean();
		throw new CRainmanException(0, __FILE__, __LINE__, "Only DXT1,3,5 images are supported. Use TGA for raw RGBA data.");
	}
	pFile->VSeek(5 * sizeof(long)); // Skip Pixel format: dwRGBBitCount, dwRBitMask, dwGBitMask, dwBBitMask, dwRGBAlphaBitMask

	unsigned long iDDSC_Caps1, iDDSC_Caps2;
	pFile->VRead(1, sizeof(unsigned long), &iDDSC_Caps1);
	if( (iDDSC_Caps1 & 0x1000) == 0 )
	{
		_Clean();
		throw new CRainmanException(0, __FILE__, __LINE__, "DDS Caps invalid; Cap1 (%lu) must contain at least DDSCAPS_TEXTURE", iDDSC_Caps1);
	}
	pFile->VRead(1, sizeof(unsigned long), &iDDSC_Caps2);

	pFile->VSeek(3 * sizeof(long)); // Skip DDSCaps Reserved[0-1], DDS dwReserved2

	/*
	Caps1:
	N = (bMipLevels ? 8 : 0) | 0x1000 | (bMipLevels ? 0x400000 : 0); // DDSCAPS_COMPLEX, DDSCAPS_TEXTURE, DDSCAPS_MIPMAP
	*/

	// Make DATATFMT (Texture format)
	_MakeDxtcDataTexFormat(pFoldChunk, iDDS_Width, iDDS_Height, sDDSPF_FourCC[3] - '0');

	// Make DATATMAN
	CChunkyFile::CChunk* pChunkTMAN = pFoldChunk->AppendNew("TMAN", CChunkyFile::CChunk::T_Data);
	pChunkTMAN->SetVersion(1);
	pChunkTMAN->SetUnknown1(-1);
	CMemoryStore::COutStream *pOutTMAN = CMemoryStore::OpenOutputStreamExt();

	// Make DATATDAT
	CChunkyFile::CChunk* pChunkTDAT = pFoldChunk->AppendNew("TDAT", CChunkyFile::CChunk::T_Data);
	pChunkTDAT->SetVersion(1);
	pChunkTDAT->SetUnknown1(-1);
	CMemoryStore::COutStream *pOutTDAT = CMemoryStore::OpenOutputStreamExt();

	// Write TMAN & TDAT
	_MakeDxtcData(pOutTMAN, pOutTDAT, pFile, iDDS_Width, iDDS_Height, sDDSPF_FourCC[3] - '0', (iDDSC_Caps1 & 0x400000 ? true : false) );

	pChunkTMAN->SetData(pOutTMAN);
	delete pOutTMAN;

	pChunkTDAT->SetData(pOutTDAT);
	delete pOutTDAT;

	// All done
	_LoadFromChunky();
}

void CRgtFile::Save(IFileStore::IOutputStream *pFile)
{
	try
	{
		m_pChunky->Save(pFile);
	}
	catch(CRainmanException* pE)
	{
		throw new CRainmanException(__FILE__, __LINE__, "Error writing raw chunky file", pE);
	}
}

void CRgtFile::_LoadFromChunky()
{
	// Identify image type
	m_eFormat = IF_None;

	CChunkyFile::CChunk *pChunk = m_pChunky->GetChildByName("TSET", CChunkyFile::CChunk::T_Folder);
	if(!pChunk) throw new CRainmanException(__FILE__, __LINE__, "Cannot identify image type (No FOLDTSET)");
	pChunk = pChunk->GetChildByName("TXTR", CChunkyFile::CChunk::T_Folder);
	if(!pChunk) throw new CRainmanException(__FILE__, __LINE__, "Cannot identify image type (No FOLDTXTR)");
	size_t iN = pChunk->GetChildCount();
	for(size_t i = 0; i < iN; ++i)
	{
		CChunkyFile::CChunk *pChild = pChunk->GetChild(i);
		if(pChild->GetType() == CChunkyFile::CChunk::T_Folder)
		{
			if(strcmp(pChild->GetName(), "IMAG") == 0)
			{
				m_eFormat = IF_Tga;
				break;
			}
			if(strcmp(pChild->GetName(), "DXTC") == 0)
			{
				m_eFormat = IF_Dxtc;
				break;
			}
		}
	}

	if(m_eFormat == IF_None) throw new CRainmanException(__FILE__, __LINE__, "Cannot identify image type"); 

	// Load image
	switch(m_eFormat)
	{
	case IF_Tga:
		_Load_Tga();
		break;
	case IF_Dxtc:
		_Load_Dxtc();
		break;
	};
}

void CRgtFile::Load(IFileStore::IStream *pFile)
{
	_Clean();
	m_pChunky = CHECK_MEM(new CChunkyFile);
	try
	{
		m_pChunky->Load(pFile);
	}
	catch(CRainmanException* pE)
	{
		throw new CRainmanException(__FILE__, __LINE__, "Error reading raw chunky file", pE);
	}

	_LoadFromChunky();
}

void CRgtFile::SaveGeneric(IFileStore::IOutputStream *pFile)
{
	switch(m_eFormat)
	{
	case IF_Tga:
		_Save_Tga(pFile);
		break;

	case IF_Dxtc:
		_Save_Dxtc(pFile);
		break;

	default:
		throw new CRainmanException(__FILE__, __LINE__, "Image format cannot be saved to a generic format");
		break;
	};
}

void CRgtFile::SaveDDS(IFileStore::IOutputStream *pFile, int iCompression, bool bMipLevels )
{
	if(m_iDxtCompression == iCompression && bMipLevels == (m_pMipLevels.size() > 1))
	{
		_Save_Dxtc(pFile);
		return;
	}
	if(m_iDxtCompression != iCompression || bMipLevels)
	{
		if(m_fnDecompress == 0 || m_fnCompress == 0)
		{
			throw new CRainmanException(__FILE__, __LINE__, "DXTC compressor and decompressor functions required for operation");
		}
	}

	unsigned long iMipCount = 1;
	if(bMipLevels)
	{
		unsigned long iW = m_pMipLevels[m_iMipCurrent]->m_iWidth, iH = m_pMipLevels[m_iMipCurrent]->m_iHeight;
		while(1)
		{
			if(iH & 1) iH -= 1;
			if(iW & 1) iW -= 1;
			if((iW * iH) < 4) break;
			++iMipCount;
			iH >>= 1;
			iW >>= 1;
		}
	}
	unsigned long iW = m_pMipLevels[m_iMipCurrent]->m_iWidth, iH = m_pMipLevels[m_iMipCurrent]->m_iHeight;
	unsigned long iPrimarySize = ((iW + 3) / 4) * ((iH + 3) / 4);
	if(iCompression == 1) iPrimarySize *= 8;
	else iPrimarySize *= 16;

	_Save_Dxtc_Header(pFile, (unsigned short)iW, (unsigned short)iH, iCompression, iPrimarySize, iMipCount);

	// Primary surface
	unsigned char* pRGBATop = 0;

	if(m_iDxtCompression != iCompression || bMipLevels)
	{
		pRGBATop = new unsigned char[iW * iH * 4];
		if(m_eFormat == IF_Tga)
		{
			memcpy(pRGBATop, m_pMipLevels[m_iMipCurrent]->m_pData, m_pMipLevels[m_iMipCurrent]->m_iDataLength);
			{
				size_t w = iW;
				size_t iPixCount = w * iH;

				// data seems to come out with Red and Blue swapped, so swap them back
				for(size_t i = 0; i < iPixCount; ++i)
				{
					unsigned char* p = pRGBATop + (i << 2);
					p[0] ^= p[2];
					p[2] ^= p[0];
					p[0] ^= p[2];
				}

				// The DDS was also seemingly upside down, so fix that too
				size_t w4 = w << 2;
				for(size_t i = 0, j = (iPixCount << 2) - w4; i < j; i += w4, j -= w4)
				{
					for(size_t x = 0; x < w4; ++x)
					{
						pRGBATop[i + x] ^= pRGBATop[j + x];
						pRGBATop[j + x] ^= pRGBATop[i + x];
						pRGBATop[i + x] ^= pRGBATop[j + x];
					}
				}
			}
		}
		else if(m_eFormat == IF_Dxtc)
		{
			m_fnDecompress(pRGBATop, (int)iW, (int)iH, m_pMipLevels[m_iMipCurrent]->m_pData + 16, (1 << (m_iDxtCompression >> 1)));
		}
	}

	if(m_iDxtCompression == iCompression)
	{
		pFile->VWrite(m_pMipLevels[m_iMipCurrent]->m_iDataLength, 1, m_pMipLevels[m_iMipCurrent]->m_pData + 16);
	}
	else
	{
		unsigned char* pDXTC = new unsigned char[iPrimarySize];
		m_fnCompress(pRGBATop, (int)iW, (int)iH, pDXTC, (1 << (iCompression >> 1)));
		pFile->VWrite(iPrimarySize, 1, pDXTC);
		delete[] pDXTC;
	}

	if(bMipLevels)
	{
		while(1)
		{
			unsigned char* pReduced = DownsizeData(pRGBATop, iW, iH);
			delete[] pRGBATop;
			if( (pRGBATop = pReduced) == 0) break;
			iPrimarySize = ((iW + 3) / 4) * ((iH + 3) / 4);
			if(iCompression == 1) iPrimarySize *= 8;
			else iPrimarySize *= 16;
			unsigned char* pDXTC = new unsigned char[iPrimarySize];
			m_fnCompress(pRGBATop, (int)iW, (int)iH, pDXTC, (1 << (iCompression >> 1)));
			pFile->VWrite(iPrimarySize, 1, pDXTC);
			delete[] pDXTC;
		}
	}
}

void CRgtFile::_Save_Dxtc_Header(IFileStore::IOutputStream *pFile, unsigned short iWidth, unsigned short iHeight, int iCompression, unsigned long iPrimaryLen, unsigned long iMipCount)
{
	pFile->VWrite(4, 1, "DDS ");

	unsigned long N;
	N = 124; pFile->VWrite(1, sizeof(unsigned long), &N); // dwSize

	bool bMipLevels = (iMipCount > 1);

	N = 1 | 2 | 4 | 0x1000 | (bMipLevels ? 0x20000 : 0) | 0x80000; // DDSD_CAPS, DDSD_WIDTH, DDSD_HEIGHT, DDSD_PIXELFORMAT, DDSD_MIPMAPCOUNT, DDSD_LINEARSIZE
	pFile->VWrite(1, sizeof(unsigned long), &N); // dwFlags

	N = iHeight; pFile->VWrite(1, sizeof(unsigned long), &N); // dwHeight
	N = iWidth; pFile->VWrite(1, sizeof(unsigned long), &N); // dwWidth
	N = iPrimaryLen; pFile->VWrite(1, sizeof(unsigned long), &N); // dwPitchOrLinearSize
	N = 0; pFile->VWrite(1, sizeof(unsigned long), &N); // dwDepth
	N = iMipCount; pFile->VWrite(1, sizeof(unsigned long), &N); // dwMipMapCount
	for(int i = 11; i != 0; --i) {N = 0; pFile->VWrite(1, sizeof(unsigned long), &N);} // dwReserved[0] .. dwReserved[10]
	N = 32; pFile->VWrite(1, sizeof(unsigned long), &N); // ddpfPixelFormat -> dwSize
	N = 4; pFile->VWrite(1, sizeof(unsigned long), &N); // ddpfPixelFormat -> dwFlags (DDPF_FOURCC)
	char sFourCC[] = {'D', 'X', 'T', '?', '\0'};
	sFourCC[3] = '0' + iCompression;
	pFile->VWrite(4, 1, sFourCC); // ddpfPixelFormat -> dwFourCC
	N = 0; pFile->VWrite(1, sizeof(unsigned long), &N); // ddpfPixelFormat -> dwRGBBitCount
	N = 0; pFile->VWrite(1, sizeof(unsigned long), &N); // ddpfPixelFormat -> dwRBitMask
	N = 0; pFile->VWrite(1, sizeof(unsigned long), &N); // ddpfPixelFormat -> dwGBitMask
	N = 0; pFile->VWrite(1, sizeof(unsigned long), &N); // ddpfPixelFormat -> dwBBitMask
	N = 0; pFile->VWrite(1, sizeof(unsigned long), &N); // ddpfPixelFormat -> dwRGBAlphaBitMask
	N = (bMipLevels ? 8 : 0) | 0x1000 | (bMipLevels ? 0x400000 : 0); // DDSCAPS_COMPLEX, DDSCAPS_TEXTURE, DDSCAPS_MIPMAP
	pFile->VWrite(1, sizeof(unsigned long), &N); // ddsCaps -> dwCaps1
	N = 0; pFile->VWrite(1, sizeof(unsigned long), &N); // ddsCaps -> dwCaps2
	N = 0; pFile->VWrite(1, sizeof(unsigned long), &N); // ddsCaps -> Reserved[0]
	N = 0; pFile->VWrite(1, sizeof(unsigned long), &N); // ddsCaps -> Reserved[1]
	N = 0; pFile->VWrite(1, sizeof(unsigned long), &N); // dwReserved2
}

void CRgtFile::_Save_Dxtc(IFileStore::IOutputStream *pFile)
{
	_MipLevel* pLevel = m_pMipLevels.begin()->second;
	_Save_Dxtc_Header(pFile, pLevel->m_iWidth, pLevel->m_iHeight, m_iDxtCompression, pLevel->m_iDataLength, m_pMipLevels.size());

	for(std::map<long, _MipLevel*>::iterator itr = m_pMipLevels.begin(); itr != m_pMipLevels.end(); ++itr)
	{
		pFile->VWrite(itr->second->m_iDataLength, 1, itr->second->m_pData + 16);
	}
}

void CRgtFile::SaveTGA(IFileStore::IOutputStream *pFile, bool bIncludeAlpha)
{
	try
	{
		switch(m_eFormat)
		{
		case IF_Tga:
			{
				_Save_Tga_Header(pFile, (unsigned short)m_pMipLevels[m_iMipCurrent]->m_iWidth, (unsigned short)m_pMipLevels[m_iMipCurrent]->m_iHeight, bIncludeAlpha);
				if(bIncludeAlpha) pFile->VWrite(m_pMipLevels[m_iMipCurrent]->m_iDataLength, 1, m_pMipLevels[m_iMipCurrent]->m_pData);
				else
				{
					unsigned long iShortLen;
					unsigned char* pNoAlpha = TranscodeData(m_pMipLevels[m_iMipCurrent]->m_pData, m_pMipLevels[m_iMipCurrent]->m_iDataLength, 4, 3, iShortLen);
					pFile->VWrite(iShortLen, 1, pNoAlpha);
					delete[] pNoAlpha;
				}
				return;
			}

		case IF_Dxtc:
			break;

		default:
			_Save_Tga_Header(pFile, 1, 1);
			pFile->VWrite(4, 1, "\x00\x00\x00\x00");
			return;
		};
	}
	catch(CRainmanException *pE)
	{
		throw new CRainmanException(__FILE__, __LINE__, "Simple output error", pE);
	}

	// DXTC format
	if(m_fnDecompress == 0)
	{
		throw new CRainmanException(__FILE__, __LINE__, "DXTC data requires a decompressor function");
	}

	try
	{
		_Save_Tga_Header(pFile, (unsigned short)m_pMipLevels[m_iMipCurrent]->m_iWidth, (unsigned short)m_pMipLevels[m_iMipCurrent]->m_iHeight, bIncludeAlpha);
	}
	catch(CRainmanException *pE)
	{
		throw new CRainmanException(__FILE__, __LINE__, "Output error", pE);
	}

	size_t w = m_pMipLevels[m_iMipCurrent]->m_iWidth, h = m_pMipLevels[m_iMipCurrent]->m_iHeight;
	size_t iPixCount = w * h;
	unsigned char* pRGBAData = CHECK_MEM(new unsigned char[iPixCount * 4]);
	m_fnDecompress(pRGBAData, (int)w, (int)h, m_pMipLevels[m_iMipCurrent]->m_pData + 16, (1 << (m_iDxtCompression >> 1)));

	// data seems to come out with Red and Blue swapped, so swap them back
	for(size_t i = 0; i < iPixCount; ++i)
	{
		unsigned char* p = pRGBAData + (i << 2);
		p[0] ^= p[2];
		p[2] ^= p[0];
		p[0] ^= p[2];
	}

	// The DDS was also seemingly upside down, so fix that too
	size_t w4 = w << 2;
	for(size_t i = 0, j = (iPixCount << 2) - w4; i < j; i += w4, j -= w4)
	{
		for(size_t x = 0; x < w4; ++x)
		{
			pRGBAData[i + x] ^= pRGBAData[j + x];
			pRGBAData[j + x] ^= pRGBAData[i + x];
			pRGBAData[i + x] ^= pRGBAData[j + x];
		}
	}

	try
	{
		if(bIncludeAlpha) pFile->VWrite(iPixCount * 4, 1, pRGBAData);
		else
		{
			unsigned long iShortLen;
			unsigned char* pNoAlpha = TranscodeData(pRGBAData, iPixCount * 4, 4, 3, iShortLen);
			pFile->VWrite(iShortLen, 1, pNoAlpha);
			delete[] pNoAlpha;
		}
	}
	catch(CRainmanException *pE)
	{
		delete[] pRGBAData;
		throw new CRainmanException(__FILE__, __LINE__, "Error outputting uncompressed DXTC data", pE);
	}
	delete[] pRGBAData;
	return;
}

void CRgtFile::_Save_Tga_Header(IFileStore::IOutputStream *pFile, unsigned short iWidth, unsigned short iHeight, bool bAlpha)
{
	try
	{
		pFile->VWrite(10, 1, "\x27\x00\x02\x00\x00\x00\x00\x00\x00\x00");
		unsigned short iYOrigin = 0; // iHeight;
		pFile->VWrite(1, sizeof(short), &iYOrigin);
		pFile->VWrite(1, sizeof(short), &iWidth);
		pFile->VWrite(1, sizeof(short), &iHeight);
		pFile->VWrite(2, 1, bAlpha ? "\x20\x08" : "\x18\x00");
		pFile->VWrite(39, 1, "Made with Corsix\'s Rainman <corsix.org>");
	}
	catch(CRainmanException *pE)
	{
		throw new CRainmanException(__FILE__, __LINE__, "Header output error", pE);
	}
}

void CRgtFile::_Save_Tga(IFileStore::IOutputStream *pFile)
{
	try
	{
		_Save_Tga_Header(pFile, (unsigned short)m_iWidth, (unsigned short)m_iHeight);

		pFile->VWrite(m_pMipLevels[m_iMipCurrent]->m_iDataLength, 1, m_pMipLevels[m_iMipCurrent]->m_pData);
	}
	catch(CRainmanException *pE)
	{
		throw new CRainmanException(__FILE__, __LINE__, "Output error", pE);
	}
}

void CRgtFile::_Load_Dxtc()
{
	CChunkyFile::CChunk *pChunk = m_pChunky->GetChildByName("TSET", CChunkyFile::CChunk::T_Folder);
	if(!pChunk) throw new CRainmanException(__FILE__, __LINE__, "Cannot find FOLDTSET");
	pChunk = pChunk->GetChildByName("TXTR", CChunkyFile::CChunk::T_Folder);
	if(!pChunk) throw new CRainmanException(__FILE__, __LINE__, "Cannot find FOLDTXTR");
	pChunk = pChunk->GetChildByName("DXTC", CChunkyFile::CChunk::T_Folder);
	if(!pChunk) throw new CRainmanException(__FILE__, __LINE__, "Cannot find FOLDDXTC");

	// Read info
	CChunkyFile::CChunk *pDataChunk = pChunk->GetChildByName("TFMT", CChunkyFile::CChunk::T_Data);
	if(!pDataChunk) throw new CRainmanException(__FILE__, __LINE__, "Cannot find DATATFMT");

	CMemoryStore::CStream *pStr = pDataChunk->GetData();

	pStr->VRead(1, sizeof(long), &m_iWidth);
	pStr->VRead(1, sizeof(long), &m_iHeight);
	pStr->VSeek(8, IFileStore::IStream::SL_Current);
	pStr->VRead(1, sizeof(long), &m_iDxtCompression);

	switch(m_iDxtCompression)
	{
	case 0x0D: // 0D = DXT1
		m_iDxtCompression = 1;
		break;
	case 0x0E: // 0E = DXT3 ???
		m_iDxtCompression = 3;
		break;
	case 0x0F: // 0F = DXT5
		m_iDxtCompression = 5;
		break;
	default: // umm, dunno :/
		m_iDxtCompression = 0;
		break;
	}

	delete pStr;

	// Read mip info and data
	pDataChunk = pChunk->GetChildByName("TMAN", CChunkyFile::CChunk::T_Data);
	if(!pDataChunk) throw new CRainmanException(__FILE__, __LINE__, "Cannot find DATATMAN");

	pStr = pDataChunk->GetData();

	pDataChunk = pChunk->GetChildByName("TDAT", CChunkyFile::CChunk::T_Data);
	if(!pDataChunk) throw new CRainmanException(__FILE__, __LINE__, "Cannot find DATATDAT");

	CMemoryStore::CStream *pDataStr = pDataChunk->GetData();

	pStr->VRead(1, sizeof(long), &m_iMipCount);
	for(long iMipLevel = 0; iMipLevel < m_iMipCount; ++iMipLevel)
	{
		_MipLevel* pCurrentLevel = new _MipLevel;
		long iDataLengthCompressed;

		pStr->VRead(1, sizeof(long), &pCurrentLevel->m_iDataLength);
		pStr->VRead(1, sizeof(long), &iDataLengthCompressed);

		pCurrentLevel->m_pData = new unsigned char[iDataLengthCompressed];

		pDataStr->VRead( (unsigned long)iDataLengthCompressed, 1, pCurrentLevel->m_pData);

		if(iDataLengthCompressed != pCurrentLevel->m_iDataLength)
		{
			unsigned char* pUncompressed = new unsigned char[pCurrentLevel->m_iDataLength];
			if(uncompress((Bytef*)pUncompressed, (uLongf*)&pCurrentLevel->m_iDataLength, (Bytef*)pCurrentLevel->m_pData, iDataLengthCompressed) != Z_OK)
			{
				//! \todo
			}

			delete[] pCurrentLevel->m_pData;
			pCurrentLevel->m_pData = pUncompressed;
		}

		long* pVals = (long*) pCurrentLevel->m_pData;

		m_iMipCurrent = pVals[0];
		pCurrentLevel->m_iWidth = pVals[1];
		pCurrentLevel->m_iHeight = pVals[2];
		pCurrentLevel->m_iDataLength = pVals[3];

		m_pMipLevels[m_iMipCurrent] = pCurrentLevel;
	}

	m_iWidth = m_pMipLevels[m_iMipCurrent]->m_iWidth;
	m_iHeight = m_pMipLevels[m_iMipCurrent]->m_iHeight;
	m_iDataLength = m_pMipLevels[m_iMipCurrent]->m_iDataLength;
	m_pData = m_pMipLevels[m_iMipCurrent]->m_pData + 16;

	delete pStr;
	delete pDataStr;
}

void CRgtFile::_Load_Tga()
{
	CChunkyFile::CChunk *pChunk = m_pChunky->GetChildByName("TSET", CChunkyFile::CChunk::T_Folder);
	if(!pChunk) throw new CRainmanException(__FILE__, __LINE__, "Cannot find FOLDTSET");
	pChunk = pChunk->GetChildByName("TXTR", CChunkyFile::CChunk::T_Folder);
	if(!pChunk) throw new CRainmanException(__FILE__, __LINE__, "Cannot find FOLDTXTR");

	// Read info
	CChunkyFile::CChunk *pDataChunk = pChunk->GetChildByName("INFO", CChunkyFile::CChunk::T_Data);
	if(!pDataChunk) throw new CRainmanException(__FILE__, __LINE__, "Cannot find DATAINFO");

	CMemoryStore::CStream *pStr = pDataChunk->GetData();

	pStr->VRead(1, sizeof(long), &m_iWidth);
	pStr->VRead(1, sizeof(long), &m_iHeight);

	delete pStr;

	// Read data
	size_t iN = pChunk->GetChildCount();
	for(size_t i = 0; i < iN; ++i)
	{
		CChunkyFile::CChunk *pChild = pChunk->GetChild(i);
		if(pChild->GetType() == CChunkyFile::CChunk::T_Folder)
		{
			if(strcmp(pChild->GetName(), "IMAG") == 0)
			{
				pDataChunk = pChild->GetChildByName("ATTR", CChunkyFile::CChunk::T_Data);
				if(!pDataChunk) throw new CRainmanException(__FILE__, __LINE__, "Cannot find DATAATTR");

				_MipLevel* pCurrentLevel = new _MipLevel;

				pStr = pDataChunk->GetData();
				pStr->VSeek(4, IFileStore::IStream::SL_Current);
				pStr->VRead(1, sizeof(long), &pCurrentLevel->m_iWidth);
				pStr->VRead(1, sizeof(long), &pCurrentLevel->m_iHeight);
				delete pStr;

				pDataChunk = pChild->GetChildByName("DATA", CChunkyFile::CChunk::T_Data);
				if(!pDataChunk)
				{
					delete pCurrentLevel;
					throw new CRainmanException(__FILE__, __LINE__, "Cannot find DATADATA");
				}

				pCurrentLevel->m_iDataLength = pDataChunk->GetDataLength();

				pCurrentLevel->m_pData = new unsigned char[pCurrentLevel->m_iDataLength];

				pStr = pDataChunk->GetData();
				pStr->VRead( (unsigned long)pCurrentLevel->m_iDataLength, 1, pCurrentLevel->m_pData);
				delete pStr;

				m_pMipLevels[m_iMipCurrent = m_iMipCount] = pCurrentLevel;
				++m_iMipCount;
			}
		}
	}
}

void CRgtFile::_Clean()
{
	if(m_pChunky) delete m_pChunky;
	m_pChunky = 0;

	m_eFormat = IF_None;

	m_iWidth = 0;
	m_iHeight = 0;
	m_iDataLength = 0;

	m_iMipCount = 0;
	m_iMipCurrent = 0;
	m_iDxtCompression = 0;

	for(std::map<long, _MipLevel*>::iterator itr = m_pMipLevels.begin(); itr != m_pMipLevels.end(); ++itr)
	{
		if(itr->second->m_pData) delete[] itr->second->m_pData;
		delete itr->second;
	}
	m_pMipLevels.clear();
}

long CRgtFile::GetProperty(eImageProperties eProperty)
{
	switch(eProperty)
	{
	//! \todo Other properties
	case IP_CompressionLevel:
		return m_iDxtCompression;

	case IP_MipLevelCount:
		return m_iMipCount;
	};
	return 0;
}

unsigned char* CRgtFile::TranscodeData(unsigned char* pIn, unsigned long iInByteLen, unsigned char iBppIn, unsigned char iBppOut, unsigned long& pLenOut)
{
	unsigned long iPixCount = iInByteLen / iBppIn;
	unsigned char* pOut, *p, iToCopy, iToMake, x;
	p = pOut = new unsigned char[pLenOut = (iPixCount * iBppOut)];
	if(iBppIn >= iBppOut) iToCopy = iBppOut, iToMake = 1;
	if(iBppOut > iBppIn) iToCopy = iBppIn, iToMake = iBppOut - iBppIn + 1;
	while(iPixCount--)
	{
		memcpy(p, pIn, iToCopy);
		p += iToCopy;
		pIn += iToCopy;
		x = iToMake;
		while(--x)
		{
			*p = 0xFF;
			++p;
		}
	}
	return pOut;
}
