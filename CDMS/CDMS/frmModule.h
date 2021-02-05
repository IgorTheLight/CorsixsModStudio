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

#ifndef _FRM_MODULE_H_
#define _FRM_MODULE_H_
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
// ----------------------------
#include <Rainman.h>
#include "strings.h"

class frmModule : public wxWindow
{
public:
	frmModule(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

	void OnSize(wxSizeEvent& event);

	enum
	{
		IDP_General = wxID_HIGHEST + 1,
		IDP_DataFolders,
		IDP_DataArchives,
		IDP_RequiredMods,
		IDP_CompatibleMods
	};

protected:
	class pgMain : public wxWindow
	{
	protected:
		bool m_bDoneInit;
	public:
		pgMain(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

		void OnSize(wxSizeEvent& event);

		enum
		{
			IDC_Description = wxID_HIGHEST + 1,
			IDC_DllName,
			IDC_ModFolder,
			IDC_TextureFE,
			IDC_TextureIcon,
			IDC_UIName,
			IDC_VersionMajor,
			IDC_VersionMinor,
			IDC_VersionRevision,
			IDC_VersionHelp
		};

		void InitModFolderList(wxControlWithItems* pList);
		void InitDllList(wxControlWithItems* pList);

		void OnDescriptionUpdate(wxCommandEvent& event);
		void OnUINameUpdate(wxCommandEvent& event);
		void OnTextureFEUpdate(wxCommandEvent& event);
		void OnTextureIconUpdate(wxCommandEvent& event);

		DECLARE_EVENT_TABLE()
	};

	class pgDataFolders : public wxWindow
	{
	public:
		pgDataFolders(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
			const wxString& sTitle = AppStr(mod_datafolders_caption), const wxString& sItemName = AppStr(mod_datafolder), bool bUpdateMessage = true, void (* pInitList)(wxArrayString &) = FillInitialValues);

		void OnSize(wxSizeEvent& event);

		static void FillInitialValues(wxArrayString &aInitialValues);

		DECLARE_EVENT_TABLE()
	};

	class pgDataArchives : public pgDataFolders
	{
	public:
		pgDataArchives(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

		static void FillInitialValues(wxArrayString &aInitialValues);
	};

	class pgRequiredMods : public pgDataFolders
	{
	public:
		pgRequiredMods(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

		static void FillInitialValues(wxArrayString &aInitialValues);
	};

	class pgCompatibleMods : public pgDataFolders
	{
	public:
		pgCompatibleMods(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

		static void FillInitialValues(wxArrayString &aInitialValues);
	};

	DECLARE_EVENT_TABLE()
};

#endif