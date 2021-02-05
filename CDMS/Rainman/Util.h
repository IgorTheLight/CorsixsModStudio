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

#ifndef _RAINMAN_UTIL_H_
#define _RAINMAN_UTIL_H_
#include "gnuc_defines.h"

#include "CRgdHashTable.h"
#include "IDirectoryTraverser.h"
#include "Api.h"

//! Retrieves the Dawn of War installation path
/*!
	Looks for the DoW installation path in the registry,
	if not found looks for the Demo installation path,
	if not found returns default install location

	\return Returns a valid pointer or throws a CRainmanException

	\attention You MUST delete[] the result

	\author Corsix
*/
RAINMAN_API char* Rainman_GetDoWPath();

//! Retrieves the Dawn of War: Dark Crusade installation path
/*!
	Looks for the DoW:DC installation path in the registry,
	if not found returns default install location

	\return Returns a valid pointer or throws a CRainmanException

	\attention You MUST delete[] the result

	\author Corsix
*/
RAINMAN_API char* Rainman_GetDCPath();

//! Retrieves the Dawn of War: Soul Storm installation path
/*!
	Looks for the DoW:SS installation path in the registry,
	if not found returns default install location

	\return Returns a valid pointer or throws a CRainmanException

	\attention You MUST delete[] the result

	\author Corsix
*/
RAINMAN_API char* Rainman_GetSSPath();

//! Retrieves the Company of Heroes installation path
/*!
	Looks for the CoH installation path in the registry,
	if not found looks for the Beta installation path,
	if not found returns default install location

	\return Returns a valid pointer or throws a CRainmanException

	\attention You MUST delete[] the result

	\author Corsix
*/
RAINMAN_API char* Rainman_GetCoHPath();

RAINMAN_API void Rainman_DeleteCharArray(char* pString);

typedef void (*Rainman_ForEachFunction)(IDirectoryTraverser::IIterator*, void* pTag);
RAINMAN_API void Rainman_ForEach(IDirectoryTraverser::IIterator* pDirectory, Rainman_ForEachFunction pFn, void* pTag, bool bRecursive);

//! Loads all the RGD dictionaries from a specified folder
/*!
	All .txt files in the specified folder are loaded as RGD hash table dictionaries
	\param[in] sPath The folder to iterate over and find dictionaries in. Subdirectories are not searched.
	\param[out] sCustom If passed, the full path of the custom dictionary is strdup()ped here (If no custom dicitonary is found, nothing is done with this parameter)
	\param[in] bIgnoreLoadErrors If set to true, any errors which occour when loading a dictionary are ignored

	\return Returns a valid pointer or throws a CRainmanException

	\attention You MUST delete the result. You MUST free() *sCustom if sCustom is specified

	\author Corsix
*/
RAINMAN_API CRgdHashTable* Rainman_LoadDictionaries(const char* sPath, char** sCustom = 0, bool bIgnoreLoadErrors = true);

#endif

