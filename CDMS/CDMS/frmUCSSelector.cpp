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
#include "frmUCSSelector.h"
#include "strings.h"
#include "strconv.h"
#include "Construct.h"
#include <wx/textdlg.h>
#include "Common.h"

BEGIN_EVENT_TABLE(frmUCSSelector, wxDialog)
	EVT_SIZE(frmUCSSelector::OnSize)
	EVT_BUTTON(wxID_NEW, frmUCSSelector::OnNewClick)
	EVT_BUTTON(wxID_OPEN, frmUCSSelector::OnLoadClick)
	EVT_BUTTON(wxID_CLOSE, frmUCSSelector::OnCloseClick)
END_EVENT_TABLE()

void frmUCSSelector::_AddReadOnlyModToList(CModuleFile* pMod, wxArrayString& aList)
{
	if(pMod)
	{
		size_t iUCSCount;
		try
		{
			iUCSCount = pMod->GetUcsCount();
		}
		catch(CRainmanException *pE)
		{
			throw new CModStudioException(__FILE__, __LINE__, "Unable to get UCS count", pE);
		}
		for(size_t i = 0; i < iUCSCount; ++i)
		{
			try
			{
				CModuleFile::CUcsHandler *pUcs = pMod->GetUcs(i);
				if(TheConstruct->GetModule()->GetModuleType() == CModuleFile::MT_CompanyOfHeroes)
					aList.Add( AsciiTowxString(pUcs->GetFileName()) );
				else
					aList.Add( AsciiTowxString(pMod->GetFileMapName()) + wxT("\'s ") + AsciiTowxString(pUcs->GetFileName()) + wxT(" (Read Only)"));
				m_lstUcsFiles.push_back( pUcs->GetUcsHandle() );
			}
			catch(CRainmanException *pE)
			{
				throw new CModStudioException(pE, __FILE__, __LINE__, "Unable to get UCS name for file #%li", i);
			}
		}
	}
}

frmUCSSelector::frmUCSSelector(const wxString& sTitle)
	:wxDialog(wxTheApp->GetTopWindow(), -1, sTitle, wxPoint(0, 0) , wxSize(320, 480), wxFRAME_FLOAT_ON_PARENT | wxFRAME_TOOL_WINDOW | wxCAPTION)
{
	m_bGotAnswer = false;
	m_bAnswerIsReadOnly = false;
	m_pAnswer = 0;
	CentreOnParent();
	wxBoxSizer *pTopSizer = new wxBoxSizer( wxVERTICAL );

	wxArrayString aModUCSs;
	size_t iUCSCount;
	try
	{
		m_iWritableUcsCount = iUCSCount = TheConstruct->GetModule()->GetUcsCount();
	}
	catch(CRainmanException *pE)
	{
		delete pTopSizer;
		throw new CModStudioException(__FILE__, __LINE__, "Unable to get UCS count", pE);
	}
	for(size_t i = 0; i < iUCSCount; ++i)
	{
		try
		{
			CModuleFile::CUcsHandler *pUcs = TheConstruct->GetModule()->GetUcs(i);
			aModUCSs.Add(AsciiTowxString( pUcs->GetFileName() ));
			m_lstUcsFiles.push_back( pUcs->GetUcsHandle() );
		}
		catch(CRainmanException *pE)
		{
			delete pTopSizer;
			throw new CModStudioException(pE, __FILE__, __LINE__, "Unable to get UCS name for file #%li", i);
		}
	}

	size_t iReqCount = TheConstruct->GetModule()->GetRequiredCount();
	for(size_t i = 0; i < iReqCount; ++i)
	{

		_AddReadOnlyModToList(TheConstruct->GetModule()->GetRequired(i)->GetModHandle(), aModUCSs);
	}

	size_t iEngineCount = TheConstruct->GetModule()->GetEngineCount();
	for(size_t i = 0; i < iEngineCount; ++i)
	{
		_AddReadOnlyModToList(TheConstruct->GetModule()->GetEngine(i), aModUCSs);
	}

	wxWindow *pBgTemp;
	pTopSizer->Add(pBgTemp = new wxStaticText(this, -1, AppStr(ucsselect_caption)), 0, wxALIGN_LEFT | wxALL, 3);
	pTopSizer->Add(m_pList = new wxListBox(this, IDC_UCSList, wxDefaultPosition, wxDefaultSize, aModUCSs), 1, wxALL | wxEXPAND, 3);

	wxBoxSizer *pButtonSizer = new wxBoxSizer( wxHORIZONTAL );

	pButtonSizer->Add(new wxButton(this, wxID_NEW, AppStr(ucsselect_new)), 0, wxEXPAND | wxALL, 3);
	pButtonSizer->Add(new wxButton(this, wxID_OPEN, AppStr(ucsselect_open)), 0, wxEXPAND | wxALL, 3);
	pButtonSizer->Add(new wxButton(this, wxID_CLOSE, AppStr(ucsselect_close)), 0, wxEXPAND | wxALL, 3);

	pTopSizer->Add(pButtonSizer, 0, wxALIGN_RIGHT);

	SetSizer(pTopSizer);
	pTopSizer->SetSizeHints( this );
	SetBackgroundColour(pBgTemp->GetBackgroundColour());
}

