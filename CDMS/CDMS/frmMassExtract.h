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

#ifndef _FRM_MASSEXTRACT_H_
#define _FRM_MASSEXTRACT_H_
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
#include <wx/treectrl.h>

class frmMassExtract : public wxDialog
{
protected:
	struct _tSrc
	{
		_tSrc(const char* sM, const char* sS);
		const char* sMod;
		const char* sSrc;
	};
	std::vector<_tSrc> m_vSrcs, m_vActiveSrcs;

	wxCheckListBox *m_pCheckList;
	wxGauge* m_pGauge;
	wxStaticText* m_pCaption;
	wxButton* m_pSelectAllBtn, *m_pAdvancedBtn;
	wxString m_sPath;
	wxTreeItemId& m_oFolder;

	char* m_p4mbBuffer;
	bool m_bForceUpdate;
	bool m_bAdvancedVisible;

	size_t _DoExtract(wxTreeCtrl* pTree, wxTreeItemId& oFolder, wxString sPath, CModuleFile* pModule, bool bCountOnly, size_t iCountBase, size_t iCountDiv);

	void _FillCheckList(CModuleFile* pMod, bool bIsRoot, wxArrayString& sList, std::vector<_tSrc>& vList);
public:
	frmMassExtract(wxString sFile, wxTreeItemId& oFolder, bool bForceUpdate = false);

	void OnGoClick(wxCommandEvent& event);
	void OnSelectClick(wxCommandEvent& event);
	void OnCancelClick(wxCommandEvent& event);
	void OnAdvancedClick(wxCommandEvent& event);

	enum
	{
		IDC_Go = wxID_HIGHEST + 1,
		IDC_Cancel,
		IDC_Advanced,
		IDC_SelectAll
	};

	DECLARE_EVENT_TABLE()
};

#endif