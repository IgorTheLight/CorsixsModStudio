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

#include "frmNewMod.h"
#include "strings.h"
#include "strconv.h"
#include "config.h"
#include "Construct.h"
#include "CtrlStatusText.h"
#include <errno.h>
#include <wx/textdlg.h>
#include "Common.h"

BEGIN_EVENT_TABLE(frmNewMod, wxDialog)
	EVT_BUTTON(IDC_New, frmNewMod::OnNewClick)
	EVT_BUTTON(IDC_Cancel, frmNewMod::OnCancelClick)
	EVT_BUTTON(IDC_Browse, frmNewMod::OnBrowseClick)
	EVT_CHOICE(IDC_Game, frmNewMod::OnGameChange)
END_EVENT_TABLE()

wxString frmNewMod::_UpdatePath(wxString sName)
{
	wxString sVal = wxT("");
	size_t iL = sName.Len();
	for(size_t i = 0; i < iL; ++i)
	{
		if( (sName[i] >= 'a' && sName[i] <= 'z') || (sName[i] >= 'A' && sName[i] <= 'Z') )
		{
			sVal.Append(sName[i]);
		}
		else if(sName[i] == ' ')
		{
			sVal.Append('_');
		}
	}
	return sVal;
}

const int g_kCompanyOfHeroes = 0;
const int g_kDawnOfWar = 1;
const int g_kWinterAssault = 2;
const int g_kDarkCrusade = 3;
const int g_kSoulstorm = 4;

