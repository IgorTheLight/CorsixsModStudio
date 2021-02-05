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
#include "frmUCSEditor.h"
#include "frmUCSOutRange.h"
#include "Construct.h"
#include "Tools.h"
#include "frmUCSEditor.h"
#include "strings.h"
#include "config.h"
#include "Utility.h"
#include <wx/msgdlg.h>
#include <wx/toolbar.h>
#include <wx/tbarbase.h>
#include <memory>
#include "Common.h"

BEGIN_EVENT_TABLE(frmUCSEditor, wxWindow)
	EVT_SIZE(frmUCSEditor::OnSize)
	EVT_PG_CHANGED(IDC_PropertyGrid, frmUCSEditor::OnPropertyChange)
	EVT_BUTTON(wxID_NEW, frmUCSEditor::OnNewEntry)
	EVT_TOOL(IDC_ToolAdd, frmUCSEditor::OnNewEntry)
	EVT_BUTTON(wxID_SAVE, frmUCSEditor::OnSaveFile)
	EVT_TOOL(IDC_ToolSave, frmUCSEditor::OnSaveFile)
	EVT_BUTTON(wxID_OPEN, frmUCSEditor::OnLoad)

	EVT_BUTTON(wxID_APPLY, frmUCSEditor::OnApply)
	EVT_TOOL(IDC_ToolApply, frmUCSEditor::OnApply)
	EVT_BUTTON(wxID_CANCEL, frmUCSEditor::OnClose)
	EVT_TOOL(IDC_ToolClose, frmUCSEditor::OnClose)

	EVT_CLOSE(frmUCSEditor::OnCloseWindow)
END_EVENT_TABLE()

void frmUCSEditor::OnClose(wxCommandEvent& event)
{ UNUSED(event);
	wxCloseEvent oClose;
	oClose.SetCanVeto(true);
	OnCloseWindow(oClose);
	if(oClose.GetVeto()) return;

	frmTabDialog* pParent = (frmTabDialog*)GetParent()->GetParent();
	pParent->EndModal(wxID_OK);
}

void frmUCSEditor::OnApply(wxCommandEvent& event)
{
	wxPGId oSelected = m_pPropertyGrid->GetSelectedProperty();
	if(!oSelected.IsOk()) return;

	if(m_pResultVal)
	{
		wxString sStr = m_pPropertyGrid->GetPropertyLabel(oSelected);
		*m_pResultVal = wcstoul(sStr.c_str() + 1, 0, 10);
	}
	OnClose(event);
}

