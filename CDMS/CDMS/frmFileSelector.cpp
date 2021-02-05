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

#include "frmFileSelector.h"
#include "strings.h"
#include "strconv.h"
#include "Construct.h"
#include "config.h"
#include <wx/textdlg.h>
#include <list>
#include "Common.h"

BEGIN_EVENT_TABLE(frmFileSelector, wxDialog)
	EVT_SIZE(frmFileSelector::OnSize)
	EVT_TREE_ITEM_EXPANDING(IDC_FilesTable, frmFileSelector::OnTreeExpanding)
	EVT_TREE_SEL_CHANGED(IDC_FilesTable, frmFileSelector::OnTreeSelect)
	EVT_BUTTON(wxID_OK, frmFileSelector::OnOkClick)
	EVT_BUTTON(wxID_CANCEL, frmFileSelector::OnCloseClick)
END_EVENT_TABLE()

std::list<IDirectoryTraverser::IIterator*> g_vIteratorsToFree;

CFileSelectorTreeItemData::CFileSelectorTreeItemData(IDirectoryTraverser::IIterator* pItr, bool bToFillWith)
{
	if(bToFillWith)
	{
		pToFillWith = pItr;
		if(pItr) g_vIteratorsToFree.push_back(pItr);
		sMod = 0;
		sSource = 0;
	}
	else
	{
		pToFillWith = 0;
		sMod = 0;
		sSource = 0;
		if(pItr->VGetType() == IDirectoryTraverser::IIterator::T_File)
		{
			try
			{
				sMod = (const char*) pItr->VGetTag(0);
			}
			catch(CRainmanException *pE) {pE->destroy();}
			try
			{
				sSource = (const char*) pItr->VGetTag(1);
			}
			catch(CRainmanException *pE) {pE->destroy();}
		}
	}
}

frmFileSelector::~frmFileSelector()
{
	for(std::list<IDirectoryTraverser::IIterator*>::iterator itr = g_vIteratorsToFree.begin(); itr != g_vIteratorsToFree.end(); ++itr)
	{
		delete *itr;
	}
	g_vIteratorsToFree.clear();
}

