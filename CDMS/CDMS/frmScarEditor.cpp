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

#include "frmScarEditor.h"
#include "Construct.h"
#include "strconv.h"
#include "strings.h"
#include "Utility.h"
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}
#include <wx/toolbar.h>
#include <wx/tbarbase.h>
#include <algorithm>
#include "Common.h"

BEGIN_EVENT_TABLE(frmScarEditor, wxWindow)
	EVT_SIZE(frmScarEditor::OnSize)
	EVT_TOOL(IDC_ToolCheck, frmScarEditor::OnCheckLua)
	EVT_BUTTON(IDC_Compile, frmScarEditor::OnCheckLua)
	EVT_TOOL(IDC_ToolSave, frmScarEditor::OnSave)
	EVT_BUTTON(wxID_SAVE, frmScarEditor::OnSave)
	EVT_STC_CHARADDED(IDC_Text, frmScarEditor::OnCharAdded)
	EVT_STC_STYLENEEDED(IDC_Text, frmScarEditor::OnStyleNeeded)
	EVT_STC_USERLISTSELECTION(IDC_Text, frmScarEditor::OnAutoCompChoose)
	EVT_STC_SAVEPOINTLEFT(IDC_Text, frmScarEditor::OnSavePointLeave)
	EVT_STC_SAVEPOINTREACHED(IDC_Text, frmScarEditor::OnSavePointReach)
	EVT_CLOSE(frmScarEditor::OnCloseWindow)
	EVT_CHOICE(IDC_FunctionDrop, frmScarEditor::OnFuncListChoose)
END_EVENT_TABLE()

#define mySTC_STYLE_BOLD 1
#define mySTC_STYLE_ITALIC 2
#define mySTC_STYLE_UNDERL 4
#define mySTC_STYLE_HIDDEN 8

struct StyleInfo {
    wxChar *name;
    wxChar *foreground;
    wxChar *background;
    wxChar *fontname;
    int fontsize;
    int fontstyle;
    int lettercase;
};

const StyleInfo g_StylePrefs [] = {
// wxSTC_LUA_DEFAULT
{_T("Default"),
    _T("BLACK"), _T("WHITE"),
    _T(""), 10, 0, 0},

// wxSTC_LUA_COMMENT
{_T("Comment"),
    _T("FOREST GREEN"), _T("WHITE"),
    _T(""), 10, 0, 0},

// wxSTC_LUA_COMMENTLINE
{_T("Comment line"),
    _T("FOREST GREEN"), _T("WHITE"),
    _T(""), 10, 0, 0},

// wxSTC_LUA_COMMENTDOC
{_T("Comment (Doc)"),
    _T("FOREST GREEN"), _T("WHITE"),
    _T(""), 10, 0, 0},

// wxSTC_LUA_NUMBER
{_T("Number"),
    _T("BLACK"), _T("WHITE"),
    _T(""), 10, 0, 0},

// wxSTC_LUA_WORD
{_T("Keyword1"),
    _T("BLUE"), _T("WHITE"),
    _T(""), 10, mySTC_STYLE_BOLD, 0},

// wxSTC_LUA_STRING
{_T("String"),
    _T("PURPLE"), _T("WHITE"),
    _T(""), 10, 0, 0},

// wxSTC_LUA_CHARACTER
{_T("Character"),
    _T("GOLD"), _T("WHITE"),
    _T(""), 10, 0, 0},

// wxSTC_LUA_LITERALSTRING
{_T("Literal String"),
    _T("PURPLE"), _T("WHITE"),
    _T(""), 10, 0, 0},

// wxSTC_LUA_PREPROCESSOR (n/a)
{_T("Preprocessor"),
    _T("GREY"), _T("WHITE"),
    _T(""), 10, 0, 0},

// wxSTC_LUA_OPERATOR
{_T("Operator"),
    _T("RED"), _T("WHITE"),
    _T(""), 10, mySTC_STYLE_BOLD, 0},

// wxSTC_LUA_IDENTIFIER
{_T("Identifier"),
    _T("BLACK"), _T("WHITE"),
    _T(""), 10, 0, 0},

// wxSTC_LUA_STRINGEOL
{_T("String (EOL)"),
    _T("PURPLE"), _T("WHITE"),
    _T(""), 10, 0, 0},

// mySTC_TYPE_WORD2
{_T("Keyword2"),
    _T("MEDIUM BLUE"), _T("WHITE"),
    _T(""), 10, mySTC_STYLE_BOLD, 0},

// mySTC_TYPE_WORD3
{_T("Keyword3"),
    _T("TAN"), _T("WHITE"),
    _T(""), 10, 0, 0},

// mySTC_TYPE_WORD4
{_T("Keyword4"),
    _T("FIREBRICK"), _T("WHITE"),
    _T(""), 10, 0, 0},

// mySTC_TYPE_WORD5
{_T("Keyword5"),
    _T("DARK GREY"), _T("WHITE"),
    _T(""), 10, 0, 0},

// mySTC_TYPE_WORD6
{_T("Keyword6"),
    _T("GREY"), _T("WHITE"),
    _T(""), 10, 0, 0},

// mySTC_TYPE_WORD7
{_T("Keyword7"),
    _T("GREY"), _T("WHITE"),
    _T(""), 10, 0, 0},

// mySTC_TYPE_WORD8
{_T("Keyword8"),
    _T("GREY"), _T("WHITE"),
    _T(""), 10, 0, 0}
};

