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

#ifndef _FRM_RGDMACRO_H_
#define _FRM_RGDMACRO_H_
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
#include <wx/stc/stc.h>
#include <wx/treectrl.h>

class frmRgdMacro : public wxDialog
{
protected:
	wxStyledTextCtrl* m_pSTC;
	wxTextCtrl *m_pTextbox;
	std::map<unsigned long, char*> m_mapToUpdate;

	wxStaticText* m_pCaption;
	wxButton *m_pSaveBtn, *m_pLoadBtn, *m_pRunBtn, *m_pModeBtn;
	wxBoxSizer *m_pFormMainSizer;

	bool m_bShowingOutput;

	unsigned long m_iPathHash;
	unsigned long m_iPathLen;

	bool m_bAllowDebug;
	bool m_bAllowIO;
	bool m_bAllowOS;
	bool m_bAllowSave;
	bool m_bAllowLoad;

	wxString m_sPath;
	wxTreeItemId& m_oFolder;

	static void _callback_print(void* pTag, const char* sMsg);
	static bool _callback_save(void* pTag, const char* sFile);
	static bool _callback_load(void* pTag, const char* sFile);
	static bool _callback_security(void* pTag, CRgdFileMacro::eSecurityTypes);
	void _populateFileList(std::vector<char*>* lstFiles);
	bool _request_Permission(wxString sAction);
public:
	frmRgdMacro(wxString sFile, wxTreeItemId& oFolder);

	void OnRunClick(wxCommandEvent& event);
	void OnModeClick(wxCommandEvent& event);
	void OnLoadClick(wxCommandEvent& event);
	void OnSaveClick(wxCommandEvent& event);
	void OnCancelClick(wxCommandEvent& event);
	void OnStyleNeeded(wxStyledTextEvent &event);
	void OnCharAdded(wxStyledTextEvent &event);

	enum
	{
		IDC_Run = wxID_HIGHEST + 1,
		IDC_Load,
		IDC_Save,
		IDC_Cancel,
		IDC_Mode,
	};

	DECLARE_EVENT_TABLE()
};

#endif