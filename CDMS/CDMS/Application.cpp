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

#include "Application.h"
#include "Construct.h"
#include <crtdbg.h>
#include "strings.h"
#include "Utility.h"
#include <wx/fileconf.h>
#include <wx/wfstream.h>
#include <wx/sysopt.h>
#include <wx/image.h>
#include "Common.h"
#include <windows.h>
#include <wchar.h>

IMPLEMENT_APP(CDMSApplication)

namespace
{
	// Data files the program cannot run without. config.txt is deliberately not
	// listed: it is created on first exit.
	bool _CollectMissingDataFiles(wxString& sMissing)
	{
		static const wxChar* const apRequired[] = {
			wxT("lua.dat"),
			wxT("bfx.dat"),
			wxT("scardoc.dat"),
			wxT("dictionaries\\RGD_DIC.TXT"),
		};
		const wxString sFolder = AppStr(app_filespath);
		for(size_t i = 0; i < sizeof(apRequired) / sizeof(apRequired[0]); ++i)
		{
			if(!wxFileExists(sFolder + apRequired[i]))
			{
				sMissing.Append(wxT("\n  - ")).Append(sFolder).Append(apRequired[i]);
			}
		}
		return sMissing.IsEmpty();
	}
}

bool CDMSApplication::OnInit()
{
	// The application stores its data under a path relative to its executable.
	wchar_t sExecutablePath[MAX_PATH];
	DWORD cchExecutablePath = GetModuleFileNameW(NULL, sExecutablePath, MAX_PATH);
	if(cchExecutablePath > 0 && cchExecutablePath < MAX_PATH)
	{
		wchar_t* pLastSlash = wcsrchr(sExecutablePath, L'\\');
		if(pLastSlash)
		{
			*pLastSlash = L'\0';
			SetCurrentDirectoryW(sExecutablePath);
		}
	}

	// Refuse to start from a half-copied installation. The alternative is a
	// chain of library-error dialogs and a frame that cannot do anything.
	wxString sMissingDataFiles;
	if(!_CollectMissingDataFiles(sMissingDataFiles))
	{
		wxMessageBox(wxString::Format(AppStr(app_datamissing), wxGetCwd(), sMissingDataFiles),
			AppStr(app_datamissing_title), wxOK | wxICON_ERROR);
		return FALSE;
	}

	wxSystemOptions::SetOption(wxT("msw.remap"), 0);

	wxConfigBase::Set(new wxFileConfig(wxFileInputStream(AppStr(configfile_name))));
	wxConfigBase::Get()->SetRecordDefaults(true);
	wxConfigBase::Get()->SetExpandEnvVars(false);
	wxConfigBase::Get()->SetPath(AppStr(config_initialpath));

	wxImage::AddHandler(new wxTGAHandler);

	//_CrtSetBreakAlloc(5143);

    ConstructFrame *pConstruct = new ConstructFrame(AppStr(app_name), 
         wxDefaultPosition, wxSize(500,500) );
	pConstruct->SetIcon(wxIcon(wxT("APPICON"),wxBITMAP_TYPE_ICO_RESOURCE));
    pConstruct->Show(TRUE);
	pConstruct->Maximize(true);
    SetTopWindow(pConstruct);
    return TRUE;
} 

int CDMSApplication::OnExit()
{
	wxConfigBase* pBase = wxConfigBase::Get(false);
	if(!pBase)
	{
		// Startup was refused before the configuration was opened.
		return wxApp::OnExit();
	}
	wxFileConfig* pConfig = (wxFileConfig*)pBase;
	pConfig->Save(wxFileOutputStream(AppStr(configfile_name)));
	return wxApp::OnExit();
}

#ifndef _DEBUG
bool CDMSApplication::OnExceptionInMainLoop()
{
	throw;
}
#endif

int CDMSApplication::OnRun()
{
#ifndef _DEBUG
	try
	{
#endif
		return wxApp::OnRun();
#ifndef _DEBUG
	}
	catch(CRainmanException *pE)
	{
		_ErrorBox(pE, __FILE__, __LINE__, true);
		return -1;
	}
	catch(...)
	{
		_ErrorBox(wxT("Unhandled exception (not from Rainman)"), __FILE__, __LINE__);
		return -1;
	}
#endif
}