struct HightlightSet
{
	int type;
	const wxChar* words;
};

static HightlightSet g_LuaWords[] = {
	{wxSTC_LUA_DEFAULT, 0},
	{wxSTC_LUA_COMMENTLINE, 0},
	{wxSTC_LUA_COMMENTDOC, 0},
	{wxSTC_LUA_NUMBER, 0},
	{wxSTC_LUA_STRING, 0},
	{wxSTC_LUA_CHARACTER, 0},
	{wxSTC_LUA_LITERALSTRING, 0},
	{wxSTC_LUA_PREPROCESSOR, 0},
	{wxSTC_LUA_OPERATOR, 0},
	{wxSTC_LUA_IDENTIFIER, 0},
	{wxSTC_LUA_STRINGEOL, 0},
	{wxSTC_LUA_WORD, _T("true false nil") },
	{wxSTC_LUA_WORD2, _T("and break do else elseif end false for function if in local nil not or repeat ")
						_T("return then true until while _G") },
	// WORD3 - API Functions
	{wxSTC_LUA_WORD3, 0},
	// WORD4 - API Constants
	{wxSTC_LUA_WORD4, 0},
	// WORD5 to WORD8 unused
	{wxSTC_LUA_WORD5, 0},
	{wxSTC_LUA_WORD6, 0},
	{wxSTC_LUA_WORD7, 0},
	{wxSTC_LUA_WORD8, 0},
	{-1, 0}
};

void frmScarEditor::_RestorePreviousCalltip()
{
	if(m_stkCalltips.size())
	{
		if(m_pSTC->CallTipActive()) m_pSTC->CallTipCancel();
		m_oThisCalltip = m_stkCalltips.top();
		m_pSTC->CallTipShow(m_oThisCalltip.iPos, m_oThisCalltip.sTip);
		m_stkCalltips.pop();
	}
}

void frmScarEditor::OnSave(wxCommandEvent &event)
{ UNUSED(event);
	DoSave();
}

