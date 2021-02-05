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

#ifndef _FRM_UCSEDITOR_H_
#define _FRM_UCSEDITOR_H_
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

class frmUCSEditor : public wxWindow
{
protected:
	wxPropertyGrid* m_pPropertyGrid;
	CUcsTransaction* m_pUCS;
	bool m_bReadOnly;
	bool m_bNeedsSave;
	unsigned long* m_pResultVal;
	wxAuiNotebook* m_pTabStripForLoad;
	wxButton* m_pLoadButton;

public:
	frmUCSEditor(wxWindow* parent, wxWindowID id, bool bReadOnly, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, unsigned long* pResult = 0);
	~frmUCSEditor();

	/*!
		will throw a CRainmanException on error
	*/
	void FillFromCUcsFile(CUcsFile* pUcs, unsigned long iSelect = ULONG_MAX);

	void SetTabStripForLoad(wxAuiNotebook* pTabStrib);

	void OnSize(wxSizeEvent& event);
	void OnPropertyChange(wxPropertyGridEvent& event);
	void OnNewEntry(wxCommandEvent& event);

	void OnLoad(wxCommandEvent& event);
	void OnClose(wxCommandEvent& event);
	void OnApply(wxCommandEvent& event);

	void DoSave();
	void OnSaveFile(wxCommandEvent& event);

	void OnCloseWindow(wxCloseEvent& event);

	enum
	{
		IDC_PropertyGrid = wxID_HIGHEST + 1,
		IDC_ToolSave,
		IDC_ToolAdd,

		IDC_ToolApply,
		IDC_ToolClose,
	};

	DECLARE_EVENT_TABLE()
};

#endif