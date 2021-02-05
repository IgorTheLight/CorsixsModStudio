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

#ifndef _FRM_SGAMAKE_H_
#define _FRM_SGAMAKE_H_
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

class frmSgaMake : public wxDialog
{
protected:
	wxTextCtrl *m_pInDir;
	wxComboBox *m_pOutFile, *m_pTocName;
public:
	frmSgaMake();

	void OnBrowseInClick(wxCommandEvent& event);
	void OnBrowseOutClick(wxCommandEvent& event);
	void OnGoClick(wxCommandEvent& event);
	void OnCancelClick(wxCommandEvent& event);
	void OnFileOutUpdated(wxCommandEvent& event);

	enum
	{
		IDC_DirIn = wxID_HIGHEST + 1,
		IDC_BrowseIn,
		IDC_FileOut,
		IDC_BrowseOut,
		IDC_TocName,
		IDC_Go,
		IDC_Cancel
	};

	DECLARE_EVENT_TABLE()
};

#endif