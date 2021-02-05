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

#include "Construct.h"
#include "frmMassExtract.h"
#include "strings.h"
#include "strconv.h"
#include "Utility.h"
#include <memory>
#include "Common.h"

BEGIN_EVENT_TABLE(frmMassExtract, wxDialog)
	EVT_BUTTON(IDC_Go, frmMassExtract::OnGoClick)
	EVT_BUTTON(IDC_Cancel, frmMassExtract::OnCancelClick)
	EVT_BUTTON(IDC_Advanced, frmMassExtract::OnAdvancedClick)
	EVT_BUTTON(IDC_SelectAll, frmMassExtract::OnSelectClick)
END_EVENT_TABLE()

class CExtractAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const;
	virtual wxString VGetAction() const;
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile);
	static void DoExtract(char* saFile, char* pBuffer);
};

frmMassExtract::_tSrc::_tSrc(const char* sM, const char* sS) : sMod(sM), sSrc(sS) {}

void frmMassExtract::_FillCheckList(CModuleFile* pMod, bool bIsRoot, wxArrayString& sList, std::vector<_tSrc>& vList)
{
	// Archives
	size_t iC = pMod->GetArchiveCount();
	wxString sMod = AsciiTowxString(pMod->GetFileMapName());
	for(size_t i = 0; i < iC; ++i)
	{
		CModuleFile::CArchiveHandler* pArch = pMod->GetArchive(i);
		sList.Add( sMod + wxT("\'s ") + AsciiTowxString(pArch->GetFileName()) );
		vList.push_back(_tSrc(pMod->GetFileMapName(), pArch->GetFileName()));
	}

	// Data folders
	if(!bIsRoot)
	{
		iC = pMod->GetFolderCount();
		for(size_t i = 0; i < iC; ++i)
		{
			CModuleFile::CFolderHandler* pFold = pMod->GetFolder(i);
			sList.Add( sMod + wxT("\'s ") + AsciiTowxString(pFold->GetName()) + wxT(" folder") );
			vList.push_back(_tSrc(pMod->GetFileMapName(), pFold->GetName()));
		}
		sList.Add( sMod + wxT("\'s Data Generic folder"));
		vList.push_back(_tSrc(pMod->GetFileMapName(), "Data Generic"));
	}

	// Data sources
	iC = pMod->GetDataSourceCount();
	for(size_t i = 0; i < iC; ++i)
	{
		CModuleFile::CCohDataSource* pSrc = pMod->GetDataSource(i);

		if(pSrc->IsLoaded())
		{
			size_t iC2 = pSrc->GetArchiveCount();
			for(size_t i = 0; i < iC2; ++i)
			{
				CModuleFile::CArchiveHandler* pArch = pSrc->GetArchive(i);
				sList.Add( AsciiTowxString(pArch->GetFileName()) );
				vList.push_back(_tSrc(0, pArch->GetFileName()));
			}
			
			if( (pSrc->IsFolderWritable() == false) && pSrc->GetFolder() && *pSrc->GetFolder())
			{
				sList.Add(  AsciiTowxString(pSrc->GetFolder()) + wxT(" folder") );
				vList.push_back(_tSrc(0, pSrc->GetFolder()));
			}
		}
	}

	if(bIsRoot)
	{
		// Requireds
		iC = pMod->GetRequiredCount();
		for(size_t i = 0; i < iC; ++i)
		{
			CModuleFile::CRequiredHandler* pReq = pMod->GetRequired(i);
			_FillCheckList(pReq->GetModHandle(), false, sList, m_vSrcs);
		}

		// Engine
		iC = pMod->GetEngineCount();
		for(size_t i = 0; i < iC; ++i)
		{
			_FillCheckList(pMod->GetEngine(i), false, sList, m_vSrcs);
		}
	}
}

