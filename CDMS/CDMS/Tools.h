/*
    This file is part of Corsix's Mod Studio.

    Corsix's Mod Studio is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Corsix's Mod Studio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Corsix's Mod Studio; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _CDMS_TOOLS_H_
#define _CDMS_TOOLS_H_
#include "Construct.h"
#include "frmUCSSelector.h"

class CLocaleTool : public ConstructFrame::ITool
{
public:
	virtual wxString GetName();
	virtual wxString GetHelpString();
	virtual wxString GetBitmapName();

	virtual void DoAction();
};

class CAESetupTool : public ConstructFrame::ITool
{
public:
	virtual wxString GetName();
	virtual wxString GetHelpString();
	virtual wxString GetBitmapName();

	virtual void DoAction();
};

class CUcsTool : public ConstructFrame::ITool
{
public:
	virtual wxString GetName();
	virtual wxString GetHelpString();
	virtual wxString GetBitmapName();

	virtual void DoAction();
	static void HandleSelectorResponse(frmUCSSelector* pSelector, wxAuiNotebook* pTabsDestination, unsigned long* pResult = 0, bool bRegisterTabStrip = false);
};

class CAttribSnapshotTool : public ConstructFrame::ITool
{
public:
	virtual wxString GetName();
	virtual wxString GetHelpString();
	virtual wxString GetBitmapName();

	virtual void DoAction();
};

class CSgaPackerTool : public ConstructFrame::ITool
{
public:
	virtual wxString GetName();
	virtual wxString GetHelpString();
	virtual wxString GetBitmapName();

	virtual void DoAction();
};

class CExtractAllTool : public ConstructFrame::ITool
{
public:
	virtual wxString GetName();
	virtual wxString GetHelpString();
	virtual wxString GetBitmapName();

	virtual void DoAction();
};

class CDpsCalculatorTool : public ConstructFrame::ITool
{
public:
	virtual wxString GetName();
	virtual wxString GetHelpString();
	virtual wxString GetBitmapName();

	virtual void DoAction();
};

class CRedButtonTool : public ConstructFrame::ITool
{
public:
	virtual wxString GetName();
	virtual wxString GetHelpString();
	virtual wxString GetBitmapName();

	virtual void DoAction();
};

class CMakeLuaInheritTree : public ConstructFrame::ITool
{
protected:
	static void _ForEach(IDirectoryTraverser::IIterator* pItr, void* pTag);
	void _DoLua(IDirectoryTraverser::IIterator* pItr);
public:
	CMakeLuaInheritTree();
	virtual wxString GetName();
	virtual wxString GetHelpString();
	virtual wxString GetBitmapName();

	static bool _DoesExist(const char* sFol);
	void Do(const char* sAttrib);
	virtual void DoAction();
	CInheritTable* pTable;
	size_t iAttribL;
};

class CRefreshFilesTool : public ConstructFrame::ITool
{
public:
	virtual wxString GetName();
	virtual wxString GetHelpString();
	virtual wxString GetBitmapName();

	virtual void DoAction();
};

#endif