frmFileSelector::frmFileSelector(wxString sBaseFolder, wxString sExistingSelection, bool bRgdMode)
	:wxDialog(wxTheApp->GetTopWindow(), -1, wxT("File Selector"), wxPoint(0, 0) , wxSize(320, 480), wxFRAME_FLOAT_ON_PARENT | wxFRAME_TOOL_WINDOW | wxCAPTION),
	m_sBaseFolder(sBaseFolder),
	m_bRgdMode(bRgdMode)
{
	m_cThisMod = ConfGetColour(AppStr(file_colourthismod), 128, 64, 64);
	m_cOtherMod = ConfGetColour(AppStr(file_colourothermod), 64, 64, 128);
	m_cEngine = ConfGetColour(AppStr(file_colourengine), 128, 128, 128);

	wxWindow* pBgTemp;
	CentreOnParent();
	wxBoxSizer *pTopSizer = new wxBoxSizer( wxVERTICAL );

	pTopSizer->Add(m_pTree = new wxTreeCtrl(this, IDC_FilesTable, wxDefaultPosition, wxSize(260, 320), wxTR_HAS_BUTTONS | wxTR_HIDE_ROOT | wxTR_SINGLE | wxTR_LINES_AT_ROOT), 1, wxALL | wxEXPAND, 3);

	wxImageList* pFileTypes = new wxImageList(16, 16);
	pFileTypes->Add(wxBitmap(wxT("IDB_FILEAI"), wxBITMAP_TYPE_BMP_RESOURCE)); // ( 0) AI
	pFileTypes->Add(wxBitmap(wxT("IDB_FILELUA"), wxBITMAP_TYPE_BMP_RESOURCE)); // ( 1) LUA
	pFileTypes->Add(wxBitmap(wxT("IDB_FILENIL"), wxBITMAP_TYPE_BMP_RESOURCE)); // ( 2) NIL
	pFileTypes->Add(wxBitmap(wxT("IDB_FILERGD"), wxBITMAP_TYPE_BMP_RESOURCE)); // ( 3) RGD
	pFileTypes->Add(wxBitmap(wxT("IDB_FOLDERCLOSE"), wxBITMAP_TYPE_BMP_RESOURCE)); // ( 4) Closed
	pFileTypes->Add(wxBitmap(wxT("IDB_FOLDEROPEN"), wxBITMAP_TYPE_BMP_RESOURCE)); // ( 5) Open
	pFileTypes->Add(wxBitmap(wxT("IDB_FILESCAR"), wxBITMAP_TYPE_BMP_RESOURCE)); // ( 6) SCAR
	pFileTypes->Add(wxBitmap(wxT("IDB_TOC"), wxBITMAP_TYPE_BMP_RESOURCE)); // ( 7) ToC
	pFileTypes->Add(wxBitmap(wxT("IDB_FILETGA"), wxBITMAP_TYPE_BMP_RESOURCE)); // ( 8) TGA
	pFileTypes->Add(wxBitmap(wxT("IDB_FILERGT"), wxBITMAP_TYPE_BMP_RESOURCE)); // ( 9) RGT
	pFileTypes->Add(wxBitmap(wxT("IDB_FILEDDS"), wxBITMAP_TYPE_BMP_RESOURCE)); // (10) DDS
	pFileTypes->Add(wxBitmap(wxT("IDB_FILEBFX"), wxBITMAP_TYPE_BMP_RESOURCE)); // (11) BFX
	pFileTypes->Add(wxBitmap(wxT("IDB_FILEABP"), wxBITMAP_TYPE_BMP_RESOURCE)); // (12) ABP
	pFileTypes->Add(wxBitmap(wxT("IDB_FILERGM"), wxBITMAP_TYPE_BMP_RESOURCE)); // (13) RGM
	m_pTree->AssignImageList(pFileTypes);

	CModuleFile *pMod = TheConstruct->GetModule();
	if(sBaseFolder.Len() == 0)
	{
		wxTreeItemId oRoot = m_pTree->AddRoot(wxT(""));

		for(unsigned long iEntry = 0; iEntry < pMod->VGetEntryPointCount(); ++iEntry)
		{
			IDirectoryTraverser::IIterator* pItr = 0;
			const char* sName = 0;
			try
			{
				sName = pMod->VGetEntryPoint(iEntry);
			}
			catch(CRainmanException *pE)
			{
				pE->destroy();
				continue;
			}
			try
			{
				pItr = pMod->VIterate(sName);
				if(pItr && pItr->VGetType() == IDirectoryTraverser::IIterator::T_Nothing)
				{
					delete pItr;
					pItr = 0;
				}
			}
			catch(CRainmanException *pE)
			{
				pE->destroy();
				pItr = 0;
			}
			wxTreeItemId oChild = m_pTree->AppendItem(oRoot, AsciiTowxString(sName), 7, 7, new CFileSelectorTreeItemData(pItr, true));
			if(pItr) m_pTree->SetItemHasChildren(oChild, true);
		}
	}
	else
	{
		IDirectoryTraverser::IIterator* pRootItr = 0;
		const char* sBaseFolderAscii = wxStringToAscii(sBaseFolder);
		try
		{
			pRootItr = pMod->VIterate(sBaseFolderAscii);
			if(pRootItr && pRootItr->VGetType() == IDirectoryTraverser::IIterator::T_Nothing)
			{
				delete pRootItr;
				pRootItr = 0;
			}
		}
		catch(CRainmanException *pE)
		{
			pE->destroy();
			pRootItr = 0;
		}
		delete[] sBaseFolderAscii;
		wxTreeItemId oRoot = m_pTree->AddRoot(wxT(""), -1, -1, new CFileSelectorTreeItemData(pRootItr, true));
		MakeChildren(oRoot);
	}

	wxBoxSizer *pNameSizer = new wxBoxSizer( wxHORIZONTAL );
	if(sBaseFolder.Len()) pNameSizer->Add(new wxStaticText(this, -1, sBaseFolder), 0, wxEXPAND | wxALL | wxALIGN_CENTER_VERTICAL, 3);
	pNameSizer->Add(m_pText = new wxTextCtrl(this, -1, wxT("")), 1, wxEXPAND | wxALL | wxALIGN_CENTER_VERTICAL, 3);
	pTopSizer->Add(pNameSizer, 0, wxALIGN_LEFT | wxEXPAND);
	m_pText->SetValue(sExistingSelection);
	if(m_bRgdMode)
	{
		if(sExistingSelection.Find('.', true) != wxNOT_FOUND)
		{
			m_sRgdModeExt = wxString(wxT(".")) + sExistingSelection.AfterLast('.');
			sExistingSelection = sExistingSelection.BeforeLast('.');
		}
	}
	FindAndSelect(sExistingSelection);

	wxBoxSizer *pButtonSizer = new wxBoxSizer( wxHORIZONTAL );
	pButtonSizer->Add(pBgTemp= new wxButton(this, wxID_OK, wxT("OK")), 0, wxEXPAND | wxALL, 3);
	pButtonSizer->Add(new wxButton(this, wxID_CANCEL, wxT("Cancel")), 0, wxEXPAND | wxALL, 3);
	pTopSizer->Add(pButtonSizer, 0, wxALIGN_CENTER);

	SetSizer(pTopSizer);
	pTopSizer->SetSizeHints( this );
	SetBackgroundColour(pBgTemp->GetBackgroundColour());
}