void frmScarEditor::DoSave()
{
	BackupFile(TheConstruct->GetModule(), m_sFilename);
	char* saNewFile = wxStringToAscii(m_sFilename);
	char* saDir = strdup(saNewFile), *pSlash;
	pSlash = strrchr(saDir, '\\');
	if(pSlash)
		*pSlash = 0;
	else
		*saDir = 0;
	IDirectoryTraverser::IIterator *pDir;
	try
	{
		pDir = TheConstruct->GetModule()->VIterate(saDir);
	}
	catch(CRainmanException *pE)
	{
		ErrorBoxE(pE);
		delete[] saNewFile;
		free(saDir);
		return;
	}
	free(saDir);

	if(!saNewFile)
	{
		ErrorBoxAS(err_memory);
		delete pDir;
		return;
	}

	IFileStore::IOutputStream* pStream;
	try
	{
		pStream = TheConstruct->GetModule()->VOpenOutputStream(saNewFile, true);
	}
	catch(CRainmanException *pE)
	{
		ErrorBoxE(pE);
		delete[] saNewFile;
		delete pDir;
		return;
	}

	TheConstruct->GetFilesList()->UpdateDirectoryChildren(m_oFileParent, pDir);
	delete pDir;
	delete[] saNewFile;

	wxString sContent = m_pSTC->GetText();
	char* saContent = wxStringToAscii(sContent);
	if(!saContent)
	{
		ErrorBoxAS(err_memory);
		delete pStream;
		return;
	}

	try
	{
		pStream->VWrite((unsigned long)sContent.Length(), 1, saContent);
	}
	catch(CRainmanException *pE)
	{
		RestoreBackupFile(TheConstruct->GetModule(), m_sFilename);
		delete[] saContent;
		delete pStream;
		ErrorBoxE(pE);
		return;
	}
	m_bNeedsSaving = false;
	m_pSTC->SetSavePoint();
	wxMessageBox(AppStr(scar_savegood),AppStr(scar_save),wxICON_INFORMATION,this);

	delete[] saContent;
	delete pStream;
}

void frmScarEditor::OnSavePointLeave(wxStyledTextEvent &event)
{ UNUSED(event);
	m_bNeedsSaving = true;
}

void frmScarEditor::OnSavePointReach(wxStyledTextEvent &event)
{ UNUSED(event);
	m_bNeedsSaving = false;
}

void frmScarEditor::OnCloseWindow(wxCloseEvent& event)
{
	if(m_bNeedsSaving)
	{
		wxMessageDialog* dialog = new wxMessageDialog(this,
		wxT("Text has been modified. Save file?"), m_sFilename, wxYES_NO|wxCANCEL);

		int ans = dialog->ShowModal();
		dialog->Destroy();

		switch (ans)
		{
		case wxID_YES:
			DoSave();
			break;

		case wxID_CANCEL:
		default:
			if (event.CanVeto()) event.Veto();
		case wxID_NO:
			break;
		}
	}
}

void frmScarEditor::OnAutoCompChoose(wxStyledTextEvent &event)
{ UNUSED(event);
	_RestorePreviousCalltip();
}

void frmScarEditor::_PushThisCalltip()
{
	if(m_pSTC->CallTipActive())
		m_stkCalltips.push(m_oThisCalltip);
	else
	{
		while(m_stkCalltips.size()) m_stkCalltips.pop();
	}
}

