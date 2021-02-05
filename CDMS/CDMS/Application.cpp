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

IMPLEMENT_APP(CDMSApplication)

bool CDMSApplication::OnInit()
{
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
	wxFileConfig* pConfig = (wxFileConfig*)wxConfigBase::Get();
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
	}
#endif
}