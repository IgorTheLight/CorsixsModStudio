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

#ifndef _I_DIRECTRY_TRAVERSER_H_
#define _I_DIRECTRY_TRAVERSER_H_

#include "gnuc_defines.h"
#include "IFileStore.h"
#include "WriteTime.h"
#include "Api.h"

//! Directory traversiong interface
/*!
	A directory traversing interface is designed to allow you to iterate over the contents of a
	directory, get information about the file system entry points and also create directories.
	In this way it almost extends IFileStore to do more than the bare minimum.
	If a class extend this interface and the IFileStore interface then it is a good idea to call the class's VInit()
	method before calling any of the methods in this interface.

	\par Path names
	Firstly are entry points; these are "entry points" into the directory structure. For the file system, entry points are drive letters. For SGA archives they are TOC entries. <BR>
	A regex for file paths is [:entry point:](\[:directory:])*\[:file name:] <BR>
	A regex for directory paths is [:entry point:](\[:directory:])*\? <BR>
	Entry points, file names and directories can use any alphanumeric characters apart from '\0' and '\\', and any naming limitations imposed from the underlying system <BR>
*/
class RAINMAN_API IDirectoryTraverser
{
public:
	IDirectoryTraverser(void);
	virtual ~IDirectoryTraverser(void);

	//! A directory contents iterator
	/*!
		A directory contents iterator is capable of getting information about the current item
		and moving onto the next item. In this way it is similar to a simple STL iterator.
		It is extendable with the VGetTag() method, which implementations can use to allow applications to
		get specific/custom information about the current item.
	*/
	class RAINMAN_API IIterator
	{
	protected:
		IIterator();

	public:
		virtual ~IIterator();

		//! Enumerated type to identify what type of item the current item is
		enum eTypes
		{
			T_File, //!< A file
			T_Directory, //!< A directory
			T_Nothing //!< Nothing (end of directory / blank directory)
		};

		//! Returns the type of the current object
		/*!
			\return Returns a valid value or throws a CRainmanException
			\see IDirectoryTraverser::IIterator::eTypes
		*/
		virtual eTypes VGetType() = 0;

		//! Create an iterator to iterate over the current directory item
		/*!
			If the current item is a directory (VGetType() == T_Directory) then this function can be
			called to iterate over this sub directory.
			\return Returns a valid pointer, although may return 0 for an empty sub-dir and will throw a CRainmanException on error. Remember to delete this pointer when you are done using it.
		*/
		virtual IIterator* VOpenSubDir() = 0;

		//! Create a stream to read from the current file item
		/*!
			If the current item is a file (VGetType() == T_File) then this function can be
			called to open a stream to read from the file.
			\return Returns a valid pointer, but will throw a CRainmanException on error. Remember to delete this pointer when you are done using it.
		*/
		virtual IFileStore::IStream* VOpenFile() = 0;

		//! Get the name of the current item
		/*!
			For files, gives the file name, eg. "badwords.lua", "somefile.rgd"
			For directories, gives the directory name, eg. "attrib" or "axis"
			Read IDirectoryTraverser for information on file and directory naming.
			Use VGetFullPath() to get the absolute path of the item
			\return Returns a valid pointer or throws a CRainmanException
			\see VGetFullPath VGetDirectoryPath
		*/
		virtual const char* VGetName() = 0;

		//! Get the full path of the current item
		/*!
			For files, gives the full file name, eg. "Data\onlineconfig\badwords.lua", "C:\stuff\somefile.rgd"
			For directories, gives the directory name, eg. "Data\attrib" or "Data\art\ingame\axis"
			Read IDirectoryTraverser for information on file and directory naming.
			\return Returns a valid pointer or throws a CRainmanException
			\see VGetName VGetDirectoryPath
		*/
		virtual const char* VGetFullPath() = 0;

		//! Get the directory path of the current item (aka. the name of the directory being iterated)
		/*!
			Read IDirectoryTraverser for information on file and directory naming.
			\return Returns a valid pointer or throws a CRainmanException
			\see VGetName VGetFullPath
		*/
		virtual const char* VGetDirectoryPath() = 0;

		//! Get the time at which the current file item was last modified
		/*!
			Will probably only work when VGetType() == T_File
			\return Returns a valid value or throws a CRainmanException
		*/
		virtual tLastWriteTime VGetLastWriteTime() = 0;

		//! Enumerated type for the VNextItem() return value
		/*!
			\sa VNextItem()
		*/
		enum eErrors
		{
			E_OK, //!< No errors
			E_AtEnd //!< Now at end of directory; all items have been iterated over
		};

		//! Move onto the next item in the current directory
		/*!
			Will move the iterator onto the next item in the current directory
			and inform the caller if the end of the current directory has been reached.
			\return Returns E_OK, E_AtEnd or throws a CRainmanException
		*/
		virtual eErrors VNextItem() = 0;

		//! Get implementation dependant information
		/*!
			Different implementations of this interface can use this function to allow an
			application to get custom information about the current item.
			\param[in] iTag An identifier of some sorts specifying which information to get
			\return Returns an implementation dependant value; may throw a CRainmanException
		*/
		virtual void* VGetTag(long iTag);
	};

	//! Iterate over the contents of a directory
	/*!
		\param[in] sPath The name of the directory to iterate. Read IDirectoryTraverser for information on directory paths.
		\return Returns a valid pointer, or may return 0 if the directory is empty, or may throw a CRainmanException
	*/
	virtual IIterator* VIterate(const char* sPath) = 0;

	//! Get the number of entry points available to iterate over
	/*!
		See IDirectoryTraverser for information on path names and entry points
		\sa VGetEntryPoint()
		\return Returns 0 or a positive value; cannot throw a CRainmanException
	*/
	virtual unsigned long VGetEntryPointCount() = 0;

	//! Get the name of a specified entry point
	/*!
		See IDirectoryTraverser for information on path names and entry points
		The return value from this can be passed directly to VIterate() to iterate over the entry point
		\param[in] iID The ID of the entry point to get the name of (Range is 0 to VGetEntryPointCount() - 1 inclusive)
		\return Returns a valid pointer or throws a CRainmanException
	*/
	virtual const char* VGetEntryPoint(unsigned long iID) = 0;

	//! Create a new folder somewhere in the directory structure
	/*!
		See IDirectoryTraverser for information on path names. <br>
		If the new folder is created then existing iterators iterating over sPath may not be aware of its existance.
		\param[in] sPath The path of an existing directory in the directory structure
		\param[in] sNewFolderName The name of the new folder to create within the specified directory
		\return Returns no value but throws a CRainmanException on error
	*/
	virtual void VCreateFolderIn(const char* sPath, const char* sNewFolderName) = 0;

	//! Get the last modification date of a file
	/*!
		See IDirectoryTraverser for information on path names and file names.
		\param[in] sPath The path of a file in the directory structure
		\return Returns a valid value or throws a CRainmanException
	*/
	virtual tLastWriteTime VGetLastWriteTime(const char* sPath) = 0;
};

#endif

