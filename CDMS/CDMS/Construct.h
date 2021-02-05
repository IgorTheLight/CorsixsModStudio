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

#ifndef _CONSTRUCT_H_
#define _CONSTRUCT_H_
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#include <wx/aui/auibook.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
// ----------------------------
#include "wx/splitter.h"
#include <wx/dynlib.h>
#include "frmLoading.h"
#include "frmFiles.h"
#include <Rainman.h>

class ConstructFrame: public wxFrame
{
protected:
	wxSplitterWindow* m_pSplitter;
	wxAuiNotebook* m_pTabs;
	CModuleFile* m_pModule;
	wxString m_sModuleFile;
	frmLoading* m_pLoadingForm;

	CRgdHashTable* m_pRgdHashTable;
	char* m_sRgdHashCustomOut;

public:
	wxDynamicLibrary oSquishLibrary;
	wxDynamicLibrary oLua512Library;

	class ITool
	{
	public:
		virtual wxString GetName() = 0;
		virtual wxString GetHelpString() = 0;
		virtual wxString GetBitmapName() = 0;

		virtual void DoAction() = 0;
	};
protected:
	std::vector<ITool*> m_vTools;
 
public:
    ConstructFrame(const wxString& sTitle, const wxPoint& oPos, const wxSize& oSize);
	~ConstructFrame();

	void OnCloseMod(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
	void OnOpenModDoW(wxCommandEvent& event);
	void OnOpenModDC(wxCommandEvent& event);
	void OnOpenModSS(wxCommandEvent& event);
	void OnOpenModCoH(wxCommandEvent& event);
	void OnOpenModOF(wxCommandEvent& event);
	void OnOpenSga(wxCommandEvent& event);
	void DoNewMod();

	enum eLoadModGames
	{
		LM_Any,
		LM_DoW_WA,
		LM_DC,
		LM_SS,
		LM_CoH_OF,
	};

	void DoLoadSga();
	void DoLoadMod(wxString sPath = wxT(""), eLoadModGames eGame = LM_Any);
	void LaunchModTool1(wxCommandEvent& event);
	void LaunchModTool2(wxCommandEvent& event);
	void LaunchModTool3(wxCommandEvent& event);
	void LaunchModTool4(wxCommandEvent& event);
	void LaunchModTool5(wxCommandEvent& event);
	void LaunchModTool6(wxCommandEvent& event);
	void LaunchModTool7(wxCommandEvent& event);
	void LaunchModTool8(wxCommandEvent& event);
	void LaunchModTool9(wxCommandEvent& event);
	void LaunchModTool10(wxCommandEvent& event);
	void LaunchModTool11(wxCommandEvent& event);
	void LaunchModTool12(wxCommandEvent& event);
	void LaunchModTool13(wxCommandEvent& event);
	void LaunchModTool14(wxCommandEvent& event);
	void LaunchModTool15(wxCommandEvent& event);
	void LaunchModTool16(wxCommandEvent& event);
	void LaunchModTool17(wxCommandEvent& event);
	void LaunchModTool18(wxCommandEvent& event);
	void LaunchModTool19(wxCommandEvent& event);
	void LaunchModTool20(wxCommandEvent& event);
	void LaunchRelicAttributeEditor(wxCommandEvent& event);
	void LaunchRelicAudioEditor(wxCommandEvent& event);
	void LaunchRelicChunkyViewer(wxCommandEvent& event);
	void LaunchRelicFXTool(wxCommandEvent& event);
	void LaunchRelicMisionEditor(wxCommandEvent& event);
	void LaunchRelicModPackager(wxCommandEvent& event);
	void LaunchRelicObjectEditor(wxCommandEvent& event);
	void LaunchCOH(wxCommandEvent& event);
	void LaunchW40k(wxCommandEvent& event);
	void LaunchW40kWA(wxCommandEvent& event);
	void LaunchDC(wxCommandEvent& event);
	void LaunchSS(wxCommandEvent& event);
	void LaunchWarnings(wxCommandEvent& event);
	void LaunchCredits(wxCommandEvent& event);
	void LaunchHelp(wxCommandEvent& event);
	static void StaticLaunchHelp();
	void LaunchDonate(wxCommandEvent& event);
	void LaunchLuaRef(wxCommandEvent& event);
	void LaunchKresjahWiki(wxCommandEvent& event);
	void LaunchForumDoW(wxCommandEvent& event);
	void LaunchForumCoH(wxCommandEvent& event);
	void LaunchNewRDNWiki(wxCommandEvent& event);
	void LaunchRDNWiki(wxCommandEvent& event);
	void HideDonate(wxCommandEvent& event);

	void LaunchTool(wxString sExeName, bool bIsInModTools = true);
	static void LaunchURL(wxString sURL);

	void OnCloseWindow(wxCloseEvent& event);

	void OnSashMove(wxSplitterEvent& event);

	CModuleFile* GetModule() const;
	const wxString& GetModuleFile() const;
	void SetModule(CModuleFile* pMod);
	wxAuiNotebook* GetTabs();
	frmLoading* GetLoadingForm();
	frmFiles* GetFilesList();

	size_t GetToolCount();
	ITool* GetTool(size_t i);
	void DoTool(wxString sName);

	void SetLoadingForm(frmLoading* pLoading);

	static void __cdecl LoadModCallback(const char* sMsg, void* pTag);
	static IFileStore::IOutputStream* SaveFileCallback(const char* sFile, bool bEraseIfPresent, void* pTag);

	/*!
		may throw a CRainmanException
	*/
	CRgdHashTable* GetRgdHashTable();
 
    DECLARE_EVENT_TABLE()
};

inline ConstructFrame* GetConstruct() {return (ConstructFrame*)wxTheApp->GetTopWindow();}
#define TheConstruct GetConstruct()

enum
{
	// File Menu
    IDM_NewMod = wxID_HIGHEST + 1,
    IDM_LoadModDoWWA,
	IDM_LoadModDC,
	IDM_LoadModSS,
	IDM_LoadModCoH,
	IDM_LoadSga,
	IDM_Quit,
	// Mod Menu
	IDM_Settings,
	IDM_Tools,
	// Mod.Tools Menu
	IDM_ModTool1,
	IDM_ModTool2,
	IDM_ModTool3,
	IDM_ModTool4,
	IDM_ModTool5,
	IDM_ModTool6,
	IDM_ModTool7,
	IDM_ModTool8,
	IDM_ModTool9,
	IDM_ModTool10,
	IDM_ModTool11,
	IDM_ModTool12,
	IDM_ModTool13,
	IDM_ModTool14,
	IDM_ModTool15,
	IDM_ModTool16,
	IDM_ModTool17,
	IDM_ModTool18,
	IDM_ModTool19,
	IDM_ModTool20,
	// Relic's Tools Menu
	IDM_AttributeEditor,
	IDM_AudioEditor,
	IDM_ChunkyViewer,
	IDM_FXTool,
	IDM_MissionEditor,
	IDM_ModPackager,
	IDM_ObjectEditor,
	// Play Menu
	IDM_PlayCOH,
	IDM_PlayW40k,
	IDM_PlayWXP,
	IDM_PlayDC,
	IDM_PlaySS,
	IDM_PlayWarn,
	IDM_PlayDev,
	IDM_PlayNoMovies,
	// Help Menu
	IDM_Index,
	IDM_LuaRef,
	IDM_ForumDoW,
	IDM_ForumCoH,
	IDM_RDNWikiNew,
	IDM_RDNWiki,
	IDM_KresjahWiki,
	IDM_About,
	IDM_Credits,
	IDM_HideDonate,

	// Stuff
	IDC_Splitter
};

#endif