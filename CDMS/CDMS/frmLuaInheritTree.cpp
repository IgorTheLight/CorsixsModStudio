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
#include "frmLuaInheritTree.h"
#include "Tools.h"
#include "frmLoading.h"
#include "frmRgdEditor.h"
#include "Common.h"

BEGIN_EVENT_TABLE(frmLuaInheritTree, wxWindow)
	EVT_SIZE(frmLuaInheritTree::OnSize)
	EVT_TREE_ITEM_GETTOOLTIP(IDC_LuaTree, frmLuaInheritTree::OnTreeTooltip)
	EVT_TREE_ITEM_ACTIVATED(IDC_LuaTree, frmLuaInheritTree::OnTreeActivate)
	EVT_TREE_ITEM_MENU(IDC_LuaTree, frmLuaInheritTree::OnTreeRightClick)
	EVT_TREE_ITEM_EXPANDING(IDC_LuaTree, frmLuaInheritTree::OnTreeExpanding)
END_EVENT_TABLE()

class CLuaTreeItemData : public wxTreeItemData
{
public:
	CInheritTable::CNode* pNode;
	bool bNeedsFilling;

	CLuaTreeItemData(CInheritTable::CNode* pp, bool b = false)
	{pNode = pp;bNeedsFilling = b;}
	~CLuaTreeItemData()
	{
		
	}

};

void frmLuaInheritTree::OnActivated()
{
	if(bFirstActivate)
	{
		if(m_pInheritTable)
		{
			frmLoading* pLoadingForm = new frmLoading(AppStr(mod_loading));
			pLoadingForm->Show(TRUE);
			pLoadingForm->SetMessage(wxString(wxT("Generating LUA inheritance tree")));
			wxSafeYield(pLoadingForm);

			CMakeLuaInheritTree* pTool = new CMakeLuaInheritTree;
			pTool->pTable = m_pInheritTable;
			try
			{
				pTool->Do("Generic\\Attrib\\");
			}
			catch(CRainmanException *pE)
			{
				delete pTool;
				ErrorBoxE(pE);
			}
			delete pTool;

			pLoadingForm->SetMessage(wxString(wxT("Updating GUI")));
			wxSafeYield(pLoadingForm);

			m_pTree->Freeze();
			_AddChildren(m_pTree->GetRootItem(), m_pInheritTable->getRoot());
			m_pTree->Thaw();
			m_pTree->Expand(m_pTree->GetRootItem());

			pLoadingForm->Close(TRUE);
			delete pLoadingForm;
		}

		bFirstActivate = false;
	}
}

void frmLuaInheritTree::_AddChildren(wxTreeItemId& oParent, CInheritTable::CNode* pParent)
{
	size_t iL = pParent->getChildCount();
	for(size_t i = 0; i < iL; ++i)
	{
		CInheritTable::CNode *pChild = pParent->getChild(i);
		int iImg = pChild->getIsNil() ? 1 : 0;
		wxTreeItemId oChild = m_pTree->AppendItem(oParent, AsciiTowxString(pChild->getMiniName()), iImg, iImg, new CLuaTreeItemData(pChild,true));
		//m_pTree->SetItemImage(oChild, iImg, wxTreeItemIcon_Expanded);
		//m_pTree->SetItemImage(oChild, iImg, wxTreeItemIcon_SelectedExpanded);
		//_AddChildren(oChild, pChild);
		m_pTree->SetItemHasChildren(oChild, pChild->getChildCount() > 0);
	}
	if(iL > 0) m_pTree->SortChildren(oParent);
}