frmUCSEditor::frmUCSEditor(wxWindow* parent, wxWindowID id, bool bReadOnly, const wxPoint& pos, const wxSize& size, unsigned long* pResult)
	: wxWindow(parent, id, pos, size)
{
	m_pTabStripForLoad = 0;
	m_pUCS = 0;
	m_pResultVal = pResult;
	m_bNeedsSave = false;
	m_bReadOnly = bReadOnly;
	wxBoxSizer *pTopSizer = new wxBoxSizer( wxVERTICAL );

	wxToolBar *pToolbar;
	pTopSizer->Add(pToolbar = new wxToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_HORIZONTAL | wxNO_BORDER), 0, wxEXPAND | wxALL, 3);
	wxBitmap oSaveBmp(wxT("IDB_32SAVE") ,wxBITMAP_TYPE_BMP_RESOURCE), oAddBmp(wxT("IDB_32ADD") ,wxBITMAP_TYPE_BMP_RESOURCE);
	wxBitmap oApplyBmp(wxT("IDB_32APPLY") ,wxBITMAP_TYPE_BMP_RESOURCE), oCancelBmp(wxT("IDB_32CANCEL") ,wxBITMAP_TYPE_BMP_RESOURCE);
	oSaveBmp.SetMask(new wxMask(oSaveBmp, wxColour(128, 128, 128)));
	oAddBmp.SetMask(new wxMask(oAddBmp, wxColour(128, 128, 128)));
	oApplyBmp.SetMask(new wxMask(oApplyBmp, wxColour(128, 128, 128)));
	oCancelBmp.SetMask(new wxMask(oCancelBmp, wxColour(128, 128, 128)));
	pToolbar->SetToolBitmapSize(wxSize(32,32));
	pToolbar->AddTool(IDC_ToolSave, AppStr(ucsedit_save), oSaveBmp, AppStr(ucsedit_save));
	pToolbar->AddTool(IDC_ToolAdd, AppStr(ucsedit_newentry), oAddBmp, AppStr(ucsedit_newentry));
	if(m_pResultVal)
	{
		pToolbar->AddTool(IDC_ToolClose, AppStr(ucsedit_rgdcancel), oCancelBmp, AppStr(ucsedit_rgdcancel));
		pToolbar->AddTool(IDC_ToolApply, AppStr(ucsedit_rgdapply), oApplyBmp, AppStr(ucsedit_rgdapply));
	}
	pToolbar->Realize();

	pTopSizer->Add(m_pPropertyGrid = new wxPropertyGrid(this, IDC_PropertyGrid, wxDefaultPosition, wxDefaultSize, wxPG_DEFAULT_STYLE | wxPG_HIDE_MARGIN), 1, wxEXPAND | wxALL, 0); 

	wxBoxSizer *pButtonSizer = new wxBoxSizer( wxHORIZONTAL );
	wxWindow *pBgTemp, *pButton2;

	pButtonSizer->Add(m_pLoadButton = new wxButton(this, wxID_OPEN, AppStr(ucsedit_load_title)), 0, wxEXPAND | wxALL, 3);
	if(m_pResultVal)
	{
		pButtonSizer->Add(new wxButton(this, wxID_APPLY, AppStr(ucsedit_rgdapply)), 0, wxEXPAND | wxALL, 3);
		pButtonSizer->Add(new wxButton(this, wxID_CANCEL, AppStr(ucsedit_rgdcancel)), 0, wxEXPAND | wxALL, 3);
	}
	pButtonSizer->Add(pBgTemp = new wxButton(this, wxID_NEW, AppStr(ucsedit_newentry)), 0, wxEXPAND | wxALL, 3);
	pButtonSizer->Add(pButton2 = new wxButton(this, wxID_SAVE, AppStr(ucsedit_save)), 0, wxEXPAND | wxALL, 3);

	pTopSizer->Add(pButtonSizer, 0, wxALIGN_RIGHT);

	SetBackgroundColour(pBgTemp->GetBackgroundColour());

	m_pLoadButton->Show(false);

	SetSizer(pTopSizer);
	pTopSizer->SetSizeHints( this );
}

void frmUCSEditor::SetTabStripForLoad(wxAuiNotebook* pTabStrib)
{
	m_pTabStripForLoad = pTabStrib;

	m_pLoadButton->Show(m_pTabStripForLoad != 0);

	Layout();
}

void frmUCSEditor::OnLoad(wxCommandEvent& event)
{ UNUSED(event);
	frmUCSSelector *pSelector = new frmUCSSelector(wxT("Select UCS to get value from"));
	CUcsTool::HandleSelectorResponse(&*pSelector, m_pTabStripForLoad, m_pResultVal, true);
}

frmUCSEditor::~frmUCSEditor()
{
	delete m_pUCS;
}

void frmUCSEditor::FillFromCUcsFile(CUcsFile* pUcs, unsigned long iSelect)
{
	wxPGId oSelectMe;

	delete m_pUCS;
	m_pUCS = new CUcsTransaction(pUcs);
	std::map<unsigned long, wchar_t*> *pEntries;
	try
	{
		pEntries = pUcs->GetRawMap();
	}
	catch(CRainmanException *pE)
	{
		throw new CModStudioException(__FILE__, __LINE__, "Unable to get UCS mappings", pE);
	}
	for(std::map<unsigned long, wchar_t*>::iterator itr = pEntries->begin(); itr != pEntries->end(); ++itr)
	{
		if(itr->second)
		{
			wchar_t sNumberBuffer[34];
			sNumberBuffer[0] = '$';
			_ultow(itr->first, sNumberBuffer + 1, 10);
			wxPGProperty* pTmp;
			wxPGId oTmp = m_pPropertyGrid->Append( pTmp = wxStringProperty(sNumberBuffer, sNumberBuffer, itr->second) );
			if(m_bReadOnly) pTmp->SetFlag(wxPG_PROP_DISABLED);
			if(iSelect == itr->first) oSelectMe = oTmp;
		}
	}
	m_pPropertyGrid->SetSplitterLeft();

	if(oSelectMe.IsOk())
	{
		m_pPropertyGrid->EnsureVisible(oSelectMe);
		m_pPropertyGrid->SelectProperty(oSelectMe, true);
	}
}

void frmUCSEditor::OnSize(wxSizeEvent& event)
{ UNUSED(event);
	Layout();
}

