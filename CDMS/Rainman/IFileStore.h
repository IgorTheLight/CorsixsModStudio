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

#ifndef _I_FILE_STORE_H_
#define _I_FILE_STORE_H_

#include "gnuc_defines.h"
#include "Api.h"

//! Input/output interface
/*!
	An IFileStore is used to create input and output streams
	You must begin by calling VInit()
*/
class RAINMAN_API IFileStore
{
public:
	IFileStore(void);
	virtual ~IFileStore(void);

	//! An input/read stream interface
	/*!
		An input stream is capable of reading from a stream, seeking to a location
		in the stream and telling you where in the stream you are.
	*/
	class RAINMAN_API IStream
	{
	protected:
		//! Constructor
		/*!
			Constructor is protected to ensure that it can only be created by the IFileStore methods
			Use IFileStore::VOpenStream() to create an IStream
		*/
		IStream(void);
	public:
		virtual ~IStream(void);

		//! Location constants for VSeek()
		/*!
			These are counterparts to fseek()'s SEEK_CUR, SEEK_SET and SEEK_END
		*/
		enum SeekLocation
		{
			SL_Current, //!< Seek relative to the current location (SEEK_CUR)
			SL_Root, //!< Seek relative to the begginning of the file (SEEK_SET)
			SL_End //!< Seek relative to the end of the file (SEEK_END)
		};

		//! Read data from the stream
		/*!
			\param[in] iItemCount The number of items to read
			\param[in] iItemSize The size (in bytes) of each item
			\param[out] pDestination The destination buffer for the read data
			\return Returns nothing, but throws a CRainmanException on error (eg. trying to read beyond end of stream)
		*/
		virtual void VRead(unsigned long iItemCount, unsigned long iItemSize, void* pDestination) = 0;

		template <class T> inline void Read(T& rDestination) {VRead(1,sizeof(T),&rDestination);}

		//! Seek to a position in the stream
		/*!
			iPosition must be negative, 0 or positive with SeekLocation::SL_Current
			iPosition must be 0 or positive with SeekLocation::SL_Root
			iPosition must be 0 or negative with SeekLocation::SL_End

			\param[in] iPosition Where to seek relative to SeekFrom
			\param[in] SeekFrom The position to seek from
			\return Returns nothing, but throws a CRainmanException on error
		*/
		virtual void VSeek(long iPosition, SeekLocation SeekFrom = SL_Current) = 0;

		//! Get the current position in the stream
		/*!
			\return Returns the position in the stream relative to thee begginning, throws a CRainmanException on eror
		*/
		virtual long VTell() = 0;
	};

	//! An output/write stream interface
	/*!
		An output stream is capable of all the things an input stream is capable of
		(reading from a stream, seeking to a location in the stream and telling you where in the stream you are)
		and also capable of writing data to the stream.

		VSeek() still cannot seek beyond the end of the file, even with an output stream.
	*/
	class RAINMAN_API IOutputStream : public IStream
	{
	protected:
		//! Constructor
		/*!
			Constructor is protected to ensure that it can only be created by the IFileStore methods
			Use IFileStore::VOpenStream() to create an IStream
		*/
		IOutputStream(void);
	public:
		virtual ~IOutputStream(void);

		//! Write data to the stream
		/*!
			Writes data to the current location of the stream; overwiriting existing data, or adding new data to the end of the file.
			\param[in] iItemCount The number of items to write
			\param[in] iItemSize The size (in bytes) of each write
			\param[in] pSource The source buffer which holds the data to be written
			\return Returns nothing, but throws a CRainmanException on error (eg. trying to read beyond end of stream)
		*/
		virtual void VWrite(unsigned long iItemCount, unsigned long iItemSize, const void* pSource) = 0;

		template <class T>
		inline void VWriteString(const T* sString, size_t (* pSizeFn)(const T*) = strlen)
		{
			if(sString != 0)
				VWrite( (unsigned long)pSizeFn(sString), sizeof(T), sString);
		}
	};


	//! Sets up the file store ready to be used
	/*!
		\param[in] pInitData Classes which inherit this interface will have different requirements for this parameter
		\return Returns nothing, but throws a CRainmanException on error
	*/
	virtual void VInit(void* pInitData = 0);

	//! Open an input/read stream
	/*!
		\param[in] sIdentifier For most file stores this will be the file path (See IDirectoryTraverser for information on file path naming)
		\return Returns a valid stream or throws an exception. Cannot return zero. Remember to delete this pointer when you are done using it.
	*/
	virtual IStream* VOpenStream(const char* sIdentifier) = 0;

	//! Open an output/write stream
	/*!
		Not all file stores will support output streams (and hence the function is not pure virtual)
		\param[in] sIdentifier For most file stores this will be the file path (See IDirectoryTraverser for information on file path naming)
		\param[in] bEraseIfPresent If set to true then when the stream is opened, if the file has any existing content then that is destroyed
		\return Returns a valid stream or throws a CRainmanException. Cannot return zero. Remember to delete this pointer when you are done using it.
	*/
	virtual IOutputStream* VOpenOutputStream(const char* sIdentifier, bool bEraseIfPresent);

protected:
	bool m_bInited;
};

#endif