void frmFileSelector::FindAndSelect(wxString sFile)
{
	wxTreeItemId oParent = m_pTree->GetRootItem();

	wxTreeItemIdValue oCookie;
	wxTreeItemId oChild = m_pTree->GetFirstChild(oParent, oCookie);

	wxString sPart = sFile.BeforeFirst('\\');
	bool bMoveNext = true;
	while(oChild.IsOk())
	{
		if(m_pTree->GetItemText(oChild).IsSameAs(sPart, false))
		{
			sFile = sFile.AfterFirst('\\');
			sPart = sFile.BeforeFirst('\\');

			m_pTree->EnsureVisible(oChild);

			if(!m_pTree->ItemHasChildren(oChild) || sPart.IsEmpty())
			{
				if(sPart.IsEmpty()) m_pTree->SelectItem(oChild);
				return;
			}
			else
			{
				oParent = oChild;
				if(!m_pTree->IsExpanded(oParent)) m_pTree->Expand(oParent);
				oChild = m_pTree->GetFirstChild(oParent, oCookie);
				bMoveNext = false;
			}
		}
		if(bMoveNext) oChild = m_pTree->GetNextChild(oParent, oCookie);
		else bMoveNext = true;
	}
}

void frmFileSelector::OnTreeExpanding(wxTreeEvent& event)
{
	MakeChildren(event.GetItem());
}

void frmFileSelector::OnTreeSelect(wxTreeEvent& event)
{
	wxTreeItemId& oItem = event.GetItem(), &oRoot = m_pTree->GetRootItem();

	CFileSelectorTreeItemData* pData = (CFileSelectorTreeItemData*)m_pTree->GetItemData(oItem);

	wxString sPath;
	while(oItem != oRoot)
	{
		wxString sPart = m_pTree->GetItemText(oItem);
		if(!sPath.IsEmpty()) sPath.Prepend(wxT("\\"));
		sPath.Prepend(sPart);
		oItem = m_pTree->GetItemParent(oItem);
	}
	if(pData && pData->sMod && pData->sSource) sPath.Append(m_sRgdModeExt);
	m_pText->SetValue(sPath);
}