void frmUCSEditor::OnPropertyChange(wxPropertyGridEvent& event)
{
	unsigned long iID = wcstoul(((const wchar_t*)event.GetPropertyLabel()) + 1, 0, 10);
	try
	{
		m_pUCS->SetString(iID, event.GetPropertyValueAsString());
	}
	catch(CRainmanException *pE)
	{
		ErrorBoxE(pE);
	}
	m_bNeedsSave = true;
}

void frmUCSEditor::OnNewEntry(wxCommandEvent& event)
{ UNUSED(event);
	if(m_bReadOnly)
	{
		wxMessageBox(AppStr(ucsedit_readonlyerror),AppStr(ucsedit_newentry),wxICON_ERROR,this);
		return;
	}

	wxString sLastID;
	wchar_t sNumberBuffer[34];
	sNumberBuffer[0] = '$';
	try
	{
		if(m_pUCS->GetRawMap()->size() > 0)
			_ultow((m_pUCS->GetRawMap()->rbegin()->first) + 1, sNumberBuffer + 1, 10);
		else
			wcscpy(sNumberBuffer, AppStr(ucsedit_newentrydefault));
	}
	catch(CRainmanException *pE)
	{
		ErrorBoxE(pE);
		return;
	}

	wxString sNewID;
	sNewID = wxGetTextFromUser(AppStr(ucsedit_newentrycaption), AppStr(ucsedit_newentry), sNumberBuffer, this, wxDefaultCoord, wxDefaultCoord, false);
	if(!sNewID.IsEmpty())
	{
		const wchar_t* pStr = sNewID;
		while(*pStr && (*pStr < '0' || *pStr > '9')) ++pStr;
		if(*pStr)
		{
			unsigned long iNewID = wcstoul(pStr, 0, 10);
			if(iNewID < 15000000 || iNewID > 20000000)
			{
				bool bDontAsk;
				TheConfig->Read(AppStr(config_mod_ucsrangeremember), &bDontAsk, false);
				if(!bDontAsk)
				{
					frmUCSOutOfRange *pQuestion = new frmUCSOutOfRange(AppStr(ucsrange_title), iNewID);
					if(pQuestion->ShowModal() == wxID_NO)
					{
						delete pQuestion;
						return;
					}
					delete pQuestion;
				}
			}
			std::map<unsigned long, wchar_t*> *pEntries;
			try
			{
				pEntries = m_pUCS->GetRawMap();
			}
			catch(CRainmanException *pE)
			{
				ErrorBoxE(pE);
				return;
			}
			for(std::map<unsigned long, wchar_t*>::iterator itr = pEntries->begin(); itr != pEntries->end(); ++itr)
			{
				if(itr->second)
				{
					if(itr->first == iNewID)
					{
						wxMessageBox(AppStr(ucsedit_newentrydupcaption),AppStr(ucsedit_newentryduptitle),wxICON_ERROR,this);
						return;
					}
					else if(itr->first >= iNewID)
					{
						wchar_t sNumberBuffer[34];
						sNumberBuffer[0] = '$';
						_ultow(itr->first, sNumberBuffer + 1, 10);
						wxPGId oEntry = m_pPropertyGrid->GetPropertyByLabel(sNumberBuffer);
						_ultow(iNewID, sNumberBuffer + 1, 10);
						try
						{
							m_pUCS->SetString(iNewID, L"");
							m_bNeedsSave = true;
						}
						catch(CRainmanException *pE)
						{
							ErrorBoxE(pE);
							return;
						}
						oEntry = m_pPropertyGrid->Insert(oEntry, wxStringProperty(sNumberBuffer, sNumberBuffer, wxT("")));
						m_pPropertyGrid->Refresh();
						m_pPropertyGrid->EnsureVisible(oEntry);
						m_pPropertyGrid->SelectProperty(oEntry, true);
						return;
					}
				}
			}
			_ultow(iNewID, sNumberBuffer + 1, 10);
			try
			{
				m_pUCS->SetString(iNewID, L"");
				m_bNeedsSave = true;
			}
			catch(CRainmanException *pE)
			{
				ErrorBoxE(pE);
				return;
			}
			wxPGId oEntry = m_pPropertyGrid->Append(wxStringProperty(sNumberBuffer, sNumberBuffer, wxT("")));
			m_pPropertyGrid->Refresh();
			m_pPropertyGrid->EnsureVisible(oEntry);
			m_pPropertyGrid->SelectProperty(oEntry, true);
			if(m_pPropertyGrid->GetChildrenCount() == 1)
			{
				m_pPropertyGrid->SetSplitterLeft();
			}
			return;
		}
	}
}

