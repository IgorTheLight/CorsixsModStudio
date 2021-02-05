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
#include "frmMessage.h"
#include "resource.h"
#include "Common.h"

BEGIN_EVENT_TABLE(frmMessage, wxFrame)
	EVT_CLOSE(frmMessage::OnQuit)
	EVT_PAINT(frmMessage::OnPaint)
END_EVENT_TABLE()

frmMessage::frmMessage(const wxString& sImage, const wxString& sTitle)
	:wxFrame(wxTheApp->GetTopWindow(), -1, sTitle, wxPoint(0, 0) , wxSize(200, 132), wxFRAME_FLOAT_ON_PARENT | wxFRAME_TOOL_WINDOW)
{
	CentreOnParent();
	m_pImage = 0;
	m_pText = 0;

	m_pImage = new wxBitmap(sImage ,wxBITMAP_TYPE_BMP_RESOURCE);
	m_pText = new wxStaticText(this, -1, sTitle, wxPoint(0, 50), wxSize(200, 82), wxST_NO_AUTORESIZE | wxALIGN_CENTER);
	m_pText->Wrap(200);
	m_pText->SetBackgroundStyle(wxBG_STYLE_COLOUR);
	m_pText->SetBackgroundColour(wxColour(255, 255, 255));
	this->SetBackgroundColour(wxColour(255, 255, 255));
}

void frmMessage::OnQuit(wxCloseEvent& event)
{ UNUSED(event);
	delete m_pImage;
}

void frmMessage::OnPaint(wxPaintEvent& event)
{ UNUSED(event);
	wxPaintDC dc(this);
	wxMemoryDC temp_dc;
	temp_dc.SelectObject(*m_pImage);
	dc.Blit(76, 0, 48, 48, &temp_dc, 0, 0);
}

void frmMessage::SetMessage(wxString& sMsg)
{
	m_pText->SetLabel(sMsg);
}