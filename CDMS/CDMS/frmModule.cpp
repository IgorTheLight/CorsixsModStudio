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

#include "frmModule.h"
#include "Construct.h"
#include "strconv.h"
#include "CtrlStatusText.h"
#include "strings.h"
#include <wx/notebook.h>
#include "Common.h"

BEGIN_EVENT_TABLE(frmModule, wxWindow)
	EVT_SIZE(frmModule::OnSize)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(frmModule::pgMain, wxWindow)
	EVT_SIZE(frmModule::pgMain::OnSize)
	EVT_TEXT(IDC_Description, frmModule::pgMain::OnDescriptionUpdate)
	EVT_TEXT(IDC_UIName, frmModule::pgMain::OnUINameUpdate)
	EVT_TEXT(IDC_TextureFE, frmModule::pgMain::OnTextureFEUpdate)
	EVT_TEXT(IDC_TextureIcon, frmModule::pgMain::OnTextureIconUpdate)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(frmModule::pgDataFolders, wxWindow)
	EVT_SIZE(frmModule::pgDataFolders::OnSize)
END_EVENT_TABLE()

frmModule::frmModule(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
	: wxWindow(parent, id, pos, size)
{
	SetBackgroundStyle(wxBG_STYLE_SYSTEM);
	wxBoxSizer *pTopSizer = new wxBoxSizer( wxVERTICAL );
	wxNotebook *pTabs;

	pTopSizer->Add(pTabs = new wxNotebook(this, -1, wxPoint(0,0)), 1, wxEXPAND | wxALL, 3);
	pTabs->AddPage(new pgMain(pTabs, IDP_General), wxT("General"));
	pTabs->AddPage(new pgDataFolders(pTabs, IDP_DataFolders), wxT("Data Folders"));
	pTabs->AddPage(new pgDataArchives(pTabs, IDP_DataArchives), wxT("Data Archives"));
	pTabs->AddPage(new pgRequiredMods(pTabs, IDP_RequiredMods), wxT("Required Mods"));
	pTabs->AddPage(new pgCompatibleMods(pTabs, IDP_CompatibleMods), wxT("Compatible Mods"));

	SetBackgroundColour(pTabs->GetBackgroundColour());
	SetSizer(pTopSizer);
	pTopSizer->SetSizeHints( this );
}

void frmModule::OnSize(wxSizeEvent& event)
{ UNUSED(event);
	Layout();
}

frmModule::pgMain::pgMain(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
	: wxWindow(parent, id, pos, size)
{
	m_bDoneInit = false;
	SetBackgroundStyle(wxBG_STYLE_SYSTEM);
	wxFlexGridSizer *pTopSizer = new wxFlexGridSizer(2);
	pTopSizer->SetFlexibleDirection(wxHORIZONTAL);
	pTopSizer->AddGrowableCol(1, 1);
	wxStaticText* pBgTemp;

	wxArrayString aEmptyList;

	CModuleFile* pMod = TheConstruct->GetModule();
	wxControlWithItems* pModFolders;

	pTopSizer->Add(SBT(pBgTemp = new wxStaticText(this, -1, AppStr(mod_description)), AppStr(mod_description_help)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE | wxALL, 3);
	pTopSizer->Add(SBT(new wxTextCtrl(this, IDC_Description, AsciiTowxString(pMod->GetDescription())), AppStr(mod_description_help)), 0, wxALL | wxEXPAND, 3);
	pTopSizer->Add(SBT(new wxStaticText(this, -1, AppStr(mod_dll)), AppStr(mod_dll_help)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE | wxALL, 3);
	pTopSizer->Add(SBT(pModFolders = new wxChoice(this, IDC_DllName, wxDefaultPosition, wxDefaultSize, aEmptyList, wxCB_SORT), AppStr(mod_dll_help)), 0, wxALL | wxEXPAND, 3);
	InitDllList(pModFolders);
	pTopSizer->Add(SBT(new wxStaticText(this, -1, AppStr(mod_folder)), AppStr(mod_folder_help)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE | wxALL, 3);
	pTopSizer->Add(SBT(pModFolders = new wxComboBox(this, IDC_ModFolder, wxT(""), wxDefaultPosition, wxDefaultSize, aEmptyList, wxCB_SORT), AppStr(mod_folder_help)), 0, wxALL | wxEXPAND, 3);
	InitModFolderList(pModFolders);
	pTopSizer->Add(SBT(new wxStaticText(this, -1, AppStr(mod_texturefe)), AppStr(mod_texturefe_help)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE | wxALL, 3);
	pTopSizer->Add(SBT(new wxTextCtrl(this, IDC_TextureFE, AsciiTowxString(pMod->GetTextureFe())), AppStr(mod_texturefe_help)), 0, wxALL | wxEXPAND, 3);
	pTopSizer->Add(SBT(new wxStaticText(this, -1, AppStr(mod_textureicon)), AppStr(mod_textureicon_help)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE | wxALL, 3);
	pTopSizer->Add(SBT(new wxTextCtrl(this, IDC_TextureIcon, AsciiTowxString(pMod->GetTextureIcon())), AppStr(mod_textureicon_help)), 0, wxALL | wxEXPAND, 3);
	pTopSizer->Add(SBT(new wxStaticText(this, -1, AppStr(mod_uiname)), AppStr(mod_uiname_help)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE | wxALL, 3);
	pTopSizer->Add(SBT(new wxTextCtrl(this, IDC_UIName, AsciiTowxString(pMod->GetUiName())), AppStr(mod_uiname_help)), 0, wxALL | wxEXPAND, 3);

	wxBoxSizer *pVersionSizer = new wxBoxSizer( wxHORIZONTAL );
	wchar_t sNumBuffer[34];
	_ltow(TheConstruct->GetModule()->GetVersionMajor(), sNumBuffer, 10);
	pVersionSizer->Add(SBT(new wxTextCtrl(this, IDC_VersionMajor, sNumBuffer), AppStr(mod_version_help)), 1, wxALL | wxEXPAND, 3);
	pVersionSizer->Add(SBT(new wxStaticText(this, -1, AppStr(decimal_point)), AppStr(mod_version_help)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_BOTTOM |wxFIXED_MINSIZE | wxALL, 3);
	_ltow(TheConstruct->GetModule()->GetVersionMinor(), sNumBuffer, 10);
	pVersionSizer->Add(SBT(new wxTextCtrl(this, IDC_VersionMinor, sNumBuffer), AppStr(mod_version_help)), 1, wxALL | wxEXPAND, 3);
	pVersionSizer->Add(SBT(new wxStaticText(this, -1, AppStr(decimal_point)), AppStr(mod_version_help)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_BOTTOM |wxFIXED_MINSIZE | wxALL, 3);
	_ltow(TheConstruct->GetModule()->GetVersionRevision(), sNumBuffer, 10);
	pVersionSizer->Add(SBT(new wxTextCtrl(this, IDC_VersionRevision, sNumBuffer), AppStr(mod_version_help)), 1, wxALL | wxEXPAND, 3);
	pVersionSizer->Add(new wxButton(this, IDC_VersionHelp, AppStr(question_mark), wxDefaultPosition, wxSize(24,-1)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_BOTTOM |wxFIXED_MINSIZE | wxALL, 3);

	pTopSizer->Add(SBT(new wxStaticText(this, -1, AppStr(mod_version)), AppStr(mod_version_help)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE | wxALL, 3);
	pTopSizer->Add(pVersionSizer, 0, wxALL | wxEXPAND, 0);

	m_bDoneInit = true;
	SetBackgroundColour(pBgTemp->GetBackgroundColour());
	SetSizer(pTopSizer);
	pTopSizer->SetSizeHints( this );
}

void frmModule::pgMain::OnDescriptionUpdate(wxCommandEvent& event)
{
	if(m_bDoneInit)
	{
		wxString sValue = event.GetString();
		char* sVal = wxStringToAscii(sValue);
		TheConstruct->GetModule()->SetDescription(sVal);
		delete[] sVal;
	}
}

void frmModule::pgMain::OnUINameUpdate(wxCommandEvent& event)
{
	if(m_bDoneInit)
	{
		wxString sValue = event.GetString();
		char* sVal = wxStringToAscii(sValue);
		TheConstruct->GetModule()->SetUiName(sVal);
		delete[] sVal;
	}
}

void frmModule::pgMain::OnTextureFEUpdate(wxCommandEvent& event)
{
	if(m_bDoneInit)
	{
		wxString sValue = event.GetString();
		char* sVal = wxStringToAscii(sValue);
		TheConstruct->GetModule()->SetTextureFe(sVal);
		delete[] sVal;
	}
}

void frmModule::pgMain::OnTextureIconUpdate(wxCommandEvent& event)
{
	if(m_bDoneInit)
	{
		wxString sValue = event.GetString();
		char* sVal = wxStringToAscii(sValue);
		TheConstruct->GetModule()->SetTextureIcon(sVal);
		delete[] sVal;
	}
}

void frmModule::pgMain::InitModFolderList(wxControlWithItems* pList)
{
	CFileSystemStore oFSO;
	wxArrayString oStrings;
	char* sDoWFolder = wxStringToAscii(TheConstruct->GetModuleFile().BeforeLast('\\'));
	IDirectoryTraverser::IIterator* pItr = oFSO.VIterate(sDoWFolder);
	delete[] sDoWFolder;
	IDirectoryTraverser::IIterator::eTypes eType;
	eType = pItr->VGetType();
	while(eType == IDirectoryTraverser::IIterator::T_Directory || eType == IDirectoryTraverser::IIterator::T_File)
	{
		if(eType == IDirectoryTraverser::IIterator::T_Directory)
		{
			wxString sThisFolder = AsciiTowxString(pItr->VGetName());
			if(sThisFolder != wxT("Badges") && sThisFolder != wxT("Banners")
			&& sThisFolder != wxT("BugReport") && sThisFolder != wxT("Drivers")
			&& sThisFolder != wxT("Engine") && sThisFolder != wxT("GraphicsOptions")
			&& sThisFolder != wxT("Logfiles") && sThisFolder != wxT("ModTools")
			&& sThisFolder != wxT("Patch") && sThisFolder != wxT("Playback")
			&& sThisFolder != wxT("Profiles") && sThisFolder != wxT("ScreenShots")
			&& sThisFolder != wxT("Stats") && sThisFolder != wxT("Utils"))
			{
				oStrings.Add(sThisFolder);
			}
		}
		if(pItr->VNextItem() != IDirectoryTraverser::IIterator::E_OK) break;
		eType = pItr->VGetType();
	}
	delete pItr;

	pList->Append(oStrings);
	pList->SetStringSelection(AsciiTowxString(TheConstruct->GetModule()->GetModFolder()));
}

void frmModule::pgMain::InitDllList(wxControlWithItems* pList)
{
	wxString sTheDll = AsciiTowxString(TheConstruct->GetModule()->GetDllName());
	CFileSystemStore oFSO;
	wxArrayString oStrings;
	char* sDoWFolder = wxStringToAscii(TheConstruct->GetModuleFile().BeforeLast('\\'));
	IDirectoryTraverser::IIterator* pItr = oFSO.VIterate(sDoWFolder);
	delete[] sDoWFolder;
	IDirectoryTraverser::IIterator::eTypes eType;
	eType = pItr->VGetType();
	while(eType == IDirectoryTraverser::IIterator::T_Directory || eType == IDirectoryTraverser::IIterator::T_File)
	{
		if(eType == IDirectoryTraverser::IIterator::T_File)
		{
			wxString sThisFile = AsciiTowxString(pItr->VGetName());
			if(sThisFile.AfterLast('.') == wxT("dll"))
			{
				sThisFile = sThisFile.BeforeLast('.');
				if(!(sThisFile.IsSameAs(sTheDll, false))
				&& !(sThisFile.IsSameAs(wxT("dbghelp"), false)) && !(sThisFile.IsSameAs(wxT("Debug"), false))
				&& !(sThisFile.IsSameAs(wxT("divxdecoder"), false)) && !(sThisFile.IsSameAs(wxT("divxmedialib"), false))
				&& !(sThisFile.IsSameAs(wxT("DllTie"), false)) && !(sThisFile.IsSameAs(wxT("fileparser"), false))
				&& !(sThisFile.IsSameAs(wxT("FileSystem"), false)) && !(sThisFile.IsSameAs(wxT("GSLobby"), false))
				&& !(sThisFile.IsSameAs(wxT("ijl15"), false)) && !(sThisFile.IsSameAs(wxT("Localizer"), false))
				&& !(sThisFile.IsSameAs(wxT("luabind"), false)) && !(sThisFile.IsSameAs(wxT("LuaConfig"), false))
				&& !(sThisFile.IsSameAs(wxT("MathBox"), false)) && !(sThisFile.IsSameAs(wxT("Memory"), false))
				&& !(sThisFile.IsSameAs(wxT("mfc71"), false)) && !(sThisFile.IsSameAs(wxT("msvcp71"), false))
				&& !(sThisFile.IsSameAs(wxT("msvcr71"), false)) && !(sThisFile.IsSameAs(wxT("Patch"), false))
				&& !(sThisFile.IsSameAs(wxT("Platform"), false)) && !(sThisFile.IsSameAs(wxT("PlatHook"), false))
				&& !(sThisFile.IsSameAs(wxT("seInterface"), false)) && !(sThisFile.IsSameAs(wxT("SimEngine"), false))
				&& !(sThisFile.IsSameAs(wxT("spDx9"), false)) && !(sThisFile.IsSameAs(wxT("STLPort"), false))
				&& !(sThisFile.IsSameAs(wxT("UserInterface"), false)) && !(sThisFile.IsSameAs(wxT("Util"), false))
				)
				{
					oStrings.Add(sThisFile);
				}
			}
		}
		if(pItr->VNextItem() != IDirectoryTraverser::IIterator::E_OK) break;
		eType = pItr->VGetType();
	}
	delete pItr;

	oStrings.Add(sTheDll);
	pList->Append(oStrings);
	pList->SetStringSelection(sTheDll);
}

void frmModule::pgMain::OnSize(wxSizeEvent& event)
{ UNUSED(event);
	Layout();
}

frmModule::pgDataFolders::pgDataFolders(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
										const wxString& sTitle, const wxString& sItemName, bool bUpdateMessage, void (* pInitList)(wxArrayString &))
	: wxWindow(parent, id, pos, size)
{
	SetBackgroundStyle(wxBG_STYLE_SYSTEM);
	wxBoxSizer *pTopSizer = new wxBoxSizer( wxVERTICAL );
	wxStaticText* pBgTemp;

	wxArrayString aActiveValues;
	pInitList(aActiveValues);

	pTopSizer->Add(pBgTemp = new wxStaticText(this, -1, sTitle), 0, wxALIGN_LEFT | wxALL, 3);
	pTopSizer->Add(new wxListBox(this, -1, wxDefaultPosition, wxDefaultSize, aActiveValues), 1, wxALL | wxEXPAND, 3);

	wxBoxSizer *pButtons = new wxBoxSizer( wxHORIZONTAL );
	pButtons->Add(SBT(new wxBitmapButton(this, -1, wxIcon(wxT("#111"), wxBITMAP_TYPE_ICO_RESOURCE, 16, 16)), AppStrS(mod_add_a_new, sItemName)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 3);
	pButtons->Add(SBT(new wxBitmapButton(this, -1, wxIcon(wxT("#112"), wxBITMAP_TYPE_ICO_RESOURCE, 16, 16)), AppStrS(mod_remove_an_existing, sItemName)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 3);
	pButtons->Add(SBT(new wxBitmapButton(this, -1, wxIcon(wxT("#113"), wxBITMAP_TYPE_ICO_RESOURCE, 16, 16)), AppStrS(mod_move_up, sItemName)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 3);
	pButtons->Add(SBT(new wxBitmapButton(this, -1, wxIcon(wxT("#114"), wxBITMAP_TYPE_ICO_RESOURCE, 16, 16)), AppStrS(mod_move_down, sItemName)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 3);
	if(bUpdateMessage) pButtons->Add(new wxStaticText(this, -1, AppStrS(mod_changes_to, sItemName), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 3);

	pTopSizer->Add(pButtons, 0, wxALL | wxALIGN_LEFT, 0);

	SetBackgroundColour(pBgTemp->GetBackgroundColour());
	SetSizer(pTopSizer);
	pTopSizer->SetSizeHints( this );
}

void frmModule::pgDataFolders::OnSize(wxSizeEvent& event)
{ UNUSED(event);
	Layout();
}

void frmModule::pgDataFolders::FillInitialValues(wxArrayString &aInitialValues)
{
	CModuleFile *pMod = TheConstruct->GetModule();
	size_t iCount = pMod->GetFolderCount();
	for(size_t i = 0; i < iCount; ++i)
	{
		CModuleFile::CFolderHandler *pFolder = pMod->GetFolder(i);
		wxString S;
		S.Printf(wxT("[%li] "), pFolder->GetNumber());
		S.Append( AsciiTowxString( pFolder->GetName() ) );
		aInitialValues.Add(S);
	}
}


frmModule::pgDataArchives::pgDataArchives(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
: pgDataFolders(parent, id, pos, size, AppStr(mod_dataarchives_caption), AppStr(mod_dataarchive), true, FillInitialValues)
{
}

void frmModule::pgDataArchives::FillInitialValues(wxArrayString &aInitialValues)
{
	CModuleFile *pMod = TheConstruct->GetModule();
	size_t iCount = pMod->GetArchiveCount();
	for(size_t i = 0; i < iCount; ++i)
	{
		CModuleFile::CArchiveHandler* pArch = pMod->GetArchive(i);
		wxString S;
		S.Printf(wxT("[%li] "), pArch->GetNumber() );
		S.Append(AsciiTowxString( pArch->GetFileName() ));
		aInitialValues.Add(S);
	}
}

frmModule::pgRequiredMods::pgRequiredMods(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
: pgDataFolders(parent, id, pos, size, AppStr(mod_requiredmods_caption), AppStr(mod_requiredmod), true, FillInitialValues)
{
}

void frmModule::pgRequiredMods::FillInitialValues(wxArrayString &aInitialValues)
{
	CModuleFile *pMod = TheConstruct->GetModule();
	size_t iCount = pMod->GetRequiredCount();
	for(size_t i = 0; i < iCount; ++i)
	{
		CModuleFile::CRequiredHandler* pReq = pMod->GetRequired(i);
		wxString S;
		S.Printf(wxT("[%li] "), pReq->GetNumber() );
		S.Append(AsciiTowxString( pReq->GetFileName() ));
		aInitialValues.Add(S);
	}
}

frmModule::pgCompatibleMods::pgCompatibleMods(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
: pgDataFolders(parent, id, pos, size, AppStr(mod_compatiblemods_caption), AppStr(mod_compatiblemod), false, FillInitialValues)
{
}

void frmModule::pgCompatibleMods::FillInitialValues(wxArrayString &aInitialValues)
{
	CModuleFile *pMod = TheConstruct->GetModule();
	size_t iCount = pMod->GetCompatibleCount();
	for(size_t i = 0; i < iCount; ++i)
	{
		CModuleFile::CCompatibleHandler* pCompat = pMod->GetCompatible(i);
		wxString S;
		S.Printf(wxT("[%li] "), pCompat->GetNumber() );
		S.Append(AsciiTowxString( pCompat->GetFileName() ));
		aInitialValues.Add(S);
	}
}