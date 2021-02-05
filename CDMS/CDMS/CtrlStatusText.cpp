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

#include "CtrlStatusText.h"
#include <wx/tooltip.h>
#include "Construct.h"
#include "Common.h"

class CStatText : public wxEvtHandler
{
public:
	void Event(wxMouseEvent& e)
	{
		if(e.Entering())
		{
			TheConstruct->GetStatusBar()->PushStatusText(((wxWindow*)this)->GetToolTip()->GetTip());
		}
		else
		{
			TheConstruct->GetStatusBar()->PopStatusText();
		}
	}
};

void AddStatusbarText(wxWindow* pWnd, wxString sMsg)
{
	pWnd->SetToolTip(sMsg);
	pWnd->GetEventHandler()->Connect(wxEVT_ENTER_WINDOW, wxMouseEventHandler(CStatText::Event));
	pWnd->GetEventHandler()->Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(CStatText::Event));
}