size_t frmMassExtract::_DoExtract(wxTreeCtrl* pTree, wxTreeItemId& oFolder, wxString sPath, CModuleFile* pModule, bool bCountOnly, size_t iCountBase, size_t iCountDiv)
{
	int iGVal = (int)(iCountBase / iCountDiv);
	size_t iCount = 0;
	wxTreeItemIdValue oCookie;
	wxTreeItemId oChild = pTree->GetFirstChild(oFolder, oCookie);
	while(oChild.IsOk())
	{
		CFilesTreeItemData *pData = (CFilesTreeItemData*)pTree->GetItemData(oChild);
		if(pData && pData->sMod)
		{
			// File
			//const char* sTmp = strrchr(pData->sSource, '.');
			//bool bSga = (sTmp && (stricmp(sTmp, ".sga") == 0));
			bool bExtract = false;
			for(std::vector<_tSrc>::iterator itr = m_vActiveSrcs.begin(); itr != m_vActiveSrcs.end(); ++itr)
			{
				if((itr->sMod == 0 || (stricmp(pData->sMod, itr->sMod) == 0)) && stricmp(pData->sSource, itr->sSrc) == 0)
				{
					bExtract = true;
					break;
				}
			}

			if(bExtract)
			{
				++iCount;
				if(!bCountOnly)
				{
					wxString sFile = sPath;
					if(sPath.Len()) sFile.Append('\\');
					sFile.Append(pTree->GetItemText(oChild));
					char* saFile = wxStringToAscii(sFile);
					try
					{
						CExtractAction::DoExtract(saFile, m_p4mbBuffer);
					}
					catch(CRainmanException *pE)
					{
						ErrorBoxE(pE);
					}
					int iNGVal = (int)((iCountBase + iCount) / iCountDiv);
					if(iNGVal != iGVal)
					{
						m_pGauge->SetValue(iNGVal);
						m_pGauge->Refresh();
						iGVal = iNGVal;
					}
					delete[] saFile;
				}
			}
		}
		else
		{
			wxString sNewPath = sPath;
			if(sPath.Len()) sNewPath.Append('\\');
			sNewPath.Append(pTree->GetItemText(oChild));
			// Folder
			iCount += _DoExtract(pTree, oChild, sNewPath, pModule, bCountOnly, iCountBase + iCount, iCountDiv);
		}
		
		oChild = pTree->GetNextChild(oChild, oCookie);
	}

	if(!bCountOnly && sPath.Len())
	{
		char* saPath = wxStringToAscii(sPath);
		IDirectoryTraverser::IIterator *pDir = pModule->VIterate(saPath);
		delete[] saPath;
		if(m_bForceUpdate || pTree->IsExpanded(oFolder)) TheConstruct->GetFilesList()->UpdateDirectoryChildren(oFolder, pDir);
		delete pDir;
	}
	return iCount;
}

frmMassExtract::frmMassExtract(wxString sFile, wxTreeItemId& oFolder, bool bForceUpdate)
	:m_oFolder(oFolder), m_sPath(sFile), wxDialog(wxTheApp->GetTopWindow(), -1, AppStr(massext_title), wxPoint(0, 0) , wxDefaultSize, wxFRAME_FLOAT_ON_PARENT | wxFRAME_TOOL_WINDOW | wxCAPTION)
{
	CentreOnParent();
	wxBoxSizer *pTopSizer = new wxBoxSizer( wxVERTICAL );

	wxArrayString oStrs;
	_FillCheckList(TheConstruct->GetModule(), true, oStrs, m_vSrcs);

	pTopSizer->Add(m_pCaption = new wxStaticText(this, -1, AppStr(massext_caption)), 0, wxALIGN_LEFT | wxALL, 3);
	pTopSizer->Add(m_pGauge = new wxGauge(this, -1, 1, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL | wxGA_SMOOTH), 0, wxEXPAND | wxALL, 3);

	wxBoxSizer *pButtonSizer = new wxBoxSizer( wxHORIZONTAL );
	pButtonSizer->Add(m_pSelectAllBtn = new wxButton(this, IDC_SelectAll, AppStr(massext_selectall)), 0, wxFIXED_MINSIZE | wxALL, 3);
	pButtonSizer->Add(m_pAdvancedBtn = new wxButton(this, IDC_Advanced, AppStr(massext_advanced_show)), 0, wxFIXED_MINSIZE | wxALL, 3);
	pButtonSizer->Add(new wxButton(this, IDC_Cancel, AppStr(massext_cancel)), 0, wxFIXED_MINSIZE | wxALL, 3);
	pButtonSizer->Add(new wxButton(this, IDC_Go, AppStr(massext_go)), 0, wxFIXED_MINSIZE | wxALL, 3);

	pTopSizer->Add(pButtonSizer, 0, wxALIGN_RIGHT);

	pTopSizer->Add(m_pCheckList = new wxCheckListBox(this, -1, wxDefaultPosition, wxDefaultSize, oStrs), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 3);

	SetSizer(pTopSizer);
	pTopSizer->SetSizeHints( this );

	m_bAdvancedVisible = false;

	for(size_t i = 0; i < m_vSrcs.size(); ++i)
	{
		m_pCheckList->Check( (unsigned int)i, true);
	}
	m_pCheckList->Show(m_bAdvancedVisible);
	m_pSelectAllBtn->Show(m_bAdvancedVisible);

	m_bForceUpdate = bForceUpdate;
}

