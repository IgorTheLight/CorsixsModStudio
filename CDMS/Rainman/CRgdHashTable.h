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

#ifndef _C_RGD_HASH_TABLE_H_
#define _C_RGD_HASH_TABLE_H_

#include "gnuc_defines.h"
#include <stdio.h>
#include <map>
#include <vector>
#include "Api.h"
//#include "IFileStore.h"

class RAINMAN_API CRgdHashTable
{
public:
	//! Constructor
	/*!
		Will not throw a CRainmanException
	*/
	CRgdHashTable(void);
	~CRgdHashTable(void);

	//! Creates a new (empty) RGD hash table
	/*!
		\return Returns no value, but may throw a CRainmanException on error
	*/
	void New();

	//! Attempts to extend the hash table using an RGD dictionary file
	/*!
		Loads the file using the RGD dictionary specification: <br>
		First line (and the header) SHALL be '#RGD_DIC'. <br>
		Lines MAY be of any length. <br>
		hash ('#') MAY be used as a comment mark: it MAY be used in front or end of lines, whatever is behind it SHALL ignored until the next line. <br>
		The format of the data contained within SHALL be: <br>
			[code]=[token], <br>
			where [code] SHALL be the hexadecimal value within the RGD file prefixed using '0x' notation - the hexadecimal numbers including the '0x' notation SHALL be case insensitive, <br>
			while [token] SHALL be the text token represented by the [code]. <br>
		Whitespaces such as space and tab characters anywhere within the <code> and/or <token> SHALL be ignored. <br>
		Blank lines SHALL be ignored. <br>
		The entries within the file MAY be sorted, however it is NOT MANDATORY. <br>
		The file SHOULD NOT contain duplicate entries. <br>
		The file MAY terminate with a blank line, however it is NOT MANDATORY. <br>
		There SHALL be no copyrights claimed, anyone SHALL be able to obtain this file from any source, to freely modify according to personal needs. It SHALL be possible to freely distribute this file in its original or modified form. <br>
		
		\param[in] sFile The name of a file on hard disk (eg. openable using fopen() ) to load as a dictionary
		\param[in] bCustom If set, all entries loaded from this file will be flagged as "custom"
		\return Returns nothing if successful, or throws a CRainmanException on error
	*/
	void ExtendWithDictionary(const char* sFile, bool bCustom = false);

	void XRefWithStringList(const char* sFile);

	//! Saves all keys flagged as "custom" to a specified file
	/*!
		Key/value pairs can be flagged as "custom" in two ways: <br>
		- When ExtendWithDictionary() has bCustom set to true, all pairs loaded from the file are flagged <br>
		- When ValueToHash() is called and the returned hash is one not already in the dictionary, that is flagged

		Creates a dictionary file loadable by ExtendWithDictionary() from all of the key/value pairs flagged as "custom"

		\param[in] sFile The name of the file to save the custom key/value pairs to (if the file already exists then the contents of it will be wiped)
		\return Returns nothing if successful, but throws a CRainmanException on error
	*/
	void SaveCustomKeys(const char* sFile);

	//! Attempts to resolve a hash to a string value
	/*!
		\param[in] iHash
		\return Returns the string, and returns 0 if the hash could not be resolved
	*/
	const char* HashToValue(unsigned long iHash);

	//! Gives the hash value for a string value
	/*!
		\param[in] sValue
		\return Returns the hash associated with sValue, but will throw a CRainmanException on error
	*/
	unsigned long ValueToHash(const char* sValue);
	
	static unsigned long ValueToHashStatic(const char* sValue);
	static unsigned long ValueToHashStatic(const char* sValue, size_t iValueLen);


	//! Allows an application to know all of the hashes that the dictionary could not resolve
	/*!
		Whenever HashToValue() is called, and the hash is not in the dictionary, then
		the hash is remembered as an "unknown" one. This function will fill the passed
		list with all of these unknown hashes.

		\param[in] oList The list to fill
		\return Returns no value and never throws a CRainmanException
	*/
	void FillUnknownList(std::vector<unsigned long>& oList);

protected:
	struct _Value
	{
		const char* sString;
		bool bCustom;
		_Value();
	};
	std::map<unsigned long, _Value> m_mHashTable;

	void _Clean();
};

#endif

