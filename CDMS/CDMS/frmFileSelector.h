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

#ifndef _FRM_FILESELECTOR_H_
#define _FRM_FILESELECTOR_H_
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
// ----------------------------
#include <Rainman.h>
#include <list>
#include <wx/treectrl.h>

class CFileSelectorTreeItemData : public wxTreeItemData
{
public:
	CFileSelectorTreeItemData(IDirectoryTraverser::IIterator* pItr, bool bToFillWith);
	const char* sMod;
	const char* sSource;

	IDirectoryTraverser::IIterator* pToFillWith;
};

class frmFileSelector : public wxDialog
{
protected:
	wxString m_sBaseFolder;
	wxString m_sFile;
	wxString m_sRgdModeExt;
	wxTreeCtrl* m_pTree;
	wxTextCtrl* m_pText;
	wxColour m_cThisMod, m_cOtherMod, m_cEngine;

	void MakeChildren(const wxTreeItemId& parent);
	bool m_bRgdMode;

	void FindAndSelect(wxString sFile);

public:
	frmFileSelector(wxString sBaseFolder, wxString sExistingSelection, bool bRgdMode);
	virtual ~frmFileSelector();

	void OnSize(wxSizeEvent& event);

	void OnOkClick(wxCommandEvent& event);
	void OnCloseClick(wxCommandEvent& event);
	void OnTreeExpanding(wxTreeEvent& event);
	void OnTreeSelect(wxTreeEvent& event);

	inline wxString& GetBaseFolder() {return m_sBaseFolder;}
	inline wxString& GetFile() {return m_sFile;}

	enum
	{
		IDC_FilesTable = wxID_HIGHEST + 1,
	};

	DECLARE_EVENT_TABLE()
};

#endif