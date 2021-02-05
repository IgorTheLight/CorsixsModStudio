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
#include "frmUCSOutRange.h"
#include "strings.h"
#include "config.h"
#include "Common.h"

BEGIN_EVENT_TABLE(frmUCSOutOfRange, wxDialog)
	EVT_SIZE(frmUCSOutOfRange::OnSize)
	EVT_BUTTON(wxID_YES, frmUCSOutOfRange::OnYesClick)
	EVT_BUTTON(wxID_NO, frmUCSOutOfRange::OnNoClick)
END_EVENT_TABLE()

frmUCSOutOfRange::frmUCSOutOfRange(const wxString& sTitle, unsigned long iID)
	:wxDialog(wxTheApp->GetTopWindow(), -1, sTitle, wxPoint(0, 0) , wxSize(320, 480), wxFRAME_FLOAT_ON_PARENT | wxFRAME_TOOL_WINDOW | wxCAPTION)
{
	CentreOnParent();
	wxBoxSizer *pTopSizer = new wxBoxSizer( wxVERTICAL );

	wxString sCaption;
	sCaption.Printf(AppStr(ucsrange_caption), iID);

	wxWindow *pBgTemp;
	pTopSizer->Add(pBgTemp = new wxStaticText(this, -1, sCaption), 0, wxALIGN_LEFT | wxALL, 3);

	wxBoxSizer *pButtonSizer = new wxBoxSizer( wxHORIZONTAL );

	pButtonSizer->Add(new wxButton(this, wxID_YES, AppStr(ucsrange_yes)), 0, wxEXPAND | wxALL, 3);
	pButtonSizer->Add(new wxButton(this, wxID_NO, AppStr(ucsrange_no)), 0, wxEXPAND | wxALL, 3);

	pTopSizer->Add(pButtonSizer, 0, wxALIGN_LEFT);
	pTopSizer->Add(m_pCheckbox = new wxCheckBox(this, -1, AppStr(ucsrange_remember)), 0, wxALIGN_LEFT | wxALL, 3);

	bool bTicked;
	TheConfig->Read(AppStr(config_mod_ucsrangeremember), &bTicked, false);
	m_pCheckbox->SetValue(bTicked);

	SetSizer(pTopSizer);
	pTopSizer->SetSizeHints( this );
	SetBackgroundColour(pBgTemp->GetBackgroundColour());
}

void frmUCSOutOfRange::OnSize(wxSizeEvent& event)
{ UNUSED(event);
	Layout();
}

void frmUCSOutOfRange::OnYesClick(wxCommandEvent& event)
{ UNUSED(event);
	TheConfig->Write(AppStr(config_mod_ucsrangeremember), m_pCheckbox->GetValue());
	EndModal(wxID_YES);
}

void frmUCSOutOfRange::OnNoClick(wxCommandEvent& event)
{ UNUSED(event);
	TheConfig->Write(AppStr(config_mod_ucsrangeremember), m_pCheckbox->GetValue());
	EndModal(wxID_NO);
}