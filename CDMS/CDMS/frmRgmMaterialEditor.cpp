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

#include "frmRgmMaterialEditor.h"
#include "strconv.h"
#include "Construct.h"
#include "config.h"
#include "strings.h"
#include "Utility.h"
#include "Common.h"

BEGIN_EVENT_TABLE(frmRgmMaterialEditor, wxWindow)
	EVT_SIZE(frmRgmMaterialEditor::OnSize)
	EVT_PG_CHANGED(IDC_PropertyGrid, frmRgmMaterialEditor::OnPropertyChange)
	EVT_TREE_SEL_CHANGING(IDC_TablesTree, frmRgmMaterialEditor::OnTreeSelect)
	EVT_TREE_ITEM_MENU(IDC_TablesTree, frmRgmMaterialEditor::OnTreeRightClick)
	EVT_BUTTON(wxID_SAVE, frmRgmMaterialEditor::OnSave)
END_EVENT_TABLE()

class CRgmMaterialTreeData : public wxTreeItemData
{
public:
	CRgmMaterialTreeData(CRgmFile::CMaterial* pMat)
		: pMaterial(pMat), pVariable(0) {}

		CRgmMaterialTreeData(CRgmFile::CMaterial::CVariable* pVar)
		: pMaterial(0), pVariable(pVar) {}

	~CRgmMaterialTreeData()
	{

	}

public:
	CRgmFile::CMaterial* pMaterial;
	CRgmFile::CMaterial::CVariable* pVariable;
};

class RgmMaterialwxPropertyGridManager : public wxPropertyGridManager
{
public:
	RgmMaterialwxPropertyGridManager( wxWindow *parent, wxWindowID id = wxID_ANY,
               	           const wxPoint& pos = wxDefaultPosition,
               	           const wxSize& size = wxDefaultSize,
               	           long style = wxPGMAN_DEFAULT_STYLE,
               	           const wxChar* name = wxPropertyGridManagerNameStr )
		: wxPropertyGridManager(parent, id, pos, size, style, name)
	{
	}

	void OnSize(wxSizeEvent& event)
	{
		wxPropertyGridManager::OnSize(event);
		if(m_pTxtHelpCaption)
		{
			m_pTxtHelpCaption->SetBackgroundColour(GetBackgroundColour());
			m_pTxtHelpCaption->SetWindowStyleFlag(m_pTxtHelpCaption->GetWindowStyleFlag() | wxST_NO_AUTORESIZE);
		}
		if(m_pTxtHelpContent)
		{
			m_pTxtHelpContent->SetBackgroundColour(GetBackgroundColour());
		}
	}

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(RgmMaterialwxPropertyGridManager, wxPropertyGridManager)
	EVT_SIZE(RgmMaterialwxPropertyGridManager::OnSize)
END_EVENT_TABLE()

void frmRgmMaterialEditor::OnSave(wxCommandEvent &event)
{ UNUSED(event);
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
		free(saDir);
		delete[] saNewFile;
		return;
	}
	free(saDir);

	if(!saNewFile)
	{
		ErrorBoxAS(err_memory);
		delete pDir;
		return;
	}

	IFileStore::IOutputStream* pStream = 0;
	try
	{
		pStream = TheConstruct->GetModule()->VOpenOutputStream(saNewFile, true);
		TheConstruct->GetFilesList()->UpdateDirectoryChildren(m_oFileParent, pDir);
	}
	catch(CRainmanException *pE)
	{
		ErrorBoxE(pE);
		delete pDir;
		delete pStream;
		delete[] saNewFile;
		return;
	}
	delete pDir;

	if(!pStream)
	{
		ErrorBoxAS(err_couldnotopenoutput);
		delete[] saNewFile;
		return;
	}
	delete[] saNewFile;

	try
	{
		m_pRgmFile->Save(pStream);
	}
	catch(CRainmanException *pE)
	{
		RestoreBackupFile(TheConstruct->GetModule(), m_sFilename);
		ErrorBoxE(pE);
		delete pStream;
		return;
	}
	wxMessageBox(AppStr(rgd_savegood),AppStr(rgd_save),wxICON_INFORMATION,this);

