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

#ifndef _C_UCS_TRANSACTION_H_
#define _C_UCS_TRANSACTION_H_

#include "gnuc_defines.h"
// UCS = unicode character string ?
#include "IFileStore.h"
#include <wchar.h>
#include <map>
#include "Api.h"
#include "CUcsFile.h"

class RAINMAN_API CUcsTransaction : public CUcsFile
{
public:
	CUcsTransaction(CUcsFile* pUcsObject);
	~CUcsTransaction(void);

	//! Load an existing UCS file
	/*!
		Passes call onto stored UCS object, throws out any uncommited changes to the currently loaded
		\param[in] pStream Pointer to an input stream from which to read the UCS file
		\return Returns nothing, but throws a CRainmanException on error
	*/
	void Load(IFileStore::IStream *pStream);

	//! Save the current UCS file to disk
	/*!
		Commits changes and then saves the UCS object
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

	//! Get the raw mappings
	/*!
		\return NULL if no mapping object exists, or the mapping object is not
		a std::map<unsigned long, wchar_t*>. Otherwise returns a pointer to the
		std::map<unsigned long, wchar_t*> object
	*/
	std::map<unsigned long, wchar_t*>* GetRawMap();

	CUcsFile* GetRawObject();
protected:
	CUcsFile* m_pRawFile;
	std::map<unsigned long, wchar_t*> m_mapCombinationValues;
};

#endif