void frmFileSelector::MakeChildren(const wxTreeItemId& parent)
{
	CFileSelectorTreeItemData* pData = (CFileSelectorTreeItemData*)m_pTree->GetItemData(parent);
	if(!pData || !pData->pToFillWith) return;

	wxString sPreviousName;
	while(pData->pToFillWith->VGetType() != IDirectoryTraverser::IIterator::T_Nothing)
	{
		if(pData->pToFillWith->VGetType() == IDirectoryTraverser::IIterator::T_File)
		{
			wxString sName = AsciiTowxString(pData->pToFillWith->VGetName());
			bool bSkip = false;
			if(m_bRgdMode)
			{
				sName = sName.BeforeLast('.');
				if(sName.IsSameAs(sPreviousName, false)) bSkip = true;
				else sPreviousName = sName;
			}
			if(!bSkip)
			{
				int iExt = sName.Find('.', true), iImg = -2;
				if(iExt != -1)
				{
					wxString sExtension = sName.Mid(iExt);
					if(sExtension.IsSameAs(wxT(".ai"), false)) iImg = 0;
					else if(sExtension.IsSameAs(wxT(".lua"), false)) iImg = 1;
					else if(sExtension.IsSameAs(wxT(".nil"), false)) iImg = 2;
					else if(sExtension.IsSameAs(wxT(".rgd"), false)) iImg = 3;
					else if(sExtension.IsSameAs(wxT(".scar"), false)) iImg = 6;
					else if(sExtension.IsSameAs(wxT(".tga"), false)) iImg = 8;
					else if(sExtension.IsSameAs(wxT(".rgt"), false)) iImg = 9;
					else if(sExtension.IsSameAs(wxT(".dds"), false)) iImg = 10;
					else if(sExtension.IsSameAs(wxT(".bfx"), false)) iImg = 11;
					else if(sExtension.IsSameAs(wxT(".abp"), false)) iImg = 12;
					else if(sExtension.IsSameAs(wxT(".rgm"), false)) iImg = 13;
				}
				wxTreeItemId oChild = m_pTree->AppendItem(parent, sName, iImg, iImg, new CFileSelectorTreeItemData(pData->pToFillWith, false));

				if(strcmp((const char*)pData->pToFillWith->VGetTag(0), TheConstruct->GetModule()->GetFileMapName()) == 0)
				{
					m_pTree->SetItemTextColour(oChild, m_cThisMod);
				}
				else if(frmFiles::_IsEngineFileMapName((const char*)pData->pToFillWith->VGetTag(0), TheConstruct->GetModule()))
				{
					m_pTree->SetItemTextColour(oChild, m_cEngine);
				}
				else
				{
					m_pTree->SetItemTextColour(oChild, m_cOtherMod);
				}
			}
		}
		else
		{
			IDirectoryTraverser::IIterator* pChildren = 0;
			try
			{
				pChildren = pData->pToFillWith->VOpenSubDir();
				if(pChildren && pChildren->VGetType() == IDirectoryTraverser::IIterator::T_Nothing)
				{
					delete pChildren;
					pChildren = 0;
				}
			}
			catch(CRainmanException *pE)
			{
				pE->destroy();
				pChildren = 0;
			}
			wxTreeItemId oChild = m_pTree->AppendItem(parent, AsciiTowxString(pData->pToFillWith->VGetName()), 4, 4, new CFileSelectorTreeItemData(pChildren, true));
			m_pTree->SetItemImage(oChild, 5, wxTreeItemIcon_Expanded);
			if(pChildren) m_pTree->SetItemHasChildren(oChild, true);
		}

		try
		{
			if(pData->pToFillWith->VNextItem() != IDirectoryTraverser::IIterator::E_OK) break;
		}
		catch(CRainmanException *pE)
		{
			pE->destroy();
			break;
		}
	}

	g_vIteratorsToFree.remove(pData->pToFillWith);
	delete pData->pToFillWith;
	pData->pToFillWith = 0;
}

void frmFileSelector::OnSize(wxSizeEvent& event)
{  UNUSED(event);
	Layout();
}

void frmFileSelector::OnOkClick(wxCommandEvent& event)
{  UNUSED(event);
	m_sFile = m_pText->GetValue();
	EndModal(wxID_OK);
}

void frmFileSelector::OnCloseClick(wxCommandEvent& event)
{  UNUSED(event);
	m_sFile = wxT("");
	EndModal(wxID_CANCEL);
}