	delete pStream;
}

void frmRgmMaterialEditor::SetObject(CRgmFile* pRgmFile, bool bTakeOwnership)
{
	if(m_bOwnRgm) delete m_pRgmFile;
	m_pRgmFile = pRgmFile;
	m_bOwnRgm = bTakeOwnership;

	_FillLeft();
}

void frmRgmMaterialEditor::_FillLeft()
{
	m_pTables->SetWindowStyle(m_pTables->GetWindowStyle() | wxTR_HIDE_ROOT);
	wxTreeItemId oRoot = m_pTables->AddRoot(wxT(""), -1, -1, 0 );

	size_t iMaterialCount = m_pRgmFile->GetMaterialCount();
	for(size_t i = 0; i < iMaterialCount; ++i)
	{
		CRgmFile::CMaterial* pMaterial = m_pRgmFile->GetMaterial(i);

		wxTreeItemId oMatNode = m_pTables->AppendItem(oRoot, AsciiTowxString(pMaterial->GetName()), -1, -1, new CRgmMaterialTreeData(pMaterial));

		if(i == 0)
		{
			m_pTables->SelectItem(oMatNode);
		}
	}
}

wxPGProperty* frmRgmMaterialEditor::GetVariableEditor(CRgmFile::CMaterial::CVariable* pVar)
{
	wxString sName = AsciiTowxString(pVar->GetName());
	try
	{
		switch(pVar->GetType())
		{
		case CRgmFile::CMaterial::CVariable::VT_Number :
			return wxFloatProperty(sName, sName, pVar->GetValueNumber());

		case CRgmFile::CMaterial::CVariable::VT_Text :
			return wxStringProperty(sName, sName, AsciiTowxString(pVar->GetValueText()));
		}
	}
	catch(CRainmanException *pE)
	{
		pE->destroy();
		return 0;
	}
	wxPGProperty* pTmp = wxStringProperty(sName, sName, wxT("This property cannot be edited"));
	pTmp->SetFlag(wxPG_PROP_DISABLED);
	return pTmp;
}

void frmRgmMaterialEditor::_FillRight(CRgmFile::CMaterial* pMaterial)
{
	m_pPropertyGrid->AppendCategory( wxT("Material") );

	wxPGId oEntry = m_pPropertyGrid->Append(wxStringProperty(wxT("Name"), wxT("Name"), AsciiTowxString(pMaterial->GetName())));
	m_pPropertyGrid->SetPropertyHelpString(oEntry, wxT("Name of this material"));
	m_pPropertyGrid->SetPropertyClientData(oEntry, (void*)0);

	oEntry = m_pPropertyGrid->Append(wxStringProperty(wxT("Shader"), wxT("Shader"), AsciiTowxString(pMaterial->GetDxName())));
	m_pPropertyGrid->SetPropertyHelpString(oEntry, wxT("The shader used by this material (see Data\\shaders folder)"));
	m_pPropertyGrid->SetPropertyClientData(oEntry, (void*)0);

	m_pPropertyGrid->AppendCategory( wxT("Children") );

	size_t iChildren = pMaterial->GetVariableCount();
	for(size_t i = 0; i < iChildren; ++i)
	{
		CRgmFile::CMaterial::CVariable *pVar = pMaterial->GetVariable(i);
		oEntry = m_pPropertyGrid->Append(GetVariableEditor(pVar));
		m_pPropertyGrid->SetPropertyHelpString(oEntry, wxT("No help available"));
		m_pPropertyGrid->SetPropertyClientData(oEntry, (void*)pVar);
	}
}