void frmMassExtract::OnGoClick(wxCommandEvent& event)
{ UNUSED(event);
	void* pSource = 0;
	if(TheConstruct->GetModule()->IsFauxModule())
	{
		wxString sSaveDirectory = wxDirSelector(wxT("Choose where to extract folder"));
		if(sSaveDirectory.IsEmpty())
			return;

		CModuleFile *pModule = TheConstruct->GetModule();
		pSource = pModule->GetFileMap()->RegisterSource(0, false, 0, "", "", pModule->GetFileSystem(), pModule->GetFileSystem(), true, true, true);

		const char* saPath = wxStringToAscii(sSaveDirectory);
		std::auto_ptr<IDirectoryTraverser::IIterator> pItr (pModule->GetFileSystem()->VIterate(saPath));
		delete[] saPath;

		const char* saPathVirtual = wxStringToAscii(m_sPath);
		pModule->GetFileMap()->MapIteratorDeep(pSource, saPathVirtual, &*pItr);
		delete[] saPathVirtual;
	}

	for(size_t i = 0; i < m_vSrcs.size(); ++i)
	{
		if(m_pCheckList->IsChecked( (unsigned int)i))
		{
			m_vActiveSrcs.push_back(m_vSrcs[i]);
		}
	}

	size_t iFileCount = _DoExtract(TheConstruct->GetFilesList()->GetTree(), m_oFolder, m_sPath, TheConstruct->GetModule(), true, 0, 1);
	size_t iDiv = 1;
	while((iFileCount / iDiv) > (size_t)0xFF)
	{
		++iDiv;
	}
	m_pGauge->SetRange((int)(iFileCount / iDiv)); // Won't overflow (see above code)
	m_p4mbBuffer = new char[4194304];
	_DoExtract(TheConstruct->GetFilesList()->GetTree(), m_oFolder, m_sPath, TheConstruct->GetModule(), false, 0, iDiv);
	delete[] m_p4mbBuffer;

	if(TheConstruct->GetModule()->IsFauxModule())
	{
		TheConstruct->GetModule()->GetFileMap()->EraseSource(pSource);
	}

	wxMessageBox(AppStr(massext_done),AppStr(massext_title),wxICON_INFORMATION,this);
	EndModal(wxID_CLOSE);
}

void frmMassExtract::OnCancelClick(wxCommandEvent& event)
{ UNUSED(event);
	EndModal(wxID_CLOSE);
}

void frmMassExtract::OnAdvancedClick(wxCommandEvent& event)
{ UNUSED(event);
	m_bAdvancedVisible = !m_bAdvancedVisible;
	m_pCheckList->Show(m_bAdvancedVisible);
	m_pSelectAllBtn->Show(m_bAdvancedVisible);

	if(m_bAdvancedVisible)
	{
		m_pCaption->SetLabel(AppStr(massext_caption_adv));
		m_pAdvancedBtn->SetLabel(AppStr(massext_advanced_hide));
	}
	else
	{
		for(size_t i = 0; i < m_vSrcs.size(); ++i)
		{
			if(!m_pCheckList->IsChecked( (unsigned int)i)) goto not_all_checked;
		}
		m_pCaption->SetLabel(AppStr(massext_caption));
not_all_checked:
		m_pAdvancedBtn->SetLabel(AppStr(massext_advanced_show));
	}
}

void frmMassExtract::OnSelectClick(wxCommandEvent& event)
{ UNUSED(event);
	if(m_vSrcs.size())
	{
		bool bSetTo = !m_pCheckList->IsChecked(0);
		for(size_t i = 0; i < m_vSrcs.size(); ++i)
		{
			m_pCheckList->Check( (unsigned int)i, bSetTo);
		}
	}
}