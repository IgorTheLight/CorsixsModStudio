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

#include "IDirectoryTraverser.h"
#include "memdebug.h"
#include "Exception.h"

IDirectoryTraverser::IDirectoryTraverser(void)
{
}

IDirectoryTraverser::~IDirectoryTraverser(void)
{
}

IDirectoryTraverser::IIterator::IIterator(void)
{
}

IDirectoryTraverser::IIterator::~IIterator(void)
{
}

void* IDirectoryTraverser::IIterator::VGetTag(long iTag)
{
	throw new CRainmanException(0, __FILE__, __LINE__, "Unknown tag %li", iTag);
}

