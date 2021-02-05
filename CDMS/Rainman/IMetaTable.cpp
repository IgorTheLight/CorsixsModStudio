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

#include "IMetaTable.h"
#include "memdebug.h"
#include "Exception.h"

IMetaNode::IMetaNode()
{
}

IMetaNode::~IMetaNode()
{
}

IMetaNode::IMetaTable::IMetaTable(void)
{
}

IMetaNode::IMetaTable::~IMetaTable(void)
{
}

bool IMetaNode::IMetaTable::VSupportsRefresh()
{
	return false;
}

void IMetaNode::IMetaTable::VDoRefresh()
{
	QUICK_THROW("Refresh not supported")
}

unsigned long IMetaNode::VGetNameHash()
{
	throw new CRainmanException(__FILE__, __LINE__, "Variable has no name hash");
}

void IMetaNode::VSetNameHash(unsigned long iHash)
{
	throw new CRainmanException(__FILE__, __LINE__, "NameHash cannot be set");
}

