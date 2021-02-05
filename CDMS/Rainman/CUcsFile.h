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

#ifndef _C_UCS_FILE_H_
#define _C_UCS_FILE_H_

#include "gnuc_defines.h"
// UCS = unicode character string ?
#include "IFileStore.h"
#include <wchar.h>
#include <map>
#include "Api.h"

//! A UCS (DoW locilization) file
/*!
	A set of unicode strings, each with a unique number,
	which can be loaded and saved to a UCS file.
*/
class RAINMAN_API CUcsFile
{
public:
	CUcsFile(void);
	~CUcsFile(void);

	static bool IsDollarString(const char* sString);
	static bool IsDollarString(const wchar_t* sString);

	//! Create a new UCS file
	/*!
		\return Returns nothing, but throws a CRainmanException on error
	*/
	void New();

	//! Load an existing UCS file
	/*!
		\param[in] pStream Pointer to an input stream from which to read the UCS file
		\return Returns nothing, but throws a CRainmanException on error
	*/
	void Load(IFileStore::IStream *pStream);
	void LoadDat(IFileStore::IStream *pStream);

	//! Save the current UCS file to disk
	/*!
		\param[in] sFile Fully qualified name of the file
		\return Returns nothing, but throws a CRainmanException on error
	*/
	void Save(const char* sFile);

	//! Get the unicode string associated with a number
	/*!
		\param[in] iID The identifying number
		\return a unicode string if it exists, returns 0 if no string exists
		for the requested number, throws a CRainmanException on error
	*/
	const wchar_t* ResolveStringID(unsigned long iID);

	//! Set the unicode string associated with a number
	/*!
		\param[in] iID The identifying number
		\param[in] pString The string to associate with the identifier
		\return Returns nothing, but throws a CRainmanException on error
	*/
	void SetString(unsigned long iID, const wchar_t* pString);

	//! Set the unicode string associated with a number
	/*!
		\param[in] iID The identifying number
		\param[in] pString The string to associate with the identifier. This pointer will be later freed by the class instance.
		\return Returns nothing, but throws a CRainmanException on error
	*/
	void ReplaceString(unsigned long iID, wchar_t* pString);

	//! Get the raw mappings
	/*!
		\return NULL if no mapping object exists, or the mapping object is not
		a std::map<unsigned long, wchar_t*>. Otherwise returns a pointer to the
		std::map<unsigned long, wchar_t*> object
	*/
	std::map<unsigned long, wchar_t*>* GetRawMap();
	const std::map<unsigned long, wchar_t*>* GetRawMap() const;
protected:
	//! The raw mappings
	std::map<unsigned long, wchar_t*> m_mapValues;

	//! Free memory
	/*!
		Called by the destructor to free all the allocated memory, and
		by the New() and Load() methods to clear the class ready for a
		new set of mappings.
	*/
	void _Clean();
};

#endif

