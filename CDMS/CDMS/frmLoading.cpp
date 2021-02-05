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

#include "frmLoading.h"
#include "resource.h"
#include "Common.h"

BEGIN_EVENT_TABLE(frmLoading, wxFrame)
	EVT_CLOSE(frmLoading::OnQuit)
	EVT_PAINT(frmLoading::OnPaint)
END_EVENT_TABLE()

frmLoading::frmLoading(const wxString& sTitle)
	:wxFrame(wxTheApp->GetTopWindow(), -1, sTitle, wxPoint(0, 0) , wxSize(384, 384), wxFRAME_FLOAT_ON_PARENT | wxFRAME_TOOL_WINDOW)
{
	CentreOnParent();
	m_pLoadingImage = 0;
	m_pText = 0;

	m_pLoadingImage = new wxBitmap(wxT("RIDB_LOADING") ,wxBITMAP_TYPE_BMP_RESOURCE);
	m_pText = new wxStaticText(this, -1, sTitle, wxPoint(0, 317), wxSize(384, 33), wxST_NO_AUTORESIZE | wxALIGN_CENTER);
	m_pText->Wrap(380);
	m_pText->SetBackgroundStyle(wxBG_STYLE_COLOUR);
	m_pText->SetBackgroundColour(wxColour(255, 255, 255));
	wxFont f = m_pText->GetFont();
	f.SetWeight(wxFONTWEIGHT_BOLD);
	m_pText->SetFont(f);
}

void frmLoading::OnQuit(wxCloseEvent& event)
{  UNUSED(event);
	delete m_pLoadingImage;
}

void frmLoading::OnPaint(wxPaintEvent& event)
{  UNUSED(event);
	wxPaintDC dc(this);
	wxMemoryDC temp_dc;
	temp_dc.SelectObject(*m_pLoadingImage);
	dc.Blit(0, 0, 384, 384, &temp_dc, 0, 0);
}

void frmLoading::SetMessage(wxString& sMsg)
{
	m_pText->SetLabel(sMsg);
}