void frmScarEditor::OnCharAdded(wxStyledTextEvent &event)
{
    int iCurrentLine = m_pSTC->GetCurrentLine();
    if ((char)event.GetKey() == '\n')
	{
        int iLineIndentation = 0;
        if (iCurrentLine > 0)
		{
            iLineIndentation = m_pSTC->GetLineIndentation(iCurrentLine - 1);
        }
        if (iLineIndentation == 0) return;
        m_pSTC->SetLineIndentation(iCurrentLine, iLineIndentation);
		m_pSTC->LineEnd();
    }
	else if((char)event.GetKey() == '(')
	{
		_PushThisCalltip();
		int iPos = m_pSTC->GetCurrentPos() - 1;
		int iWordPos = m_pSTC->WordStartPosition(iPos, false);
		m_oThisCalltip.iPos = iWordPos;
		wxString sWord = m_pSTC->GetTextRange(iWordPos, iPos);
		char* saWord = wxStringToAscii(sWord);
		for(std::list<_ScarFunction>::iterator itr = m_lstScarFunctions.begin(); itr != m_lstScarFunctions.end(); ++itr)
		{
			if(itr->iType == 0 && strcmp(saWord, itr->sName) == 0)
			{
				m_oThisCalltip.sTip = wxT("");
				m_oThisCalltip.sTip.Append(AsciiTowxString(itr->sReturn));
				m_oThisCalltip.sTip.Append(wxT(" "));
				m_oThisCalltip.sTip.Append(AsciiTowxString(itr->sName));
				m_oThisCalltip.sTip.Append(wxT("("));
				bool bNeedComma = false;
				for(std::list<char*>::iterator itr2 = itr->sArguments.begin(); itr2 != itr->sArguments.end(); ++itr2)
				{
					if(bNeedComma) m_oThisCalltip.sTip.Append(wxT(",\n    "));
					m_oThisCalltip.sTip.Append(AsciiTowxString(*itr2));
					bNeedComma = true;
				}
				m_oThisCalltip.sTip.Append(wxT(")\n"));
				m_oThisCalltip.sTip.Append(AsciiTowxString(itr->sDesc));
				if(m_pSTC->CallTipActive()) m_pSTC->CallTipCancel();
				m_pSTC->CallTipSetBackground(wxColour(_T("LIGHT BLUE")));
				m_pSTC->CallTipSetForeground(wxColour(_T("BLACK")));
				m_pSTC->CallTipShow(iWordPos, m_oThisCalltip.sTip);
				delete[] saWord;
				return;
			}
		}
		if(m_pSTC->CallTipActive()) m_pSTC->CallTipCancel();
		m_pSTC->CallTipSetBackground(wxColour(_T("LIGHT BLUE")));
		m_pSTC->CallTipSetForeground(wxColour(_T("BLACK")));
		 m_oThisCalltip.sTip = wxT("No help available");
		m_pSTC->CallTipShow(iWordPos, m_oThisCalltip.sTip);
		delete[] saWord;
	}
	else if((char)event.GetKey() == '_')
	{
		if(!m_pSTC->AutoCompActive())
		{
			int iPos = m_pSTC->GetCurrentPos() - 1;
			int iWordPos = m_pSTC->WordStartPosition(iPos, false);
			wxString sWord = m_pSTC->GetTextRange(iWordPos, iPos);
			char* saWord = wxStringToAscii(sWord);
			wxString sItems;
			size_t iLen = 0, iWordLen = sWord.Len();
			bool GotWords = false;
			for(std::list<_ScarFunction>::iterator itr = m_lstScarFunctions.begin(); itr != m_lstScarFunctions.end(); ++itr) iLen += (strlen(itr->sName) + 1);
			sItems.Alloc(iLen);
			for(std::list<_ScarFunction>::iterator itr = m_lstScarFunctions.begin(); itr != m_lstScarFunctions.end(); ++itr)
			{
				if(strncmp(saWord, itr->sName, iWordLen) == 0)
				{
					if(GotWords) sItems.Append(' ');
					sItems.Append(AsciiTowxString(itr->sName));
					GotWords = true;
				}
			}
			delete[] saWord;
			m_pSTC->AutoCompSetAutoHide(true);
			if(GotWords)
			{
				_PushThisCalltip();
				m_pSTC->AutoCompShow(iPos - iWordPos + 1, sItems);
			}
		}
	}
	else if((char)event.GetKey() == ')')
	{
		m_pSTC->CallTipCancel();
		_RestorePreviousCalltip();
	}
}

