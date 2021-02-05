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
#include "TabDialog.h"
#include "Common.h"

BEGIN_EVENT_TABLE(frmTabDialog, wxDialog)
	EVT_CLOSE(frmTabDialog::OnCloseWindow)
	EVT_SIZE(frmTabDialog::OnSize)
	EVT_AUINOTEBOOK_PAGE_CLOSE(-1, frmTabDialog::OnTabClose)
END_EVENT_TABLE()

frmTabDialog::frmTabDialog(const wxString& sTitle)
	:wxDialog(wxTheApp->GetTopWindow(), -1, sTitle, wxPoint(0, 0) , wxSize(620, 750), wxFRAME_FLOAT_ON_PARENT | wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxSYSTEM_MENU)
{
	CentreOnParent();
	wxBoxSizer *pTopSizer = new wxBoxSizer( wxVERTICAL );

	m_pTabs = 0;

	pTopSizer->Add(m_pTabs = new wxAuiNotebook(this, -1, wxPoint(0,0), wxSize(620, 750), (wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxAUI_NB_WINDOWLIST_BUTTON | wxNO_BORDER) & (~(wxAUI_NB_CLOSE_BUTTON | wxAUI_NB_CLOSE_ON_ACTIVE_TAB | wxAUI_NB_CLOSE_ON_ALL_TABS))) , 1, wxALL | wxEXPAND, 0);

	SetSizer(pTopSizer);
	pTopSizer->SetItemMinSize(m_pTabs, 620, 750);
	pTopSizer->SetSizeHints( this );
	pTopSizer->SetItemMinSize(m_pTabs, 100, 100);
}

void frmTabDialog::OnSize(wxSizeEvent& event)
{ UNUSED(event);
	Layout();
}

void frmTabDialog::OnTabClose(wxAuiNotebookEvent& event)
{ UNUSED(event);
	if(m_pTabs->GetPageCount() == 1) EndModal(wxID_OK);
}

void frmTabDialog::OnCloseWindow(wxCloseEvent& event)
{
	for(size_t i = 0; i < m_pTabs->GetPageCount(); ++i)
	{
		wxCloseEvent e2(wxEVT_CLOSE_WINDOW);
		e2.SetCanVeto(event.CanVeto());
		m_pTabs->GetPage(i)->GetEventHandler()->ProcessEvent(e2);
		if(e2.GetVeto())
		{
			event.Veto();
			return;
		}
	}
	this->Destroy();
}

wxAuiNotebook* frmTabDialog::GetTabs()
{
	return m_pTabs;
}