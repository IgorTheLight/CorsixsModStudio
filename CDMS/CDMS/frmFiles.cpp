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

#include "frmFiles.h"

#include "frmImage.h"
#include "frmRgdEditor.h"
#include "frmScarEditor.h"
#include "frmRgmMaterialEditor.h"
#include "frmMassExtract.h"
#include "frmRgdMacro.h"
#include "frmMessage.h"
#include "Construct.h"
#include "strconv.h"
#include "strings.h"
#include "config.h"
#include "Utility.h"
#include "../../Rainman/src/zLib/zlib.h"
extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

#include <wx/progdlg.h>
#include <wx/clipbrd.h>
#include "Common.h"

BEGIN_EVENT_TABLE(frmFiles, wxWindow)
	EVT_SIZE(frmFiles::OnSize)
	EVT_TREE_ITEM_GETTOOLTIP(IDC_FilesTree, frmFiles::OnNodeTooltip)
	EVT_LIST_ITEM_ACTIVATED(IDC_ToolsList, frmFiles::LaunchTool)
	EVT_TREE_ITEM_ACTIVATED(IDC_FilesTree, frmFiles::OnNodeActivate)
	EVT_TREE_ITEM_MENU(IDC_FilesTree, frmFiles::OnNodeRightClick)
	EVT_MENU(wxID_ANY, frmFiles::OnMenu)
	EVT_AUINOTEBOOK_PAGE_CHANGED(wxID_ANY, frmFiles::OnPageChange)
END_EVENT_TABLE()

extern "C" {
typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned       char ub1;
ub4 hash(ub1 * k,ub4 length,ub4 initval);
ub4 hash3(ub1 * k,ub4 length,ub4 initval);
}

