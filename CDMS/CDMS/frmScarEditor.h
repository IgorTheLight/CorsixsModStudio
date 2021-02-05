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

#ifndef _FRM_SCAREDITOR_H_
#define _FRM_SCAREDITOR_H_
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
#include <list>
#include <stack>
#include <wx/treectrl.h>
#include "strings.h"

class frmScarEditor : public wxWindow
{
protected:
	wxStyledTextCtrl* m_pSTC;
	wxString m_sFilename;
	wxChoice* m_pFunctionDropdown;
	wxTreeItemId m_oFileParent;
	bool m_bNeedsSaving;

	struct _ScarFunction
	{
		char* sReturn;
		char* sName;
		std::list<char*> sArguments;
		char* sDesc;
		int iType;
	};

	struct _CCalltipPop
	{
		int iPos;
		wxString sTip;
	};

	_CCalltipPop m_oThisCalltip;
	std::stack<_CCalltipPop> m_stkCalltips;
	std::list<_ScarFunction> m_lstScarFunctions;
	static char* _ReadNiceString(FILE* f);
	void _RestorePreviousCalltip();
	void _PushThisCalltip();
	int _FillFunctionDrop(wxString sNameTarget);

public:
	frmScarEditor(wxTreeItemId& oFileParent, wxString sFilename, wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, const wchar_t* pLangRef = AppStr(app_scarreffile));
	~frmScarEditor();

	void OnSize(wxSizeEvent& event);

	void OnCharAdded(wxStyledTextEvent &event);
	void OnStyleNeeded(wxStyledTextEvent &event);
	void OnSavePointLeave(wxStyledTextEvent &event);
	void OnSavePointReach(wxStyledTextEvent &event);

	void DoSave();
	void OnSave(wxCommandEvent &event);
	void OnCheckLua(wxCommandEvent &event);
	void OnAutoCompChoose(wxStyledTextEvent &event);
	void OnFuncListChoose(wxCommandEvent &event);

	void Load(IFileStore::IStream* pFile);

	void OnCloseWindow(wxCloseEvent& event);

	enum
	{
		IDC_Text = wxID_HIGHEST + 1,
		IDC_Compile,
		IDC_ToolSave,
		IDC_ToolCheck,
		IDC_FunctionDrop
	};

	DECLARE_EVENT_TABLE()
};

#endif