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

/*
#ifndef _C_COH_UCS_FILE_H_
#define _C_COH_UCS_FILE_H_

#include "IFileStore.h"
#include <map>
#include <wchar.h>
#include <vector>

class CCohUcsFile
{
public:
	CCohUcsFile();
	~CCohUcsFile();

	bool New();

	bool Load(IFileStore::IStream *pStream);

private:
	struct _tRange
	{
		_tRange();
		~_tRange();

		char* sComment;
		unsigned long iBegin, iEnd;
		std::map<unsigned long, wchar_t*> mapEntries;
	};

	char* m_sFileComment;
	unsigned long m_iBegin, m_iEnd;
	std::vector<_tRange*> m_vRanges;

	void _Clean();
};

#endif
*/