frmNewMod::frmNewMod()
	:wxDialog(wxTheApp->GetTopWindow(), -1, AppStr(new_mod), wxPoint(0, 0) , wxDefaultSize, wxFRAME_FLOAT_ON_PARENT | wxFRAME_TOOL_WINDOW | wxCAPTION)
{
	m_pCreation = 0;
	try
	{
		m_sDoWPath = ConfGetDoWFolder();
	}
	catch(CRainmanException *pE)
	{
		throw new CModStudioException(__FILE__, __LINE__, "Unable to get DoW folder", pE);
	}
	try
	{
		m_sCoHPath = ConfGetCoHFolder();
	}
	catch(CRainmanException *pE)
	{
		throw new CModStudioException(__FILE__, __LINE__, "Unable to get CoH folder", pE);
	}

	try
	{
		m_sDCPath = ConfGetDCFolder();
	}
	catch(CRainmanException *pE)
	{
		throw new CModStudioException(__FILE__, __LINE__, "Unable to get DC folder", pE);
	}

	try
	{
		m_sSSPath = ConfGetSSFolder();
	}
	catch(CRainmanException *pE)
	{
		throw new CModStudioException(__FILE__, __LINE__, "Unable to get SS folder", pE);
	}

	CentreOnParent();
	wxFlexGridSizer *pTopSizer = new wxFlexGridSizer(2);
	pTopSizer->SetFlexibleDirection(wxHORIZONTAL);
	pTopSizer->AddGrowableCol(1, 1);

	wxArrayString aBases;
	aBases.Add(wxT("Company of Heroes / Opposing Fronts"));
	aBases.Add(wxT("Dawn of War"));
	aBases.Add(wxT("Dawn of War: Winter Assault"));
	aBases.Add(wxT("Dawn of War: Dark Crusade"));
	aBases.Add(wxT("Dawn of War: Soulstorm"));

	wxWindow *pBgTemp;

	pTopSizer->Add(SBT(pBgTemp = new wxStaticText(this, -1, AppStr(newmod_name)), AppStr(newmod_namehelp)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE | wxALL, 3);
	pTopSizer->Add(SBT(m_pName = new wxTextCtrl(this, IDC_Name, wxT("My Mod"), wxDefaultPosition, wxSize(300, -1)), AppStr(newmod_namehelp)), 1, wxALL | wxEXPAND, 3);

	pTopSizer->Add(SBT(new wxStaticText(this, -1, AppStr(newmod_base)), AppStr(newmod_basehelp)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE | wxALL, 3);
	pTopSizer->Add(SBT(m_pList = new wxChoice(this, IDC_Game, wxDefaultPosition, wxDefaultSize, aBases), AppStr(newmod_basehelp)), 1, wxALL | wxEXPAND , 3);

	pTopSizer->Add(SBT(new wxStaticText(this, -1, AppStr(newmod_destination)), AppStr(newmod_destinationhelp)), 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxFIXED_MINSIZE | wxALL, 3);

	wxBoxSizer *pDestSizer = new wxBoxSizer( wxHORIZONTAL );

	pDestSizer->Add(SBT(m_pCreation = new wxStaticText(this, -1, m_sDoWPath, wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE), AppStr(newmod_destinationhelp)), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 3);
	pDestSizer->Add(SBT(new wxButton(this, IDC_Browse, AppStr(sgapack_browse)), AppStr(sgapack_dirselect_label_help)), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxFIXED_MINSIZE | wxALL, 3);
	pTopSizer->Add(pDestSizer, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL);

	m_pList->SetSelection(g_kDawnOfWar);

	wxBoxSizer *pButtonSizer = new wxBoxSizer( wxHORIZONTAL );

	pTopSizer->AddSpacer(0);
	pButtonSizer->Add(new wxButton(this, IDC_Cancel, AppStr(newmod_cancel)), 0, wxEXPAND | wxALL, 3);
	pButtonSizer->Add(new wxButton(this, IDC_New, AppStr(newmod_create)), 0, wxEXPAND | wxALL, 3);

	pTopSizer->Add(pButtonSizer, 0, wxALIGN_LEFT);

	SetSizer(pTopSizer);
	pTopSizer->SetSizeHints( this );
	SetBackgroundColour(pBgTemp->GetBackgroundColour());
}

wxString frmNewMod::GetPath()
{
	return m_sDoWPath;
}

void frmNewMod::OnGameChange(wxCommandEvent& event)
{
	if(event.GetSelection() == g_kCompanyOfHeroes)
	{
		m_pCreation->SetLabel(m_sCoHPath);
	}
	else if(event.GetSelection() == g_kDarkCrusade)
	{
		m_pCreation->SetLabel(m_sDCPath);
	}
	else if(event.GetSelection() == g_kSoulstorm)
	{
		m_pCreation->SetLabel(m_sSSPath);
	}
	else if(event.GetSelection() == g_kDawnOfWar || event.GetSelection() == g_kWinterAssault)
	{
		m_pCreation->SetLabel(m_sDoWPath);
	}
}

void frmNewMod::OnBrowseClick(wxCommandEvent& event)
{
	wxString sVal = wxDirSelector(AppStr(sgapack_dirselect), m_pCreation->GetLabel(), 0, wxDefaultPosition, TheConstruct);

	m_pCreation->SetLabel(sVal);

	if(event.GetSelection() == g_kCompanyOfHeroes)
	{
		m_sCoHPath = sVal;
		TheConfig->Write(AppStr(config_cohfolder), sVal);
	}
	else if(event.GetSelection() == g_kDarkCrusade)
	{
		m_sDCPath = sVal;
		TheConfig->Write(AppStr(config_dcfolder), sVal);
	}
	else if(event.GetSelection() == g_kSoulstorm)
	{
		m_sSSPath = sVal;
		TheConfig->Write(AppStr(config_ssfolder), sVal);
	}
	else if(event.GetSelection() == g_kDawnOfWar || event.GetSelection() == g_kWinterAssault)
	{
		m_sDoWPath = sVal;
		TheConfig->Write(AppStr(config_dowfolder), sVal);
	}
}

void frmNewMod::_MakeCOH_Language(char* sToc, char* sName1, char* sName2, char* sDirectoryName, FILE* fModule)
{
	fprintf(fModule, "[data:%s:01]\xD\n", sToc);
	fprintf(fModule, "folder = %s\\Locale\\%s\\Data\xD\n", sDirectoryName, sName1);
	fprintf(fModule, "archive.01 = %s\\Archives\\%sLocale-%s\xD\n", sDirectoryName, sDirectoryName, sName1);
	fprintf(fModule, "\xD\n");

	fprintf(fModule, "[data:%s:02]\xD\n", sToc);
	fprintf(fModule, "folder = %s\\DataSound%s\xD\n", sDirectoryName, sName2);
	fprintf(fModule, "archive.01 = %s\\Archives\\%sSoundSpeech%s\xD\n", sDirectoryName, sDirectoryName, sName2);
	fprintf(fModule, "archive.02 = %s\\Archives\\%sAlliesSpeech%s\xD\n", sDirectoryName, sDirectoryName, sName2);
	fprintf(fModule, "archive.03 = %s\\Archives\\%sAxisSpeech%s\xD\n", sDirectoryName, sDirectoryName, sName2);
	fprintf(fModule, "archive.04 = %s\\Archives\\%sSoundNIS%s\xD\n", sDirectoryName, sDirectoryName, sName2);
	fprintf(fModule, "\xD\n");

	fprintf(fModule, "[data:%s:03]\xD\n", sToc);
	fprintf(fModule, "folder = WW2\\Locale\\%s\\Data\xD\n", sName1);
	fprintf(fModule, "archive.01 = WW2\\Archives\\WW2Locale-%s\xD\n", sName1);
	fprintf(fModule, "\xD\n");

	fprintf(fModule, "[data:%s:04]\xD\n", sToc);
	fprintf(fModule, "folder = WW2\\DataSound%s\xD\n", sName2);
	fprintf(fModule, "archive.01 = WW2\\Archives\\WW2SoundSpeech%s\xD\n",  sName2);
	fprintf(fModule, "archive.02 = WW2\\Archives\\WW2AlliesSpeech%s\xD\n", sName2);
	fprintf(fModule, "archive.03 = WW2\\Archives\\WW2AxisSpeech%s\xD\n", sName2);
	fprintf(fModule, "archive.04 = WW2\\Archives\\WW2SoundNIS%s\xD\n", sName2);
	fprintf(fModule, "archive.05 = WW2\\Archives\\OFCoreSpeech%s\xD\n", sName2);
	fprintf(fModule, "archive.06 = WW2\\Archives\\OFFullSpeech%s\xD\n", sName2);

	fprintf(fModule, "\xD\n");
}

void frmNewMod::_MakeCOH(char* sNiceName, char* sDirectoryFullPath, char* sDirectoryName, FILE* fModule)
{
	char* sName = wxStringToAscii(m_pName->GetValue());

	// Setup directories
	char* saDirExt = new char[strlen(sDirectoryFullPath) + 64];
	strcpy(saDirExt, sDirectoryFullPath); strcat(saDirExt, "\\Data"); _mkdir(saDirExt);
	strcpy(saDirExt, sDirectoryFullPath); strcat(saDirExt, "\\Locale"); _mkdir(saDirExt);
	strcpy(saDirExt, sDirectoryFullPath); strcat(saDirExt, "\\Movies"); _mkdir(saDirExt);
	strcpy(saDirExt, sDirectoryFullPath); strcat(saDirExt, "\\DataAttrib"); _mkdir(saDirExt);
	strcpy(saDirExt, sDirectoryFullPath); strcat(saDirExt, "\\DataSoundLow"); _mkdir(saDirExt);
	strcpy(saDirExt, sDirectoryFullPath); strcat(saDirExt, "\\DataSoundEnglish"); _mkdir(saDirExt);
	strcpy(saDirExt, sDirectoryFullPath); strcat(saDirExt, "\\DataSoundGerman"); _mkdir(saDirExt);
	strcpy(saDirExt, sDirectoryFullPath); strcat(saDirExt, "\\DataSoundFrench"); _mkdir(saDirExt);
	strcpy(saDirExt, sDirectoryFullPath); strcat(saDirExt, "\\DataSoundSpanish"); _mkdir(saDirExt);
	strcpy(saDirExt, sDirectoryFullPath); strcat(saDirExt, "\\DataSoundRussian"); _mkdir(saDirExt);
	strcpy(saDirExt, sDirectoryFullPath); strcat(saDirExt, "\\DataSoundHigh"); _mkdir(saDirExt);
	strcpy(saDirExt, sDirectoryFullPath); strcat(saDirExt, "\\DataArtLow"); _mkdir(saDirExt);
	strcpy(saDirExt, sDirectoryFullPath); strcat(saDirExt, "\\DataArtHigh"); _mkdir(saDirExt);

	// Write module file
	fprintf(fModule, "[global]\xD\n");
	fprintf(fModule, "Name = %s\xD\n", sNiceName);
	fprintf(fModule, "UIName = %s\xD\n", sName);
	fprintf(fModule, "Description = The %s mod\xD\n", sName);
	fprintf(fModule, "DllName = WW2Mod\xD\n");
	fprintf(fModule, "ModVersion = 1.0\xD\n");
	fprintf(fModule, "ModFolder = %s\\Data\xD\n", sDirectoryName);
	fprintf(fModule, "LocFolder = %s\\Locale\xD\n", sDirectoryName);
	fprintf(fModule, "ScenarioPackFolder = %s\\Scenarios\xD\n", sDirectoryName);
	fprintf(fModule, "\xD\n");

	fprintf(fModule, "[attrib:common]\xD\n");
	fprintf(fModule, "folder = %s\\DataAttrib\xD\n", sDirectoryName);
	fprintf(fModule, "archive.01 = %s\\Archives\\AttribArchive\xD\n", sDirectoryName);
	fprintf(fModule, "archive.02 = WW2\\Archives\\AttribArchive\xD\n");
	fprintf(fModule, "\xD\n");

	fprintf(fModule, "[movies:common:01]\xD\n");
	fprintf(fModule, "folder = %s\\Movies\xD\n", sDirectoryName);
	fprintf(fModule, "\xD\n");

	fprintf(fModule, "[movies:common:02]\xD\n");
	fprintf(fModule, "folder = WW2\\Movies\xD\n");
	fprintf(fModule, "\xD\n");

	fprintf(fModule, "[data:common:01]\xD\n");
	fprintf(fModule, "folder = %s\\Data\xD\n", sDirectoryName);
	fprintf(fModule, "archive.01 = %s\\Archives\\%sData\xD\n", sDirectoryName, sDirectoryName);
	fprintf(fModule, "archive.02 = %s\\Archives\\%sArt\xD\n", sDirectoryName, sDirectoryName);
	fprintf(fModule, "archive.03 = %s\\Archives\\%sSound\xD\n", sDirectoryName, sDirectoryName);
	fprintf(fModule, "archive.04 = %s\\Archives\\%sArtAmbient\xD\n", sDirectoryName, sDirectoryName);
	fprintf(fModule, "\xD\n");

	fprintf(fModule, "[data:common:02]\xD\n");
	fprintf(fModule, "folder = WW2\\Data\xD\n");
	fprintf(fModule, "archive.01 = WW2\\Archives\\WW2Data\xD\n");
	fprintf(fModule, "archive.02 = WW2\\Archives\\WW2Art\xD\n");
	fprintf(fModule, "archive.03 = WW2\\Archives\\WW2Sound\xD\n");
	fprintf(fModule, "archive.04 = WW2\\Archives\\WW2ArtAmbient\xD\n");
	fprintf(fModule, "archive.05 = WW2\\Archives\\OFSPData\xD\n");
	fprintf(fModule, "\xD\n");

	fprintf(fModule, "[data:sound_low:01]\xD\n");
	fprintf(fModule, "folder = %s\\DataSoundLow\xD\n", sDirectoryName);
	fprintf(fModule, "archive.01 = %s\\Archives\\%sSoundLow\xD\n", sDirectoryName, sDirectoryName);
	fprintf(fModule, "\xD\n");

	fprintf(fModule, "[data:sound_low:02]\xD\n");
	fprintf(fModule, "folder = WW2\\DataSoundLow\xD\n");
	fprintf(fModule, "archive.01 = WW2\\Archives\\WW2SoundLow\xD\n");
	fprintf(fModule, "\xD\n");

	fprintf(fModule, "[data:sound_high:01]\xD\n");
	fprintf(fModule, "folder = %s\\DataSoundHigh\xD\n", sDirectoryName);
	fprintf(fModule, "archive.01 = %s\\Archives\\%sSoundHigh\xD\n", sDirectoryName, sDirectoryName);
	fprintf(fModule, "\xD\n");

	fprintf(fModule, "[data:sound_high:02]\xD\n");
	fprintf(fModule, "folder = WW2\\DataSoundHigh\xD\n");
	fprintf(fModule, "archive.01 = WW2\\Archives\\WW2SoundHigh\xD\n");
	fprintf(fModule, "\xD\n");

	fprintf(fModule, "[data:art_low:01]\xD\n");
	fprintf(fModule, "folder = %s\\DataArtLow\xD\n", sDirectoryName);
	fprintf(fModule, "archive.01 = %s\\Archives\\%sArtLow\xD\n", sDirectoryName, sDirectoryName);
	fprintf(fModule, "\xD\n");

	fprintf(fModule, "[data:art_low:02]\xD\n");
	fprintf(fModule, "folder = WW2\\DataArtLow\xD\n");
	fprintf(fModule, "archive.01 = WW2\\Archives\\WW2ArtLow\xD\n");
	fprintf(fModule, "\xD\n");

	fprintf(fModule, "[data:art_high:01]\xD\n");
	fprintf(fModule, "folder = %s\\DataArtHigh\xD\n", sDirectoryName);
	fprintf(fModule, "archive.01 = %s\\Archives\\%sArtHigh\xD\n", sDirectoryName, sDirectoryName);
	fprintf(fModule, "\xD\n");

	fprintf(fModule, "[data:art_high:02]\xD\n");
	fprintf(fModule, "folder = WW2\\DataArtHigh\xD\n");
	fprintf(fModule, "archive.01 = WW2\\Archives\\WW2ArtHigh\xD\n");
	fprintf(fModule, "\xD\n");

	_MakeCOH_Language("english", "English", "English", sDirectoryName, fModule);
	_MakeCOH_Language("german", "German", "German", sDirectoryName, fModule);
	_MakeCOH_Language("french", "French", "French", sDirectoryName, fModule);
	_MakeCOH_Language("spanish", "Spanish", "Spanish", sDirectoryName, fModule);
	_MakeCOH_Language("russian", "Russian", "Russian", sDirectoryName, fModule);
	_MakeCOH_Language("italian", "Italian", "English", sDirectoryName, fModule);
	_MakeCOH_Language("czech", "Czech", "English", sDirectoryName, fModule);
	_MakeCOH_Language("polish", "Polish", "English", sDirectoryName, fModule);
	_MakeCOH_Language("chinesetrad", "ChineseTrad", "English", sDirectoryName, fModule);
	_MakeCOH_Language("japanese", "Japanese", "English", sDirectoryName, fModule);
	_MakeCOH_Language("korean", "Korean", "English", sDirectoryName, fModule);
	_MakeCOH_Language("chineseenglish", "ChineseEnglish", "English", sDirectoryName, fModule);
	_MakeCOH_Language("chinesesimp", "ChineseSimp", "English", sDirectoryName, fModule);

	// Cleanup
	delete[] sName;
}

void frmNewMod::OnNewClick(wxCommandEvent& event)
{ UNUSED(event);
	wxString sModNiceName = _UpdatePath(m_pName->GetValue());
	char* saNice = wxStringToAscii(sModNiceName);

	// Secure a directory
	char *saDoW = wxStringToAscii(m_pCreation->GetLabel());
	char *saDir = new char[strlen(saDoW) + sModNiceName.Len() + 32];
	int iRes;
	unsigned int iVal = 0;
	do
	{
		char buffer[20];
		strcpy(saDir, saDoW);
		size_t iL = strlen(saDir) - 1;
		if( (saDir[iL] != '\\') && (saDir[iL] != '/') ) strcat(saDir, "\\");
		strcat(saDir, saNice);
		if(iVal != 0)
		{
			_ultoa(iVal, buffer, 10);
			strcat(saDir, buffer);
		}
		++iVal;
	}while(((iRes = _mkdir(saDir)) == -1) && (errno == EEXIST));
	if(iRes == -1 && errno != EEXIST)
	{
		delete[] saDir;
		delete[] saDoW;
		delete[] saNice;
		wxMessageBox(AppStr(newmod_error),AppStr(new_mod),wxICON_ERROR,this);
		EndModal(wxID_CLOSE);
		return;
	}
	if(m_pList->GetSelection() == g_kDawnOfWar || m_pList->GetSelection() == g_kWinterAssault || m_pList->GetSelection() == g_kDarkCrusade || m_pList->GetSelection() == g_kSoulstorm)
	{
		char* saDirExt = new char[strlen(saDir) + 30];
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Data"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Data_Shared_Textures"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Data_Shared_Textures\\Full"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Data_Sound"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Data_Sound\\Low"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Data_Sound\\Med"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Data_Sound\\Full"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Data_Music"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Data_Whm"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Data_Whm\\High"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Data_Whm\\Low"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Data_Whm\\Medium"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Locale"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Locale\\Chinese"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Locale\\Czech"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Locale\\English"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Locale\\English_Chinese"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Locale\\French"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Locale\\German"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Locale\\Italian"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Locale\\Japanese"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Locale\\Korean"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Locale\\Korean adult"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Locale\\Polish"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Locale\\Russian"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Locale\\Slovak"); _mkdir(saDirExt);
		strcpy(saDirExt, saDir); strcat(saDirExt, "\\Locale\\Spanish"); _mkdir(saDirExt);
		delete[] saDirExt;
	}
	char* sTmp222 = saDir + strlen(saDoW);
	char* sTmpDir = strdup((*sTmp222 == '/' || *sTmp222 == '\\') ? sTmp222 + 1 : sTmp222);
	char* sDirBackup = strdup(saDir);
	// Secure a module file
	FILE *f = 0;
	iVal = 0;
	do
	{
		if(f) fclose(f);
		char buffer[20];
		strcpy(saDir, saDoW);
		size_t iL = strlen(saDir) - 1;
		if( (saDir[iL] != '\\') && (saDir[iL] != '/') ) strcat(saDir, "\\");
		strcat(saDir, saNice);
		if(iVal != 0)
		{
			_ultoa(iVal, buffer, 10);
			strcat(saDir, buffer);
		}
		strcat(saDir, ".module");
		++iVal;
	}while(f = fopen(saDir, "r"));
	delete[] saDoW;
	f = fopen(saDir, "wb");
	m_sDoWPath = AsciiTowxString(saDir);

	if(m_pList->GetSelection() == g_kCompanyOfHeroes)
	{
		_MakeCOH(saNice, sDirBackup, sTmpDir, f);
	}
	else
	{
		delete[] saNice;
		saNice = wxStringToAscii(m_pName->GetValue());

		fprintf(f, "[global]\xD\n");
		fprintf(f, "UIName = %s\xD\n", saNice);
		fprintf(f, "Description = \xD\n");
		switch(m_pList->GetSelection())
		{
		case g_kDawnOfWar:
			fprintf(f, "DllName = W40kMod\xD\n");
			break;
		case g_kDarkCrusade:
		case g_kSoulstorm:
			fprintf(f, "Playable = 1\xD\n");
		case g_kWinterAssault:
			fprintf(f, "DllName = WXPMod\xD\n");
			break;
		};
		fprintf(f, "ModFolder = %s\xD\nModVersion = 1.0\xD\nTextureFE = \xD\nTextureIcon = \xD\n\xD\n", sTmpDir);
		free(sTmpDir);
		fprintf(f, "DataFolder.1 = %%LOCALE%%\\Data\xD\nDataFolder.2 = Data\xD\nDataFolder.3 = Data_Shared_Textures\\%%TEXTURE-LEVEL%%\xD\n");
		fprintf(f, "DataFolder.4 = Data_Sound\\%%SOUND-LEVEL%%\xD\nDataFolder.5 = Data_Music\xD\nDataFolder.6 = Data_Whm\\%%MODEL-LEVEL%%\xD\n\xD\n");

		switch(m_pList->GetSelection())
		{
		case g_kDawnOfWar:
			fprintf(f, "RequiredMod.1 = W40k\xD\n");
			break;
		case g_kWinterAssault:
			fprintf(f, "RequiredMod.1 = WXP\xD\n");
			fprintf(f, "RequiredMod.2 = W40k\xD\n");
			break;
		case g_kDarkCrusade:
		case g_kSoulstorm:
			fprintf(f, "RequiredMod.1 = DXP2\xD\n");
			fprintf(f, "RequiredMod.2 = W40k\xD\n");
			break;
		};
	}

	free(sDirBackup);
	delete[] saDir;
	delete[] saNice;

	fclose(f);
	// End
	EndModal(wxID_NEW);
}

void frmNewMod::OnCancelClick(wxCommandEvent& event)
{ UNUSED(event);
	EndModal(wxID_CLOSE);
}