void frmUCSEditor::OnSaveFile(wxCommandEvent& event)
{ UNUSED(event);
	if(m_bReadOnly)
	{
		wxMessageBox(AppStr(ucsedit_readonlyerror),AppStr(ucsedit_save),wxICON_ERROR,this);
		return;
	}

	DoSave();
}

void frmUCSEditor::DoSave()
{
	wxString sNewFile = TheConstruct->GetModuleFile().BeforeLast('\\');
	sNewFile.Append(wxT("\\"));
	try
	{
		if(TheConstruct->GetModule()->GetModuleType() == CModuleFile::MT_CompanyOfHeroes)
		{
			sNewFile.Append(wxT("Engine\\Locale\\"));
		}
		else
		{
			const char* sLocFolder = TheConstruct->GetModule()->GetLocaleFolder();
			if(sLocFolder && *sLocFolder)
			{
				sNewFile.Append(AsciiTowxString(sLocFolder));
				sNewFile.Append(wxT("\\"));
			}
			else
			{
				sNewFile.Append(AsciiTowxString(TheConstruct->GetModule()->GetModFolder()));
				sNewFile.Append(wxT("\\Locale\\"));
			}
		}
		sNewFile.Append(AsciiTowxString(TheConstruct->GetModule()->GetLocale()));
	}
	catch(CRainmanException *pE)
	{
		ErrorBoxE(pE);
		return;
	}
	sNewFile.Append(wxT("\\"));
	
	try
	{
		size_t iUCSCount = TheConstruct->GetModule()->GetUcsCount();
		for(size_t i = 0; i < iUCSCount; ++i)
		{
			CModuleFile::CUcsHandler* pUcs = TheConstruct->GetModule()->GetUcs(i);
			if(pUcs->GetUcsHandle() == m_pUCS->GetRawObject())
			{
				sNewFile.Append(AsciiTowxString( pUcs->GetFileName() ));
				goto found_ucs_file;
			}
		}

		if(TheConstruct->GetModule()->GetModuleType() == CModuleFile::MT_CompanyOfHeroes)
		{
			size_t iModCount = TheConstruct->GetModule()->GetRequiredCount();
			for(size_t i = 0; i < iModCount; ++i)
			{
				CModuleFile* pMod = TheConstruct->GetModule()->GetRequired(i)->GetModHandle();
				iUCSCount = pMod->GetUcsCount();
				for(size_t i = 0; i < iUCSCount; ++i)
				{
					CModuleFile::CUcsHandler* pUcs = pMod->GetUcs(i);
					if(pUcs->GetUcsHandle() == m_pUCS->GetRawObject())
					{
						sNewFile.Append(AsciiTowxString( pUcs->GetFileName() ));
						goto found_ucs_file;
					}
				}
			}

			iModCount = TheConstruct->GetModule()->GetEngineCount();
			for(size_t i = 0; i < iModCount; ++i)
			{
				CModuleFile* pMod = TheConstruct->GetModule()->GetEngine(i);
				iUCSCount = pMod->GetUcsCount();
				for(size_t i = 0; i < iUCSCount; ++i)
				{
					CModuleFile::CUcsHandler* pUcs = pMod->GetUcs(i);
					if(pUcs->GetUcsHandle() == m_pUCS->GetRawObject())
					{
						sNewFile.Append(AsciiTowxString( pUcs->GetFileName() ));
						goto found_ucs_file;
					}
				}
			}
		}
		throw new CModStudioException(__FILE__, __LINE__, "Unable to match UCS handle (should never happen; o.O)");
	}
	catch(CRainmanException *pE)
	{
		ErrorBoxE(pE);
		return;
	}
found_ucs_file:

	BackupFile(sNewFile);
	char* saNewFile = wxStringToAscii(sNewFile);
	try
	{
		m_pUCS->Save(saNewFile);
	}
	catch(CRainmanException *pE)
	{
		delete[] saNewFile;
		ErrorBoxE(pE);
		RestoreBackupFile(sNewFile);
		return;
	}
	delete[] saNewFile;

	m_bNeedsSave = false;
	wxMessageBox(AppStr(ucsedit_save_caption),AppStr(ucsedit_save_title),wxICON_INFORMATION,this);
}

void frmUCSEditor::OnCloseWindow(wxCloseEvent& event)
{
	if(m_bNeedsSave)
	{
		wxMessageDialog* dialog = new wxMessageDialog(this,
		wxT("UCS has been modified. Save file?"), wxT("UCS Editor"), wxYES_NO|wxCANCEL);

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