frmLuaInheritTree::frmLuaInheritTree(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
: wxWindow(parent, id, pos, size)
{
	bFirstActivate = true;
	m_pInheritTable = 0;

	if(CMakeLuaInheritTree::_DoesExist("Generic\\attrib\\"))
	{
		wxBoxSizer *pTopSizer = new wxBoxSizer( wxVERTICAL );

		m_pInheritTable = new CInheritTable;
		m_pTree = new wxTreeCtrl(this, IDC_LuaTree, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS);

		wxImageList* pFileTypes = new wxImageList(16, 16);
		pFileTypes->Add(wxBitmap(wxT("IDB_FILELUA"), wxBITMAP_TYPE_BMP_RESOURCE)); // ( 0) LUA
		pFileTypes->Add(wxBitmap(wxT("IDB_FILENIL"), wxBITMAP_TYPE_BMP_RESOURCE)); // ( 1) NIL
		pFileTypes->Add(wxBitmap(wxT("IDB_TOC"), wxBITMAP_TYPE_BMP_RESOURCE)); // ( 2) ToC
		m_pTree->AssignImageList(pFileTypes);

		wxTreeItemId oRoot = m_pTree->AddRoot(wxT("Generic"), 2, 2);
		m_pTree->SetItemImage(oRoot, 2, wxTreeItemIcon_Expanded);
		m_pTree->SetItemImage(oRoot, 2, wxTreeItemIcon_SelectedExpanded);

		pTopSizer->Add(m_pTree, 1, wxEXPAND | wxALL, 0);

		SetSizer(pTopSizer);
		pTopSizer->SetSizeHints( this );
	}
	else
	{
	}
}
frmLuaInheritTree::~frmLuaInheritTree()
{
	if(m_pInheritTable) delete m_pInheritTable;
}
void frmLuaInheritTree::OnSize(wxSizeEvent& event)
{ UNUSED(event);
	Layout();
}

static wxString OnlyFilename(wxString sName)
{
	return sName.AfterLast('\\');
}

void frmLuaInheritTree::OnTreeActivate(wxTreeEvent& event)
{
	CLuaTreeItemData *pData = (CLuaTreeItemData*)m_pTree->GetItemData(event.GetItem());
	if(pData)
	{
		/*
		wxString sTooltip;
		sTooltip = AsciiTowxString(pData->pNode->getFullName());
		TheConstruct->GetStatusBar()->SetStatusText(sTooltip);
		*/
		wxString sFile(wxT("Generic\\attrib\\"));
		sFile += AsciiTowxString(pData->pNode->getFullName());
		char* saFile = wxStringToAscii(sFile);
		IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
		if(!pStream)
		{
			delete[] saFile;
			ErrorBox("Cannot open file");
			return;
		}
		CLuaFile2 *pLua = CLuaAction::DoLoad2(pStream, saFile);
		delete[] saFile;
		delete pStream;
		if(pLua)
		{
			frmRGDEditor* pForm;
			TheConstruct->GetTabs()->AddPage(pForm = new frmRGDEditor(TheConstruct->GetFilesList()->FindFile(sFile, true), sFile, TheConstruct->GetTabs(), -1), wxString().Append(wxT("LUA")).Append(wxT(" [")).Append(OnlyFilename(sFile)).Append(wxT("]")), true);
			if(!pForm->FillFromLua2(pLua))
			{
				ErrorBox("Cannot load file");
				delete pLua;
				return;
			}
		}
	}
}
void frmLuaInheritTree::OnTreeTooltip(wxTreeEvent& event)
{
	CLuaTreeItemData *pData = (CLuaTreeItemData*)m_pTree->GetItemData(event.GetItem());
	if(pData)
	{
		wxString sTooltip;
		sTooltip = AsciiTowxString(pData->pNode->getFullName());
		TheConstruct->GetStatusBar()->SetStatusText(sTooltip);
	}
}
void frmLuaInheritTree::OnTreeExpanding(wxTreeEvent& event)
{
	CLuaTreeItemData *pData = (CLuaTreeItemData*)m_pTree->GetItemData(event.GetItem());
	if(pData && pData->bNeedsFilling)
	{
		_AddChildren(event.GetItem(), pData->pNode);
		pData->bNeedsFilling = false;
	}
}
void frmLuaInheritTree::OnTreeRightClick(wxTreeEvent& event)
{  UNUSED(event);
	// TODO
}