void frmScarEditor::Load(IFileStore::IStream* pFile)
{
	long iLength;
	try
	{
		pFile->VSeek(0, IFileStore::IStream::SL_End);
		iLength = pFile->VTell();
		pFile->VSeek(0, IFileStore::IStream::SL_Root);
	}
	catch(CRainmanException *pE)
	{
		ErrorBoxE(pE);
		return;
	}

	unsigned char *sBuffer = new unsigned char[iLength];
	try
	{
		pFile->VRead(iLength, 1, sBuffer);
	}
	catch(CRainmanException *pE)
	{
		ErrorBoxE(pE);
		delete[] sBuffer;
		return;
	}
	wchar_t *pBuffer = new wchar_t[iLength + 1];
	for(long i = 0; i < iLength; ++i) pBuffer[i] = sBuffer[i];
	pBuffer[iLength] = 0;
	m_pSTC->AddText(pBuffer);
	delete[] pBuffer;
	delete[] sBuffer;
	m_bNeedsSaving = false;
	m_pSTC->EmptyUndoBuffer();
	m_pSTC->SetSavePoint();

	m_pFunctionDropdown->Clear();
	_FillFunctionDrop(wxString());
}

void frmScarEditor::OnStyleNeeded(wxStyledTextEvent &event)
{
	int iEndStyle = m_pSTC->GetEndStyled();
	m_pSTC->Colourise(iEndStyle, event.GetPosition());
}

const char* lua_tolstring(lua_State *L, int index, size_t *len)
{
	if(len) *len = lua_strlen(L, index);
	return lua_tostring(L, index);
}

template <class T> static inline void loadLuaFn(T& oP, wxString sN) {oP = (T)GetConstruct()->oLua512Library.GetSymbol(sN);}

void frmScarEditor::OnCheckLua(wxCommandEvent &event)
{ UNUSED(event);
	lua_State* (*pfn_lua_open)(void) = lua_open;
	int (*pfn_luaL_loadbuffer)(lua_State*,const char*,size_t,const char*) = luaL_loadbuffer;
	const char* (*pfn_lua_tolstring)(lua_State*,int,size_t*) = lua_tolstring;
	void (*pfn_lua_close)(lua_State*) = lua_close;

	if(GetConstruct()->GetModule()->GetModuleType() != CModuleFile::MT_DawnOfWar)
	{
		if(GetConstruct()->oLua512Library.IsLoaded())
		{
			loadLuaFn(pfn_lua_open, wxT("luaL_newstate"));
			loadLuaFn(pfn_luaL_loadbuffer, wxT("luaL_loadbuffer"));
			loadLuaFn(pfn_lua_tolstring, wxT("lua_tolstring"));
			loadLuaFn(pfn_lua_close, wxT("lua_close"));
		}
	}

	char *sLua = wxStringToAscii(m_pSTC->GetText());

	lua_State *L;
	L = pfn_lua_open();
	int iLuaError = pfn_luaL_loadbuffer(L, sLua, strlen(sLua), "");
	delete[] sLua;
	if(iLuaError)
	{
		const char* sErr = pfn_lua_tolstring(L, -1, 0);
		sErr = strchr(sErr, ':');
		++sErr;
		unsigned long iLine = 0;
		while(*sErr != ':')
		{
			if((*sErr >= '0') && (*sErr <= '9'))
			{
				iLine *= 10;
				iLine += (*sErr - '0');
			}
			++sErr;
		}
		++sErr;
		wxString sError;
		sError.Printf(AppStr(scar_bad), iLine, sErr);
		wxMessageBox(sError,AppStr(scar_checklua),wxICON_ERROR,this);
	}
	else
	{
		wxMessageBox(AppStr(scar_luagood),AppStr(scar_checklua),wxICON_INFORMATION,this);
	}

	pfn_lua_close(L);

	m_pFunctionDropdown->Clear();
	_FillFunctionDrop(wxString());
}

char* frmScarEditor::_ReadNiceString(FILE* f)
{
	size_t iLen;
	fread(&iLen, sizeof(size_t), 1, f);
	char* s = new char[iLen + 1];
	fread(s, iLen, 1, f);
	s[iLen] = 0;
	return s;
}

bool is_a_lt_b_function_drop(std::pair<wxString,void*>* a, std::pair<wxString,void*>* b)
{
	return a->first < b->first;
}

