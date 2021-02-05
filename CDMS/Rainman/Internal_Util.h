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

#ifndef _INTERNAL_UTIL_H_
#define _INTERNAL_UTIL_H_

#include "gnuc_defines.h"
#include <stdio.h>
#include "IDirectoryTraverser.h"

char* Util_fgetline(FILE *f, unsigned int iInitSize = 32);
void Util_TrimWhitespace(char** pStringPointer);
char* Util_mystrdup(const char* sStr);
void Util_strtolower(char* sStr);

typedef void (*Util_ForEachFunction)(IDirectoryTraverser::IIterator*, void* pTag);
void Util_ForEach(IDirectoryTraverser::IIterator* pDirectory, Util_ForEachFunction pFn, void* pTag, bool bRecursive);

void Util_EnsureEndsWith(char* sStr, char cChar);
void Util_EnsureEndsWith(char* sStr, char cChar, char cChar2);

template <class T>
class AutoDelete
{
public:
	T* p;
	bool a;
	AutoDelete(T* pp, bool bArray) : p(pp), a(bArray) {}
	~AutoDelete() {if(a) delete[] p; else delete p;}
	void reset(T* pp, bool bArray) {p = pp; a = bArray;}
	void del() {if(a) delete[] p; else delete p; p = 0;}
};

#endif

