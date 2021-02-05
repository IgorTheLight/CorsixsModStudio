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

#ifndef _C_RGT_FILE_H_
#define _C_RGT_FILE_H_

#include "gnuc_defines.h"
#include "CChunkyFile.h"
#include <map>

class RAINMAN_API CRgtFile
{
public:

	/*
		DXT1, 3 and 5 compression and decompression are not done by this library,
		but it is designed to allow a library like squish (sjbrown.co.uk) to be
		plugged in. The compressor function should work like squish's function
		'CompressImage', the decompressor like 'DecompressImage'.
	*/
	typedef void (*tfnDXTCodec)(unsigned char*, int, int, void*, int);
	bool isDXTCodecPresent();
	void setDXTCodec(tfnDXTCodec fnCompress, tfnDXTCodec fnDecompress);

	CRgtFile();
	~CRgtFile();

	void Load(IFileStore::IStream *pFile);
	void Save(IFileStore::IOutputStream *pFile);

	void LoadDDS(IFileStore::IStream *pFile);
	void LoadTGA(IFileStore::IStream *pFile, bool bMakeMips = false, bool* pIs32Bit = 0);

	/*
		Saves to DDS format for DXTC content, TGA format for RGBA content.
	*/
	void SaveGeneric(IFileStore::IOutputStream *pFile);

	/*
		Writes the current mip level as an RGBA TGA, apart from this, is
		same as SaveGeneric for RGBA content. For DXT1,3 and 5 content,
		will use the DXT decompressor function set with setDXTCodec to
		try and turn the DXTC to RGBA. Will fail for DXTC content if no
		decompressor was assigned.
	*/
	void SaveTGA(IFileStore::IOutputStream *pFile, bool bIncludeAlpha = true);

	void SaveDDS(IFileStore::IOutputStream *pFile, int iCompression, bool bMipLevels );

	// Information accessors
	enum eImageFormats
	{
		IF_None, //!< No image
		IF_Tga, //!< TGA image
		IF_Dxtc, //!< DXT compressed image
	};

	eImageFormats GetImageFormat();

	enum eImageProperties
	{
		// Usable with all types apart from IF_None :
		IP_Width, //!< Image width (in pixels)
		IP_Height, //!< Image height (in pixels)
		IP_DataLength, //!< Size of the image data (in bytes)
		IP_MipLevelCount, //!< Number of mip levels present
		IP_MipLevel, //!< Current mip level

		// Usable with IF_Dxtc:
		IP_CompressionLevel, //!< 1 = DXT1, 2 = DXT2 ... 5 = DXT5 (0 = unknown)
	};

	long GetProperty(eImageProperties eProperty);

	unsigned char* GetImageData();

protected:
	void _Clean();

	void _LoadFromChunky();

	void _Load_Tga();
	void _Save_Tga(IFileStore::IOutputStream *pFile);
	static void _Save_Tga_Header(IFileStore::IOutputStream *pFile, unsigned short iWidth, unsigned short iHeight, bool bAlpha = true);

	void _Load_Dxtc();
	void _Save_Dxtc(IFileStore::IOutputStream *pFile);
	static void _Save_Dxtc_Header(IFileStore::IOutputStream *pFile, unsigned short iWidth, unsigned short iHeight, int iCompression, unsigned long iPrimaryLen, unsigned long iMipCount);

	static unsigned char* TranscodeData(unsigned char* pIn, unsigned long iInByteLen, unsigned char iBppIn, unsigned char iBppOut, unsigned long& pLenOut);
	static unsigned char* DownsizeData(unsigned char* pIn, unsigned long& iWidth, unsigned long & iHeight);

	CChunkyFile* m_pChunky;
	eImageFormats m_eFormat;

	long m_iWidth;
	long m_iHeight;
	long m_iDataLength;

	long m_iMipCount;
	long m_iMipCurrent;
	long m_iDxtCompression;

	unsigned char* m_pData;

	struct _MipLevel
	{
		long m_iWidth;
		long m_iHeight;
		long m_iDataLength;
		unsigned char* m_pData; //!< In DXTC images, actual data = this + 16
	};
	std::map<long, _MipLevel*> m_pMipLevels;

	tfnDXTCodec m_fnCompress, m_fnDecompress;

	// Chunk makers
	static void _MakeFileBurnInfo(CChunkyFile* pChunkyFile);
	static void _MakeDataData(CChunkyFile::CChunk* pParentChunk, long iVal1 = 0, long iVal2 = 0, int iType = 3);
	static void _MakeDataAttr(CChunkyFile::CChunk* pParentChunk, long iWidth, long iHeight, long iUnknown = 0);
	static void _MakeDataInfo(CChunkyFile::CChunk* pParentChunk, long iWidth, long iHeight, char iFinalByte = 0);
	static void _MakeDxtcDataTexFormat(CChunkyFile::CChunk* pParentChunk, unsigned long iWidth, unsigned long iHeight, int iDxtN);
	static void _MakeDxtcData(CMemoryStore::COutStream* pTMAN, CMemoryStore::COutStream* pTDAT, IFileStore::IStream* pDDS, unsigned long iWidth, unsigned long iHeight, int iDxtN, bool bMipMaps, int iThisMipLevel = 0);
	static void _MakeTgaImagMips(CChunkyFile::CChunk* pTXTR, unsigned long iWidth, unsigned long iHeight, unsigned char* pRGBAData);
};

#endif // #ifndef _C_RGT_FILE_H_