void frmScarEditor::OnFuncListChoose(wxCommandEvent &event)
{
	if(event.GetSelection())
	{
		m_pFunctionDropdown->Clear();
		int iPosNew = _FillFunctionDrop(event.GetString());
		if(iPosNew != -1)
		{
			m_pSTC->GotoPos(iPosNew);
			m_pSTC->Home();
			m_pSTC->LineEndExtend();
			m_pSTC->SetFocus();
		}
	}
}

int frmScarEditor::_FillFunctionDrop(wxString sNameTarget)
{
	int iRet = -1;
	m_pFunctionDropdown->Append(AppStr(scar_funcdrop), (void*)0);
	m_pFunctionDropdown->SetSelection(0);

	std::vector<std::pair<wxString,void*>*> oList;

	wxString sContents = m_pSTC->GetText();
	const wchar_t * pText = sContents.c_str(), *pRoot;
	pRoot = pText;
	pText = wcsstr(pText, L"function");
	while(pText)
	{
		pText += 8;
		while( (*pText == ' ') || (*pText == '\t') || (*pText == '\r') || (*pText == '\n') ) ++pText;
		const wchar_t *pEnd = wcschr(pText, '('), *pTest;
		if(pEnd)
		{
			--pEnd;
			while( (*pEnd == ' ') || (*pEnd == '\t') || (*pEnd == '\r') || (*pEnd == '\n') ) --pEnd;

			if(pEnd > pText)
			{
				for(pTest = pText; pTest <= pEnd; ++pTest)
				{
					if(*pTest == ' ' || *pTest == '\r' || *pTest == '\n' || *pTest == '-') goto not_a_function_def;
				}
				oList.push_back(new std::pair<wxString,void*>(wxString(pText, 1 + (size_t)(pEnd - pText)), (void*)(pText - pRoot)));
			}
		}
		not_a_function_def:
		pText = wcsstr(pText, L"function");
	}
	std::sort(oList.begin(), oList.end(), is_a_lt_b_function_drop);
	for(std::vector<std::pair<wxString,void*>*>::iterator itr = oList.begin(); itr != oList.end(); ++itr)
	{
		if(iRet == -1)
		{
			if(sNameTarget.IsSameAs((**itr).first, false))
				iRet = (int)(**itr).second;
		}
		m_pFunctionDropdown->Append((**itr).first, (**itr).second);
		delete *itr;
	}
	return iRet;
}