CFilesTreeItemData::CFilesTreeItemData(IDirectoryTraverser::IIterator* pItr)
{
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

// Handlers

#include "frmFiles_Actions.h"

// Proper stuff

wxTreeItemId frmFiles::FindFile(wxString sFile, bool bParent)
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

			if(!m_pTree->ItemHasChildren(oChild) || sPart.IsEmpty())
			{
				if(sPart.IsEmpty()) m_pTree->SelectItem(oChild);
				return bParent ? oParent : oChild;
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

	return oParent;
}

wxTreeCtrl* frmFiles::GetTree()
{
	return m_pTree;
}

bool frmFiles::_IsEngineFileMapName(const char* sName, CModuleFile* pMod)
{
	size_t iC = pMod->GetEngineCount();
	for(size_t i = 0; i < iC; ++i)
	{
		if( strcmp(pMod->GetEngine(i)->GetFileMapName(), sName) == 0) return true;
	}
	return false;
}

void frmFiles::OnPageChange(wxAuiNotebookEvent& event)
{
	int iNew = event.GetSelection();

	if(iNew == 2) // LUA
	{
		m_pLuaTree->OnActivated();
	}
}

bool frmFiles::UpdateDirectoryChildren(wxTreeItemId& oDirectory, IDirectoryTraverser::IIterator* pChildren)
{
	char *sTreeItem;
	const char *sDirItem;
	wxTreeItemIdValue oTreeCookie;
	bool bGotOld = false;

	m_pTree->Freeze();
	wxTreeItemId oTreeItem = m_pTree->GetFirstChild(oDirectory, oTreeCookie), oTheOldItem;
	do
	{
		sTreeItem = oTreeItem.IsOk() ? wxStringToAscii(m_pTree->GetItemText(oTreeItem)) : 0;
		sDirItem = pChildren->VGetType() == IDirectoryTraverser::IIterator::T_Nothing ? 0 : pChildren->VGetName();

		if(sTreeItem && sDirItem)
		{
			// Items on both
			int i = stricmp(sTreeItem, sDirItem);
			if(i < 0)
			{
				// Tree item before dir (delete item)
				wxTreeItemId oOld = oTreeItem;
				oTheOldItem = oTreeItem;
				bGotOld = true;
				oTreeItem = m_pTree->GetNextChild(oOld, oTreeCookie);
				m_pTree->Delete(oOld);
			}
			else if(i == 0)
			{
				CFilesTreeItemData *pData = (CFilesTreeItemData*)m_pTree->GetItemData(oTreeItem);
				if(pData)
				{
					if(pChildren->VGetType() == IDirectoryTraverser::IIterator::T_File)
					{
						pData->sMod = (const char*) pChildren->VGetTag(0);
						pData->sSource = (const char*) pChildren->VGetTag(1);
					}
				}
				
				if(pChildren->VGetType() == IDirectoryTraverser::IIterator::T_File)
				{
					if(strcmp((const char*)pChildren->VGetTag(0), TheConstruct->GetModule()->GetFileMapName()) == 0)
					{
						// This mod
						m_pTree->SetItemTextColour(oTreeItem, m_cThisMod);
					}
					else if(_IsEngineFileMapName((const char*)pChildren->VGetTag(0), TheConstruct->GetModule()))
					{
						// Engine
						m_pTree->SetItemTextColour(oTreeItem, m_cEngine);
					}
					else
					{
						// other mod
						m_pTree->SetItemTextColour(oTreeItem, m_cOtherMod);
					}
				}
				oTheOldItem = oTreeItem;
				bGotOld = true;
				oTreeItem = m_pTree->GetNextChild(oTreeItem, oTreeCookie);
				pChildren->VNextItem();
			}
			else
			{
				// Tree item after dir (new item)
				bool bRet = _FillPartialFromIDirectoryTraverserIIterator(oDirectory, pChildren, 0, true, bGotOld ? &oTheOldItem : 0);
				if(!bRet)
				{
					m_pTree->Thaw();
					m_pTree->Update();
					delete[] sTreeItem;
					return false;
				}
				pChildren->VNextItem();
			}
		}
		else if(sTreeItem)
		{
			// Item in tree only
			wxTreeItemId oOld = oTreeItem;
			oTheOldItem = oTreeItem;
			bGotOld = true;
			oTreeItem = m_pTree->GetNextChild(oOld, oTreeCookie);
			m_pTree->Delete(oOld);
		}
		else if(sDirItem)
		{
			// Item in dir only
			bool bRet = _FillPartialFromIDirectoryTraverserIIterator(oDirectory, pChildren, 0);
			delete[] sTreeItem;
			m_pTree->Thaw();
			m_pTree->Update();
			return bRet;
		}

		delete[] sTreeItem;
	} while(sTreeItem && sDirItem);

	m_pTree->Thaw();
	m_pTree->Update();
	return true;
}

void frmFiles::AddHandler(IHandler* pHandler)
{
	m_vHandlers.push_back(pHandler);
}

void frmFiles::AddFolderHandler(IHandler* pHandler)
{
	m_vFolderHandlers.push_back(pHandler);
}

frmFiles::~frmFiles()
{
	for(std::vector<IHandler*>::iterator itr = m_vHandlers.begin(); itr != m_vHandlers.end(); ++itr)
	{
		delete *itr;
	}
	for(std::vector<IHandler*>::iterator itr = m_vFolderHandlers.begin(); itr != m_vFolderHandlers.end(); ++itr)
	{
		delete *itr;
	}
}

void frmFiles::LaunchTool(wxListEvent& event)
{
	TheConstruct->DoTool(event.GetText());
}

void frmFiles::OnNodeTooltip(wxTreeEvent& event)
{
	CFilesTreeItemData *pData = (CFilesTreeItemData*)m_pTree->GetItemData(event.GetItem());
	if(pData)
	{
		if(pData->sMod && pData->sSource)
		{
			wxString sTooltip;
			wxTreeItemId oID = event.GetItem();
			sTooltip = wxT("");//m_pTree->GetItemText(oID);
			//sTooltip.Append(wxT(" | "));
			sTooltip.Append(AppStr(file_modname)).Append(AsciiTowxString(pData->sMod)).Append(wxT(" | "));
			sTooltip.Append(AppStr(file_sourcename)).Append(AsciiTowxString(pData->sSource));

			//SetToolTip( 0 );
			event.SetToolTip( sTooltip );
			TheConstruct->GetStatusBar()->SetStatusText(sTooltip);
		}
		else
		{
			event.SetToolTip( wxT("") );
			TheConstruct->GetStatusBar()->SetStatusText(wxT(""));
		}
	}
}

void frmFiles::OnNodeActivate(wxTreeEvent& event)
{
	CFilesTreeItemData *pData = (CFilesTreeItemData*)m_pTree->GetItemData(event.GetItem());
	if(pData && pData->sMod && pData->sSource)
	{
		wxString sPath, sExt;
		wxTreeItemId oCurrent, oRoot, oParent;
		oCurrent = event.GetItem();
		oParent = m_pTree->GetItemParent(oCurrent);
		oRoot = m_pTree->GetRootItem();
		while(oCurrent != oRoot)
		{
			sPath.Prepend(m_pTree->GetItemText(oCurrent));
			sPath.Prepend(wxT("\\"));
			oCurrent = m_pTree->GetItemParent(oCurrent);
		}
		
		sPath.Remove(0, 1);
		sExt = sPath.AfterLast('.');
		sExt.LowerCase();
		for(std::vector<IHandler*>::iterator itr = m_vHandlers.begin(); itr != m_vHandlers.end(); ++itr)
		{
			if((*itr)->VGetExt() == sExt)
			{
				try
				{
					(*itr)->VHandle(sPath, oParent, oCurrent);
				}
				catch(CRainmanException *pE)
				{
					_ErrorBox(pE, __FILE__, __LINE__, false);
				}
				return;
			}
		}
		// If there is no specilized handler, show a list of generic ones
		OnNodeRightClick(event);
	}
}

void frmFiles::OnNodeRightClick(wxTreeEvent& event)
{
	int flags;
    wxTreeItemId oItemID = m_pTree->HitTest(event.GetPoint(), flags);
	if(oItemID.IsOk())
		m_pTree->SelectItem(oItemID);
	else
		return;

	CFilesTreeItemData *pData = (CFilesTreeItemData*)m_pTree->GetItemData(oItemID);

	wxString sPath, sExt;
	wxTreeItemId oCurrent, oRoot, oParent;
	oCurrent = oItemID;
	m_oPopupTreeItem = oCurrent;
	oParent = m_pTree->GetItemParent(oCurrent);
	oRoot = m_pTree->GetRootItem();
	while(oCurrent != oRoot)
	{
		sPath.Prepend(m_pTree->GetItemText(oCurrent));
		sPath.Prepend(wxT("\\"));
		oCurrent = m_pTree->GetItemParent(oCurrent);
	}
	
	wxMenu *pPopup = new wxMenu;

	sPath.Remove(0, 1);
	if(pData && pData->sSource)
	{
		sExt = sPath.AfterLast('.');
		sExt.LowerCase();
		int i = 0;
		for(std::vector<IHandler*>::iterator itr = m_vHandlers.begin(); itr != m_vHandlers.end(); ++itr)
		{
			if(((*itr)->VGetExt() == sExt) || ((*itr)->VGetExt() == wxT("")))
			{
				pPopup->Append(wxID_HIGHEST + 1337 + i, (*itr)->VGetAction());
			}
			++i;
		}

		m_sPopupFileName = sPath;
		m_oPopupTreeParent = oParent;
		m_bPopupIsFolder = false;
	}
	else
	{
		int i = 0;
		for(std::vector<IHandler*>::iterator itr = m_vFolderHandlers.begin(); itr != m_vFolderHandlers.end(); ++itr)
		{
			pPopup->Append(wxID_HIGHEST + 1337 + i, (*itr)->VGetAction());
			++i;
		}

		m_sPopupFileName = sPath;
		m_oPopupTreeParent = m_oPopupTreeItem;
		m_bPopupIsFolder = true;
	}
	PopupMenu(pPopup);
	delete pPopup;
}

void frmFiles::OnMenu(wxCommandEvent& event)
{
	if(event.GetId() >= (wxID_HIGHEST + 1337))
	{
		try
		{
			if(m_bPopupIsFolder)
				m_vFolderHandlers[event.GetId() - (wxID_HIGHEST + 1337)]->VHandle(m_sPopupFileName, m_oPopupTreeParent, m_oPopupTreeItem);
			else
				m_vHandlers[event.GetId() - (wxID_HIGHEST + 1337)]->VHandle(m_sPopupFileName, m_oPopupTreeParent, m_oPopupTreeItem);
		}
		catch(CRainmanException* pE)
		{
			ErrorBoxE(pE);
		}
	}
}

frmFiles::frmFiles(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
	: wxWindow(parent, id, pos, size)
{
	wxBoxSizer *pTopSizer = new wxBoxSizer( wxVERTICAL );
	m_pTabs = new wxAuiNotebook(this, -1, wxPoint(0,0), wxDefaultSize, (wxAUI_NB_DEFAULT_STYLE | wxNO_BORDER) & (~(wxAUI_NB_CLOSE_BUTTON | wxAUI_NB_CLOSE_ON_ACTIVE_TAB | wxAUI_NB_CLOSE_ON_ALL_TABS | wxAUI_NB_TAB_EXTERNAL_MOVE | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE)) );

	m_pTabs->AddPage(m_pTree = new wxTreeCtrl(m_pTabs, IDC_FilesTree, wxDefaultPosition, wxDefaultSize, wxTR_HIDE_ROOT | wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT), AppStr(file_filestabname));
	m_pTabs->AddPage(m_pTools = new wxListCtrl(m_pTabs, IDC_ToolsList, wxDefaultPosition, wxDefaultSize, wxLC_ICON | wxLC_SINGLE_SEL), AppStr(file_toolstabname));

	m_pTabs->AddPage(m_pLuaTree = new frmLuaInheritTree(m_pTabs, wxID_ANY), wxT("Lua"));

	m_pTools->SetAutoLayout(true);
	pTopSizer->Add(m_pTabs, 1, wxEXPAND);

	wxImageList* pToolImages = new wxImageList(48, 48);
	size_t iToolCount = TheConstruct->GetToolCount();
	for(size_t i = 0; i < iToolCount; ++i)
	{
		pToolImages->Add(wxBitmap(TheConstruct->GetTool(i)->GetBitmapName(), wxBITMAP_TYPE_BMP_RESOURCE), wxColour(255, 0, 255));
	}
	/*
	pToolImages->Add(wxBitmap(wxT("IDB_TOOL_LOC"), wxBITMAP_TYPE_BMP_RESOURCE), wxColour(255, 0, 255)); // ( 0) Locilization
	pToolImages->Add(wxBitmap(wxT("IDB_TOOL_UCS"), wxBITMAP_TYPE_BMP_RESOURCE), wxColour(255, 0, 255)); // ( 1) UCS Editor
	pToolImages->Add(wxBitmap(wxT("IDB_SNAPSHOT"), wxBITMAP_TYPE_BMP_RESOURCE), wxColour(255, 0, 255)); // ( 2) Attrib Snapshot
	pToolImages->Add(wxBitmap(wxT("IDB_SGAPACK"), wxBITMAP_TYPE_BMP_RESOURCE), wxColour(255, 0, 255)); // ( 3) SGA Pack
	pToolImages->Add(wxBitmap(wxT("IDB_TOOL_EXTALL"), wxBITMAP_TYPE_BMP_RESOURCE), wxColour(255, 0, 255)); // ( 4) Extract all
	pToolImages->Add(wxBitmap(wxT("IDB_TOOL_CALCULATOR"), wxBITMAP_TYPE_BMP_RESOURCE), wxColour(255, 0, 255)); // ( 5) DPS Calculator
	pToolImages->Add(wxBitmap(wxT("IDB_REDBUTTON"), wxBITMAP_TYPE_BMP_RESOURCE), wxColour(255, 0, 255)); // ( 6) Red button :D
	*/

	m_pTools->AssignImageList(pToolImages, wxIMAGE_LIST_NORMAL);
	m_pTools->Arrange(wxLIST_ALIGN_SNAP_TO_GRID);

	for(size_t i = 0; i < iToolCount; ++i)
	{
		m_pTools->InsertItem(i, TheConstruct->GetTool(i)->GetName(), i);
	}

	/*
	m_pTools->InsertItem(0, AppStr(locale), 0);
	m_pTools->InsertItem(1, AppStr(ucs_editor), 1);
	m_pTools->InsertItem(2, AppStr(xml_export), 2);
	m_pTools->InsertItem(3, AppStr(sgapack_title), 3);
	m_pTools->InsertItem(4, AppStr(massext_toolname), 4);
	m_pTools->InsertItem(5, AppStr(dpscalculator_title), 5);
	m_pTools->InsertItem(6, AppStr(redbutton_toolname), 6);
	*/

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
	
	SetSizer(pTopSizer);
	pTopSizer->SetSizeHints( this );

	// Specific actions
	AddHandler(new CRGMMaterialAction);
	AddHandler(new CNilAction);
	AddHandler(new CLuaAction);
	AddHandler(new CLuaBurnAction);
	AddHandler(new CRGDAction);
	AddHandler(new CRgdToLuaDumpAction);
	AddHandler(new CScarAction);
	AddHandler(new CAiAction);
	AddHandler(new CMuaxAction);
	AddHandler(new CSuaAction);
	AddHandler(new CMuaAction);
	AddHandler(new CAbpAction);
	AddHandler(new CBfxAction);
	AddHandler(new CBfxToLuaDumpAction);
	AddHandler(new CRgtViewAction);
	AddHandler(new CRgtToGenericAction);
	AddHandler(new CDdsViewAction);
	AddHandler(new CDdsToRgtAction);
	AddHandler(new CTgaViewAction);
	AddHandler(new CRGODeBurnAction);
	// Generic actions
	AddHandler(new CMakeCopyAction);
	AddHandler(new CExtractAction);
	AddHandler(new CFilePathCopyAction);
	AddHandler(new CTextViewAction);

	// Folder actions
	AddFolderHandler(new CExtractFolderAction);
	AddFolderHandler(new CLuaBurnFolderAction);
	AddFolderHandler(new CLuaBurnFolderIncReqAction);
	AddFolderHandler(new CRgdMacroAction);
	AddFolderHandler(new CLuaDumpFolder);
	AddFolderHandler(new CBFXRGTDeBurnAction);
	AddFolderHandler(new CScanHashesAction);
}

void frmFiles::OnSize(wxSizeEvent& event)
{ UNUSED(event);
	Layout();
	m_pTools->DeleteAllItems();
	size_t iToolCount = TheConstruct->GetToolCount();
	for(size_t i = 0; i < iToolCount; ++i)
	{
		m_pTools->InsertItem(i, TheConstruct->GetTool(i)->GetName(), i);
	}
}

bool frmFiles::FillFromIDirectoryTraverser(IDirectoryTraverser* pTraverser)
{
	m_cThisMod = ConfGetColour(AppStr(file_colourthismod), 128, 64, 64);
	m_cOtherMod = ConfGetColour(AppStr(file_colourothermod), 64, 64, 128);
	m_cEngine = ConfGetColour(AppStr(file_colourengine), 128, 128, 128);

	m_pTree->DeleteAllItems();
	wxTreeItemId oRoot = m_pTree->AddRoot(wxT(""));

	unsigned long iEntryCount = pTraverser->VGetEntryPointCount();
	for(unsigned long iEntryPoint = 0; iEntryPoint < iEntryCount; ++iEntryPoint)
	{
		const char* sEntryPoint = pTraverser->VGetEntryPoint(iEntryPoint);
		wxTreeItemId oEntry = m_pTree->AppendItem(oRoot, AsciiTowxString(sEntryPoint), 7);

		IDirectoryTraverser::IIterator* pEntry = pTraverser->VIterate(sEntryPoint);
		if(pEntry)
		{
			if(!_FillPartialFromIDirectoryTraverserIIterator(oEntry, pEntry, 0))
			{
				delete pEntry;
				return false;
			}
			delete pEntry;
		}
	}
	return true;
}

bool frmFiles::_FillPartialFromIDirectoryTraverserIIterator(wxTreeItemId oParent, IDirectoryTraverser::IIterator* pChildren, int iExpandLevel, bool bOnlyOne, wxTreeItemId *pAfter)
{
	if(!pChildren) return false;

	if(pChildren->VGetType() != IDirectoryTraverser::IIterator::T_Nothing)
	{
		IDirectoryTraverser::IIterator::eErrors eNextError;
		do
		{
			switch(pChildren->VGetType())
			{
			case IDirectoryTraverser::IIterator::T_File:
				{
				wxString sName = AsciiTowxString(pChildren->VGetName());
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
				wxTreeItemId oChild;
				if(pAfter)
					oChild = m_pTree->InsertItem(oParent, *pAfter, sName, iImg, iImg, new CFilesTreeItemData(pChildren));
				else
				{
					if(bOnlyOne) oChild = m_pTree->InsertItem(oParent, 0, sName, iImg, iImg, new CFilesTreeItemData(pChildren));
					else oChild = m_pTree->AppendItem(oParent, sName, iImg, iImg, new CFilesTreeItemData(pChildren));
				}
				//! \todo pull file colours out of config file
				if(strcmp((const char*)pChildren->VGetTag(0), TheConstruct->GetModule()->GetFileMapName()) == 0)
				{
					// This mod
					m_pTree->SetItemTextColour(oChild, m_cThisMod);
				}
				else if(_IsEngineFileMapName((const char*)pChildren->VGetTag(0), TheConstruct->GetModule()))
				{
					// Engine
					m_pTree->SetItemTextColour(oChild, m_cEngine);
				}
				else
				{
					// other mod
					m_pTree->SetItemTextColour(oChild, m_cOtherMod);
				}
				}
				break;
			case IDirectoryTraverser::IIterator::T_Directory:
				{
				wxTreeItemId oChild;
				if(pAfter)
					oChild = m_pTree->InsertItem(oParent, *pAfter, AsciiTowxString(pChildren->VGetName()), 4, 4, new CFilesTreeItemData(pChildren));
				else
					oChild = m_pTree->AppendItem(oParent, AsciiTowxString(pChildren->VGetName()), 4, 4, new CFilesTreeItemData(pChildren));
				m_pTree->SetItemImage(oChild, 5, wxTreeItemIcon_Expanded);
				IDirectoryTraverser::IIterator* pChild = pChildren->VOpenSubDir();
				if(pChild)
				{
					if(!_FillPartialFromIDirectoryTraverserIIterator(oChild, pChild, iExpandLevel? iExpandLevel - 1: 0))
					{
						delete pChild;
						return false;
					}
					if(iExpandLevel) m_pTree->EnsureVisible(oChild);
					delete pChild;
				}
				break;
				}
			};
			if(bOnlyOne) return true;
		}
		while((eNextError = pChildren->VNextItem()) == IDirectoryTraverser::IIterator::E_OK);
	}
	return true;
}