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

#ifndef _FRM_RGMMATERIALEDITOR_H_
#define _FRM_RGMMATERIALEDITOR_H_
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
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/manager.h>
#include <wx/splitter.h>
#include <wx/treectrl.h>
#include <list>

class RgmMaterialwxPropertyGridManager;

class frmRgmMaterialEditor : public wxWindow
{
protected:
	wxPropertyGrid* m_pPropertyGrid;
	wxTreeCtrl* m_pTables;
	wxSplitterWindow* m_pSplitter;
	RgmMaterialwxPropertyGridManager* m_pPropManager;
	wxString m_sFilename;
	wxTreeItemId m_oFileParent;

	CRgmFile* m_pRgmFile;
	bool m_bOwnRgm;

	void _FillLeft();
	void _FillRight(CRgmFile::CMaterial* pMaterial);

	wxPGProperty* GetVariableEditor(CRgmFile::CMaterial::CVariable* pVar);
	
public:
	frmRgmMaterialEditor(wxTreeItemId& oFileParent, wxString sFilename, wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	~frmRgmMaterialEditor();

	void SetObject(CRgmFile* pRgmFile, bool bTakeOwnership);

	void OnSize(wxSizeEvent& event);
	void OnPropertyChange(wxPropertyGridEvent& event);
	void OnTreeSelect(wxTreeEvent& event);
	void OnTreeRightClick(wxTreeEvent& event);
	void OnSave(wxCommandEvent &event);

	enum
	{
		IDC_PropertyGrid = wxID_HIGHEST + 1,
		IDC_TablesTree,
	};

	DECLARE_EVENT_TABLE()
};

#endif