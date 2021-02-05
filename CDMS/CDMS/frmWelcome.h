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

#ifndef _FRM_WELCOME_H_
#define _FRM_WELCOME_H_
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

class frmWelcome : public wxWindow
{
protected:
	wxBitmap *m_pDonateBitmap;
	bool bFirstMouseEvent;

public:
	frmWelcome(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	~frmWelcome();

	void OnSize(wxSizeEvent& event);
	void OnQuit(wxCommandEvent& event);
	void OnLoadSga(wxCommandEvent& event);
	void OnLoadMod(wxCommandEvent& event);
	void OnLoadModDC(wxCommandEvent& event);
	void OnLoadModSS(wxCommandEvent& event);
	void OnLoadModCoH(wxCommandEvent& event);
	void OnNewMod(wxCommandEvent& event);
	void OnMouseEvent(wxMouseEvent& event);

	DECLARE_EVENT_TABLE()
};

enum
{
	// File Menu
	IDC_NewMod = wxID_HIGHEST + 1,
	IDC_LoadMod,
	IDC_LoadSga,
	IDC_LoadModDC,
	IDC_LoadModSS,
	IDC_LoadModCoH,
	IDC_Help,
	IDC_Quit,
	IDC_Donate
};

#endif