frmScarEditor::frmScarEditor(wxTreeItemId& oFileParent, wxString sFilename, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, const wchar_t* pLangRef)
	: m_oFileParent(oFileParent), m_sFilename(sFilename), wxWindow(parent, id, pos, size), m_bNeedsSaving(false)
{
	// Load ref
	if(pLangRef)
	{
		FILE *f;
		if(GetConstruct()->GetModule()->GetModuleType() == CModuleFile::MT_DawnOfWar)
			f = _wfopen(AppStr(app_scarreffile), L"rb");
		else
			f = _wfopen(AppStr(app_cohscarreffile), L"rb");

		size_t iFnCount;
		fread(&iFnCount, sizeof(size_t), 1, f);
		for(size_t i = 0; i < iFnCount; ++i)
		{
			_ScarFunction oFunction;
			oFunction.sReturn = _ReadNiceString(f);
			oFunction.sName = _ReadNiceString(f);
			oFunction.iType = 0;
			if(*oFunction.sName == 0)
			{
				oFunction.iType = 1;
				delete[] oFunction.sName;
				oFunction.sName = oFunction.sReturn;
				oFunction.sReturn = 0;
				oFunction.sDesc = 0;
			}
			else
			{
				size_t iArgCount;
				fread(&iArgCount, sizeof(size_t), 1, f);
				for(size_t j = 0; j < iArgCount; ++j)
				{
					oFunction.sArguments.push_back(_ReadNiceString(f));
				}
				oFunction.sDesc = _ReadNiceString(f);
			}
			m_lstScarFunctions.push_back(oFunction);
		}
		fclose(f);
	}

	// Gui
	wxBoxSizer *pTopSizer = new wxBoxSizer( wxVERTICAL );

	wxToolBar *pToolbar;
	pTopSizer->Add(pToolbar = new wxToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_HORIZONTAL | wxNO_BORDER), 0, wxEXPAND | wxALL, 3);

	wxArrayString lstFuncDrop;
	pTopSizer->Add(m_pFunctionDropdown = new wxChoice(this, IDC_FunctionDrop, wxDefaultPosition, wxDefaultSize, lstFuncDrop), 0, wxEXPAND | wxALL, 3);

	wxBitmap oSaveBmp(wxT("IDB_32SAVE") ,wxBITMAP_TYPE_BMP_RESOURCE), oCheckBmp(wxT("IDB_32CHECK") ,wxBITMAP_TYPE_BMP_RESOURCE);
	oSaveBmp.SetMask(new wxMask(oSaveBmp, wxColour(128, 128, 128)));
	oCheckBmp.SetMask(new wxMask(oCheckBmp, wxColour(128, 128, 128)));
	pToolbar->SetToolBitmapSize(wxSize(32,32));
	pToolbar->AddTool(IDC_ToolSave, AppStr(scar_save), oSaveBmp, AppStr(scar_save));
	pToolbar->AddTool(IDC_ToolCheck, AppStr(scar_checklua), oCheckBmp, AppStr(scar_checklua));
	pToolbar->Realize();

	pTopSizer->Add(m_pSTC = new wxStyledTextCtrl(this, IDC_Text), 1, wxEXPAND | wxALL, 0);
	_FillFunctionDrop(wxString());

	m_pSTC->StyleClearAll();
	// default font for all styles
	m_pSTC->SetTabWidth(4);
	m_pSTC->SetLexer (wxSTC_LEX_LUA);
	m_pSTC->SetProperty( wxT("fold.compact"), wxT("0") );
    m_pSTC->SetViewEOL (false);
    m_pSTC->SetIndentationGuides (true);
    m_pSTC->SetEdgeMode (wxSTC_EDGE_NONE);
    m_pSTC->SetViewWhiteSpace (wxSTC_WS_INVISIBLE);
    m_pSTC->SetOvertype (false);
    m_pSTC->SetReadOnly (false);
    m_pSTC->SetWrapMode (wxSTC_WRAP_NONE);
    wxFont font (10, wxMODERN, wxNORMAL, wxNORMAL);
    m_pSTC->StyleSetFont (wxSTC_STYLE_DEFAULT, font);
    m_pSTC->StyleSetBackground (wxSTC_STYLE_DEFAULT, *wxWHITE);
    m_pSTC->StyleSetForeground (wxSTC_STYLE_LINENUMBER, wxColour (_T("DARK GREY")));
    m_pSTC->StyleSetBackground (wxSTC_STYLE_LINENUMBER, wxColour(_T("WHEAT")));
    m_pSTC->SetUseTabs (true);
    m_pSTC->SetTabIndents (true);
    m_pSTC->SetBackSpaceUnIndents (true);
    m_pSTC->SetIndent (4);

	// default fonts for all styles!
    for (int i = 0; i < wxSTC_STYLE_LASTPREDEFINED; ++i) {
        wxFont font (10, wxMODERN, wxNORMAL, wxNORMAL);
        m_pSTC->StyleSetFont (i, font);
    }

    // set common styles
    m_pSTC->StyleSetForeground (wxSTC_STYLE_DEFAULT, wxColour (_T("DARK GREY")));
    m_pSTC->StyleSetForeground (wxSTC_STYLE_INDENTGUIDE, wxColour (_T("DARK GREY")));

	int iWordSet = 0;
    for (int i = 0; g_LuaWords[i].type != -1; ++i) {
        const StyleInfo &curType = g_StylePrefs [g_LuaWords[i].type];
        wxFont font (curType.fontsize, wxMODERN, wxNORMAL, wxNORMAL, false, curType.fontname);
        m_pSTC->StyleSetFont (g_LuaWords[i].type, font);
        if (curType.foreground) m_pSTC->StyleSetForeground (g_LuaWords[i].type, wxColour (curType.foreground));
        if (curType.background) m_pSTC->StyleSetBackground (g_LuaWords[i].type, wxColour (curType.background));
        m_pSTC->StyleSetBold (g_LuaWords[i].type, (curType.fontstyle & mySTC_STYLE_BOLD) > 0);
        m_pSTC->StyleSetItalic (g_LuaWords[i].type, (curType.fontstyle & mySTC_STYLE_ITALIC) > 0);
        m_pSTC->StyleSetUnderline (g_LuaWords[i].type, (curType.fontstyle & mySTC_STYLE_UNDERL) > 0);
        m_pSTC->StyleSetVisible (g_LuaWords[i].type, (curType.fontstyle & mySTC_STYLE_HIDDEN) == 0);
        m_pSTC->StyleSetCase (g_LuaWords[i].type, curType.lettercase);
        if(g_LuaWords[i].words)
			m_pSTC->SetKeyWords (iWordSet++, g_LuaWords[i].words);
    }

	wxString sScarFns, sScarConstants;
	size_t iLenFns = 0, iLenCons = 0;
	for(std::list<_ScarFunction>::iterator itr = m_lstScarFunctions.begin(); itr != m_lstScarFunctions.end(); ++itr)
	{
		if(itr->iType == 0) iLenFns += (strlen(itr->sName) + 1);
		else iLenCons += (strlen(itr->sName) + 1);
	}
	sScarFns.Alloc(iLenFns);
	sScarConstants.Alloc(iLenCons);
	for(std::list<_ScarFunction>::iterator itr = m_lstScarFunctions.begin(); itr != m_lstScarFunctions.end(); ++itr)
	{
		if(itr->iType == 0)
		{
			sScarFns.Append(AsciiTowxString(itr->sName));
			sScarFns.Append(' ');
		}
		else
		{
			sScarConstants.Append(AsciiTowxString(itr->sName));
			sScarConstants.Append(' ');
		}
	}
	m_pSTC->SetKeyWords (iWordSet, sScarFns);
	m_pSTC->SetKeyWords (++iWordSet, sScarConstants);

	// misc. STC stuff
	m_pSTC->SetMarginWidth(0, m_pSTC->TextWidth(wxSTC_STYLE_LINENUMBER, _T("_999999")));

	// Buttons
	wxBoxSizer *pButtonSizer = new wxBoxSizer( wxHORIZONTAL );
	wxWindow *pBgTemp;

	pButtonSizer->Add(pBgTemp = new wxButton(this, IDC_Compile, AppStr(scar_checklua)), 0, wxEXPAND | wxALL, 3);
	pButtonSizer->Add(new wxButton(this, wxID_SAVE, AppStr(scar_save)), 0, wxEXPAND | wxALL, 3);

	pTopSizer->Add(pButtonSizer, 0, wxALIGN_RIGHT);

	SetBackgroundColour(pBgTemp->GetBackgroundColour());

	SetSizer(pTopSizer);
	pTopSizer->SetSizeHints( this );
}

frmScarEditor::~frmScarEditor()
{
	// Unload Ref
	for(std::list<_ScarFunction>::iterator itr = m_lstScarFunctions.begin(); itr != m_lstScarFunctions.end(); ++itr)
	{
		delete[] itr->sDesc;
		delete[] itr->sName;
		delete[] itr->sReturn;
		for(std::list<char*>::iterator itr2 = itr->sArguments.begin(); itr2 != itr->sArguments.end(); ++itr2)
		{
			delete[] *itr2;
		}
	}
}

void frmScarEditor::OnSize(wxSizeEvent& event)
{ UNUSED(event);
	Layout();
}