frmRgmMaterialEditor::frmRgmMaterialEditor(wxTreeItemId& oFileParent, wxString sFilename, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
	: m_oFileParent(oFileParent), m_sFilename(sFilename), wxWindow(parent, id, pos, size)
{
	wxBoxSizer *pTopSizer = new wxBoxSizer( wxVERTICAL );

	pTopSizer->Add(m_pSplitter = new wxSplitterWindow(this, -1), 1, wxEXPAND | wxALL, 0);

	wxBoxSizer *pButtonSizer = new wxBoxSizer( wxHORIZONTAL );
	wxWindow *pBgTemp;
	pButtonSizer->Add(pBgTemp = new wxButton(this, wxID_SAVE, AppStr(rgd_save)), 0, wxEXPAND | wxALL, 3);
	pTopSizer->Add(pButtonSizer, 0, wxALIGN_RIGHT);

	m_pTables = new wxTreeCtrl(m_pSplitter, IDC_TablesTree, wxDefaultPosition, wxDefaultSize, wxTR_EDIT_LABELS | wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_SINGLE);
	m_pPropManager = new RgmMaterialwxPropertyGridManager(m_pSplitter, IDC_PropertyGrid, wxDefaultPosition, wxDefaultSize, wxPG_DEFAULT_STYLE | wxPG_DESCRIPTION);
	m_pPropertyGrid = m_pPropManager->GetGrid(); 

	m_pSplitter->SplitVertically(m_pTables, m_pPropManager);
	m_pSplitter->SetSashGravity(0.0);
	m_pSplitter->SetMinimumPaneSize(48);

	SetBackgroundColour(pBgTemp->GetBackgroundColour());
	SetSizer(pTopSizer);
	pTopSizer->SetSizeHints( this );

	m_pRgmFile = 0;
	m_bOwnRgm = false;
}

frmRgmMaterialEditor::~frmRgmMaterialEditor()
{
	if(m_bOwnRgm) delete m_pRgmFile;
}

void frmRgmMaterialEditor::OnTreeSelect(wxTreeEvent& event)
{
	m_pPropertyGrid->Freeze();
	m_pPropertyGrid->Clear();

	if(!event.GetItem().IsOk())
	{
		m_pPropertyGrid->Thaw();
		m_pPropertyGrid->Refresh();
		return;
	}

	CRgmMaterialTreeData *pData = (CRgmMaterialTreeData*)m_pTables->GetItemData(event.GetItem());
	if(pData)
	{
		if(pData->pMaterial)
		{
			_FillRight(pData->pMaterial);
		}
	}

	m_pPropertyGrid->Thaw();
	m_pPropertyGrid->Refresh();
}

void frmRgmMaterialEditor::OnSize(wxSizeEvent& event)
{ UNUSED(event);
	Layout();
}

void frmRgmMaterialEditor::OnPropertyChange(wxPropertyGridEvent& event)
{
	CRgmFile::CMaterial::CVariable* pVar = (CRgmFile::CMaterial::CVariable*)m_pPropertyGrid->GetPropertyClientData(event.GetProperty());
	if(pVar == 0)
	{
		CRgmMaterialTreeData *pData = (CRgmMaterialTreeData*)m_pTables->GetItemData(m_pTables->GetSelection());
		if(event.GetPropertyName().IsSameAs(wxT("Name")))
		{
			char* sAscii = wxStringToAscii(event.GetPropertyValueAsString());
			pData->pMaterial->SetName(sAscii);
			delete[] sAscii;
			m_pTables->SetItemText(m_pTables->GetSelection(), event.GetPropertyValueAsString());
		}
		else if(event.GetPropertyName().IsSameAs(wxT("Shader")))
		{
			char* sAscii = wxStringToAscii(event.GetPropertyValueAsString());
			pData->pMaterial->SetDxName(sAscii);
			delete[] sAscii;
		}
	}
	else
	{
		switch(pVar->GetType())
		{
		case CRgmFile::CMaterial::CVariable::VT_Text:
			{
				char* sAscii = wxStringToAscii(event.GetPropertyValueAsString());
				pVar->SetValueText(sAscii);
				delete[] sAscii;
				break;
			}
		case CRgmFile::CMaterial::CVariable::VT_Number:
			pVar->SetValueNumber((float) event.GetPropertyValueAsDouble());
			break;
		}
	}
}

void frmRgmMaterialEditor::OnTreeRightClick(wxTreeEvent& event)
{
	int flags;
    wxTreeItemId oItemID = m_pTables->HitTest(event.GetPoint(), flags);
	if(oItemID.IsOk())
		m_pTables->SelectItem(oItemID);
	else
		return;
}