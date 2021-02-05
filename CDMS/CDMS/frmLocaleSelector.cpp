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

#include "frmLocaleSelector.h"
#include "strings.h"
#include "config.h"
#include "Common.h"

BEGIN_EVENT_TABLE(frmLocaleSelector, wxDialog)
	EVT_SIZE(frmLocaleSelector::OnSize)
	EVT_BUTTON(wxID_OPEN, frmLocaleSelector::OnNewClick)
END_EVENT_TABLE()

frmLocaleSelector::frmLocaleSelector(const wxString& sTitle, ConstructFrame::eLoadModGames eGame)
	:wxDialog(wxTheApp->GetTopWindow(), -1, sTitle, wxPoint(0, 0) , wxSize(320, 480), wxFRAME_FLOAT_ON_PARENT | wxFRAME_TOOL_WINDOW | wxCAPTION)
{
	CentreOnParent();
	wxBoxSizer *pTopSizer = new wxBoxSizer( wxVERTICAL );

	wxArrayString aLocales;
	switch(eGame)
	{
	default:
	case ConstructFrame::LM_Any:
	case ConstructFrame::LM_DoW_WA:
	case ConstructFrame::LM_DC:
	case ConstructFrame::LM_SS:
		aLocales.Add(wxT("Chinese"));
		aLocales.Add(wxT("Czech"));
		aLocales.Add(wxT("English"));
		aLocales.Add(wxT("English_Chinese"));
		aLocales.Add(wxT("French"));
		aLocales.Add(wxT("German"));
		aLocales.Add(wxT("Italian"));
		aLocales.Add(wxT("Japanese"));
		aLocales.Add(wxT("Korean"));
		aLocales.Add(wxT("Korean adult"));
		aLocales.Add(wxT("Polish"));
		aLocales.Add(wxT("Russian"));
		aLocales.Add(wxT("Slovak"));
		aLocales.Add(wxT("Spanish"));
		break;

	case ConstructFrame::LM_CoH_OF:
		aLocales.Add(wxT("Chineseenglish"));
		aLocales.Add(wxT("Chinesetrad"));
		aLocales.Add(wxT("Chinesesimp"));
		aLocales.Add(wxT("Czech"));
		aLocales.Add(wxT("English"));
		aLocales.Add(wxT("French"));
		aLocales.Add(wxT("German"));
		aLocales.Add(wxT("Italian"));
		aLocales.Add(wxT("Japanese"));
		aLocales.Add(wxT("Korean"));
		aLocales.Add(wxT("Polish"));
		aLocales.Add(wxT("Russian"));
		aLocales.Add(wxT("Spanish"));
		break;
	};

	wxWindow *pBgTemp;
	pTopSizer->Add(pBgTemp = new wxStaticText(this, -1, AppStr(localeselect_caption)), 0, wxALIGN_LEFT | wxALL, 3);
	pTopSizer->Add(m_pList = new wxListBox(this, IDC_LocaleList, wxDefaultPosition, wxDefaultSize, aLocales), 1, wxALL | wxEXPAND, 3);
	pTopSizer->Add(m_pCheckbox = new wxCheckBox(this, IDC_Remember, AppStr(localeselect_remember)), 0, wxALIGN_LEFT | wxALL, 3);

	wxBoxSizer *pButtonSizer = new wxBoxSizer( wxHORIZONTAL );

	pButtonSizer->Add(new wxButton(this, wxID_OPEN, AppStr(localeselect_use)), 0, wxEXPAND | wxALL, 3);

	pTopSizer->Add(pButtonSizer, 0, wxALIGN_RIGHT);

	m_pList->SetStringSelection(TheConfig->Read(AppStr(config_mod_locale), AppStr(localeselect_default)));

	bool bTicked;
	TheConfig->Read(AppStr(config_mod_localeremember), &bTicked, false);
	m_pCheckbox->SetValue(bTicked);

	SetSizer(pTopSizer);
	pTopSizer->SetSizeHints( this );
	SetBackgroundColour(pBgTemp->GetBackgroundColour());
}

void frmLocaleSelector::OnSize(wxSizeEvent& event)
{  UNUSED(event);
	Layout();
}

void frmLocaleSelector::OnNewClick(wxCommandEvent& event)
{  UNUSED(event);
	wxString sSel = m_pList->GetStringSelection();
	if(!sSel.IsEmpty())
	{
		TheConfig->Write(AppStr(config_mod_locale), sSel);
		TheConfig->Write(AppStr(config_mod_localeremember), m_pCheckbox->GetValue());
		EndModal(wxID_OK);
	}
}