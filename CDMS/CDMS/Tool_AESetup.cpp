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
#include "strconv.h"
#include "strings.h"
#include "Utility.h"
#include "CtrlStatusText.h"
#include "frmMessage.h"
#include "Tool_AESetup.h"
#include <vector>
#include "Common.h"

BEGIN_EVENT_TABLE(frmUCSToDAT, wxDialog)
	EVT_BUTTON(IDC_BrowseOut, frmUCSToDAT::OnBrowseOutClick)
	EVT_BUTTON(IDC_Cancel, frmUCSToDAT::OnCancelClick)
	EVT_BUTTON(IDC_Go, frmUCSToDAT::OnGoClick)
END_EVENT_TABLE()

frmUCSToDAT::frmUCSToDAT()
	: wxDialog(wxTheApp->GetTopWindow(), -1, AppStr(aesetup_name), wxPoint(0, 0) , wxDefaultSize, wxFRAME_FLOAT_ON_PARENT | wxFRAME_TOOL_WINDOW | wxCAPTION)
{
	CentreOnParent();
	wxFlexGridSizer *pTopSizer = new wxFlexGridSizer(3);
	pTopSizer->SetFlexibleDirection(wxHORIZONTAL);
	pTopSizer->AddGrowableCol(1, 1);

	wxWindow *pBgTemp;
	wxTextValidator oValidator(wxFILTER_NUMERIC);

	pTopSizer->Add(SBT(new wxStaticText(this, -1, AppStr(aesetup_outselect)), AppStr(aesetup_outselect_help)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE | wxALL, 3);
	pTopSizer->Add(SBT(m_pOutFile = new wxTextCtrl(this, IDC_FileOut, wxT("")), AppStr(aesetup_outselect_help)), 1, wxALL | wxEXPAND, 3);
	pTopSizer->Add(SBT(pBgTemp = new wxButton(this, IDC_BrowseOut, AppStr(aesetup_browse)), AppStr(aesetup_outselect_help)), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxFIXED_MINSIZE | wxALL, 3);

	pTopSizer->Add(SBT(new wxStaticText(this, -1, AppStr(aesetup_range1)), AppStr(aesetup_range_help)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE | wxALL, 3);
	pTopSizer->Add(SBT(m_pRangeStart = new wxTextCtrl(this, IDC_FileOut, wxT("90000")), AppStr(aesetup_range_help)), 1, wxALL | wxEXPAND, 3);
	m_pRangeStart->SetValidator(oValidator);
	pTopSizer->AddSpacer(0);

	pTopSizer->Add(SBT(new wxStaticText(this, -1, AppStr(aesetup_range2)), AppStr(aesetup_range_help)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE | wxALL, 3);
	pTopSizer->Add(SBT(m_pRangeEnd = new wxTextCtrl(this, IDC_FileOut, wxT("199999")), AppStr(aesetup_range_help)), 1, wxALL | wxEXPAND, 3);
	m_pRangeEnd->SetValidator(oValidator);
	pTopSizer->AddSpacer(0);

	wxBoxSizer *pButtonSizer = new wxBoxSizer( wxHORIZONTAL );
	pButtonSizer->Add(new wxButton(this, IDC_Cancel, AppStr(newmod_cancel)), 0, wxEXPAND | wxALL, 3);
	pButtonSizer->Add(new wxButton(this, IDC_Go, AppStr(newmod_create)), 0, wxEXPAND | wxALL, 3);

	pTopSizer->AddSpacer(0);
	pTopSizer->Add(pButtonSizer, 0, wxALIGN_CENTER);
	pTopSizer->AddSpacer(0);

	SetSizer(pTopSizer);
	pTopSizer->SetSizeHints( this );
	SetBackgroundColour(pBgTemp->GetBackgroundColour());
}

void frmUCSToDAT::OnCancelClick(wxCommandEvent& event)
{ UNUSED(event);
	EndModal(wxID_CLOSE);
}

void frmUCSToDAT::OnBrowseOutClick(wxCommandEvent& event)
{ UNUSED(event);
	m_pOutFile->SetValue(wxFileSelector(AppStr(aesetup_outselect), wxT(""), m_pOutFile->GetValue(), wxT("dat"), AppStr(aesetup_filter), wxSAVE | wxOVERWRITE_PROMPT, TheConstruct));
}

void frmUCSToDAT::OnGoClick(wxCommandEvent& event)
{ UNUSED(event);
	if(m_pOutFile->GetValue().empty())
	{
		::wxMessageBox(AppStr(aesetup_novalue), wxT("Error"), wxICON_ERROR, wxTheApp->GetTopWindow());
		return;
	}

	unsigned long iRangeStart, iRangeEnd;
	if(!m_pRangeStart->GetValue().ToULong(&iRangeStart) || !m_pRangeEnd->GetValue().ToULong(&iRangeEnd) || iRangeEnd < iRangeStart)
	{
		::wxMessageBox(AppStr(aesetup_norange), wxT("Error"), wxICON_ERROR, wxTheApp->GetTopWindow());
		return;
	}

	frmMessage *pMsg = new frmMessage(wxT("IDB_TOOL_AESETUP"), AppStr(aesetup_message));
	pMsg->Show(TRUE);
	wxSafeYield(pMsg);

	UCSToDATConvertor oConvertor;

	bool bGood = true;
	char* sFilename = wxStringToAscii(m_pOutFile->GetValue());
	try
	{
		oConvertor.setOutputFilename(sFilename);
		oConvertor.setRange(iRangeStart, iRangeEnd);
		oConvertor.setModule(TheConstruct->GetModule());
		oConvertor.doConvertion();
	}
	catch(CRainmanException* pE)
	{
		ErrorBoxE(pE);
		bGood = false;
	}

	delete[] sFilename;

	delete pMsg;

	if(bGood)
	{
		wxMessageBox(AppStr(aesetup_done),AppStr(aesetup_name),wxICON_INFORMATION,TheConstruct);
		EndModal(wxID_OK);
	}
}

UCSToDATConvertor::UCSToDATConvertor()
{
	m_sOutputName = 0;
	m_iRangeStart = 0;
	m_iRangeEnd = 0;
	m_pModule = 0;
}

UCSToDATConvertor::~UCSToDATConvertor()
{
	delete[] m_sOutputName;
}

void UCSToDATConvertor::setOutputFilename(const char* sFilename)
{
	delete[] m_sOutputName;
	m_sOutputName = 0;
	size_t iLength = strlen(CHECK_STR(sFilename)) + 1;
	m_sOutputName = CHECK_MEM(new char[iLength]);
	memcpy(m_sOutputName, sFilename, iLength);
}

void UCSToDATConvertor::setRange(unsigned long iStart, unsigned long iEnd)
{
	if(iEnd < iStart)
		throw new CModStudioException(0, __FILE__, __LINE__, "Range %lu -> %lu is invalid", iStart, iEnd);
	m_iRangeStart = iStart;
	m_iRangeEnd = iEnd;
}

void UCSToDATConvertor::setModule(const CModuleFile* pModule)
{
	m_pModule = pModule;
}

void UCSToDATConvertor::_startRange(unsigned long iValue)
{
	iValue = ((iValue / 50) * 50);
	unsigned long iEnd = iValue + 49;
	if(iValue < m_iRangeStart)
		iValue = m_iRangeStart;
	if(iEnd > m_iRangeEnd)
		iEnd = m_iRangeEnd;
	fprintf(m_fDAT, "\nrangestart %lu %lu\n", iValue, iEnd);
}

void UCSToDATConvertor::_endRange()
{
	fprintf(m_fDAT, "rangeend\n");
}

bool UCSToDATConvertor::_nextEntry(unsigned long* pCode, wchar_t** pValue)
{
	size_t i = 0;
	size_t n = m_iUCSCount;
	for(;i < m_iUCSCount; ++i)
	{
		if(m_aUCSFiles[i] != m_aUCSFileEnds[i])
		{
			*pCode = m_aUCSFiles[i]->first;
			n = i;
			break;
		}
	}
	for(;i < m_iUCSCount; ++i)
	{
		if(m_aUCSFiles[i] != m_aUCSFileEnds[i] && m_aUCSFiles[i]->first < *pCode)
		{
			*pCode = m_aUCSFiles[i]->first;
			n = i;
			break;
		}
	}
	if(n == m_iUCSCount)
		return false;
	*pValue = m_aUCSFiles[n]->second;
	++m_aUCSFiles[n];
	return true;
}

static void AddUcsFilesToVector(const CModuleFile* pModule, std::vector<const CUcsFile*>& vFiles)
{
	if(pModule)
	{
		for(size_t i = 0; i < pModule->GetUcsCount(); ++i)
			vFiles.push_back(pModule->GetUcs(i)->GetUcsHandle());
	}
}

void UCSToDATConvertor::doConvertion()
{
	if(m_sOutputName == 0 || m_pModule == 0)
		QUICK_THROW("Convertion parameters not set");

	std::vector<const CUcsFile*> vFiles;
	AddUcsFilesToVector(m_pModule, vFiles);
	for(size_t i = 0; i < m_pModule->GetRequiredCount(); ++i)
		AddUcsFilesToVector(m_pModule->GetRequired(i)->GetModHandle(), vFiles);
	for(size_t i = 0; i < m_pModule->GetEngineCount(); ++i)
		AddUcsFilesToVector(m_pModule->GetEngine(i), vFiles);

	m_iUCSCount = vFiles.size();
	m_aUCSFiles = new std::map<unsigned long, wchar_t*>::const_iterator[m_iUCSCount];
	m_aUCSFileEnds = new std::map<unsigned long, wchar_t*>::const_iterator[m_iUCSCount];
	for(size_t i = 0; i < m_iUCSCount; ++i)
	{
		const std::map<unsigned long, wchar_t*>* pMap = vFiles[i]->GetRawMap();

		m_aUCSFiles[i] = pMap->begin();
		m_aUCSFileEnds[i] = pMap->end();
	}

	m_fDAT = fopen(m_sOutputName, "wt");
	if(m_fDAT == 0)
	{
		delete[] m_aUCSFiles;
		delete[] m_aUCSFileEnds;
		throw new CModStudioException(0, __FILE__, __LINE__, "Cannot open output file \'%s\'", m_sOutputName);
	}

	fprintf(m_fDAT, "/////////////////////////////////////////////////////////////////////\n");
	fprintf(m_fDAT, "// Generated by Corsix\'s Mod Studio\n\n");
	fprintf(m_fDAT, "filerange %lu %lu\n", m_iRangeStart, m_iRangeEnd);

	bool bInRange = false;
	unsigned long iCurrentRange = 0;
	bool bGotEntry;
	unsigned long iEntryCode;
	wchar_t* pEntryText;
	while((bGotEntry = _nextEntry(&iEntryCode, &pEntryText)) && iEntryCode < m_iRangeStart);
	if(bGotEntry && iEntryCode <= m_iRangeEnd)
	{
		do
		{
			unsigned long iRange = iEntryCode / 50;
			if(!bInRange || iRange != iCurrentRange)
			{
				if(bInRange)
					_endRange();
				else
					bInRange = true;
				_startRange(iEntryCode);
				iCurrentRange = iRange;
			}
			char* sString = UnicodeToAscii(pEntryText);
			fprintf(m_fDAT, "%lu\t%s\n", iEntryCode, sString);
			delete[] sString;
		} while(_nextEntry(&iEntryCode, &pEntryText) && iEntryCode < m_iRangeEnd);
	}
	if(bInRange)
		_endRange();

	fclose(m_fDAT);
	delete[] m_aUCSFiles;
	delete[] m_aUCSFileEnds;
}