void frmUCSSelector::OnSize(wxSizeEvent& event)
{ UNUSED(event);
	Layout();
}

bool frmUCSSelector::GotAnswer()
{
	return m_bGotAnswer;
}

wxString& frmUCSSelector::GetAnswer()
{
	return m_sAnswer;
}

CUcsFile* frmUCSSelector::GetAnswerUcs()
{
	return m_pAnswer;
}

bool frmUCSSelector::IsAnswerUcsReadOnly()
{
	return m_bAnswerIsReadOnly;
}

void frmUCSSelector::OnNewClick(wxCommandEvent& event)
{ UNUSED(event);
	wxString sVal;
	wxString sDefault = AsciiTowxString(TheConstruct->GetModule()->GetFileMapName());
	if(TheConstruct->GetModule()->GetModuleType() == CModuleFile::MT_CompanyOfHeroes)
	{
		sDefault.Append(wxT("."));
		sDefault.Append(AsciiTowxString(TheConstruct->GetModule()->GetLocale()));
	}
	sDefault.Append(wxT(".ucs"));
	sVal = wxGetTextFromUser(AppStr(ucsselect_newcaption), AppStr(ucsselect_new), sDefault, this, wxDefaultCoord, wxDefaultCoord, false);
	if(!sVal.IsEmpty())
	{
		m_sAnswer = sVal;
		EndModal(wxID_NEW);
	}
}

void frmUCSSelector::OnLoadClick(wxCommandEvent& event)
{  UNUSED(event);
	wxString sVal;
	sVal = m_pList->GetStringSelection();
	if(!sVal.IsEmpty())
	{
		m_sAnswer = sVal;
		int iSelection = m_pList->GetSelection();
		m_bAnswerIsReadOnly = ( ((size_t)iSelection) >= m_iWritableUcsCount);
		if(TheConstruct->GetModule()->GetModuleType() == CModuleFile::MT_CompanyOfHeroes) m_bAnswerIsReadOnly = false;
		std::list<CUcsFile*>::iterator itr = m_lstUcsFiles.begin();
		while(iSelection) --iSelection, ++itr;
		m_pAnswer = *itr;

		EndModal(wxID_OPEN);
	}
}

bool frmUCSSelector::SelectFromReference(unsigned long iVal)
{
	int iN = 0;
	for(std::list<CUcsFile*>::iterator itr = m_lstUcsFiles.begin(); itr != m_lstUcsFiles.end(); ++itr, ++iN)
	{
		if(*itr && (**itr).ResolveStringID(iVal))
		{
			m_pList->SetSelection(iN);

			wxString sVal;
			sVal = m_pList->GetStringSelection();
			if(sVal.IsEmpty()) return false;
			m_sAnswer = sVal;

			int iSelection = iN;
			m_bAnswerIsReadOnly = ( ((size_t)iSelection) >= m_iWritableUcsCount);
			if(TheConstruct->GetModule()->GetModuleType() == CModuleFile::MT_CompanyOfHeroes) m_bAnswerIsReadOnly = false;
			m_pAnswer = *itr;

			return true;
		}
	}
	return false;
}

void frmUCSSelector::OnCloseClick(wxCommandEvent& event)
{ UNUSED(event);
	EndModal(wxID_CLOSE);
}