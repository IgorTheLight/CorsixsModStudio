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

#ifndef _FRM_NEWMOD_H_
#define _FRM_NEWMOD_H_
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

class frmNewMod : public wxDialog
{
protected:
	wxTextCtrl* m_pName;
	wxChoice* m_pList;
	wxStaticText* m_pCreation;
	wxString m_sDoWPath, m_sCoHPath, m_sDCPath, m_sSSPath;
	wxString _UpdatePath(wxString sName);

	void _MakeCOH(char* sNiceName, char* sDirectoryFullPath, char* sDirectoryName, FILE* fModule);
	void _MakeCOH_Language(char* sToc, char* sName1, char* sName2, char* sDirectoryName, FILE* fModule);
public:
	/*!
		may throw a CRainmanException
	*/
	frmNewMod();

	void OnNewClick(wxCommandEvent& event);
	void OnCancelClick(wxCommandEvent& event);
	wxString GetPath();

	void OnGameChange(wxCommandEvent& event);
	void OnBrowseClick(wxCommandEvent& event);

	enum
	{
		IDC_Name = wxID_HIGHEST + 1,
		IDC_New,
		IDC_Cancel,
		IDC_Game,
		IDC_Browse
	};

	DECLARE_EVENT_TABLE()
};

#endif