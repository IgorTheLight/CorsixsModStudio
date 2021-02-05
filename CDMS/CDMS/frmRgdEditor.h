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

#ifndef _FRM_RGDEDITOR_H_
#define _FRM_RGDEDITOR_H_
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
#include <wx/propgrid/propgrid.h>
#include <wx/splitter.h>
#include <wx/treectrl.h>
#include <list>
#include <wx/propgrid/propdev.h>

class RGDwxPropertyGridManager;

WX_PG_DECLARE_STRING_PROPERTY(myReferenceProperty)
WX_PG_DECLARE_STRING_PROPERTY(myUcsRefProperty)

class frmRGDEditor : public wxWindow
{
protected:
	wxPropertyGrid* m_pPropertyGrid;
	wxTreeCtrl* m_pTables;
	wxSplitterWindow* m_pSplitter;
	RGDwxPropertyGridManager* m_pPropManager;
	wxString m_sFilename;
	wxTreeItemId m_oFileParent;

	int m_iObjectType;
	union {
	CRgdFile* m_pNodeObject;
	IMetaNode::IMetaTable* m_pTableObject;
	};
	CLuaFile2* m_pLua2Object;
	bool m_bDeleteWhenDone;
	bool m_bDataNeedsSaving;

	void _FillFromMetaTable(wxTreeItemId& oParent, IMetaNode::IMetaTable* pTable, bool bSkipLuaGlobals = false);
	void _DoValueChange(wxTreeItemId oTreeItem, wxPropertyGridEvent& event, wxPGId oPropItem, bool bIsChild);

	struct _NodeHelp
	{
		_NodeHelp();
		short int iLuaType;
		// unsigned long iNameHash;
		char *sShortDesc, *sLongDesc;
		union {
		char* sSample;
		std::list<char*>* lstRefs;
		};
	};

	static std::map<unsigned long, _NodeHelp>* m_pNodeHelp;
	static char* _ReadNiceString(FILE* f);
	static void _MakeSureNodeHelpLoaded();
	void _DoFillRightSide(IMetaNode* pNode, IMetaNode::IMetaTable* pTable);
	
	/*!
		\return Returns true if you now need to delete pNode, false otherwise.
	*/
	bool _SyncTreeView(IMetaNode* pNode, wxTreeItemId& oNode);
public:
	frmRGDEditor(wxTreeItemId& oFileParent, wxString sFilename, wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	~frmRGDEditor();

	bool FillFromMetaNode(CRgdFile *pNode, bool bDeleteWhenDone = true);
	bool FillFromMetaTable(IMetaNode::IMetaTable *pTable, bool bDeleteWhenDone = true);
	bool FillFromLua2(CLuaFile2 *pLua, bool bDeleteWhenDone = true);

	static void FreeNodeHelp();
	static unsigned long NodeNameToMultiHash(const wchar_t *sName);
	static wxString GetMetaNodeName(IMetaNode* pNode);
	static wxString GetMetaTableValue(IMetaNode::IMetaTable* pTable, IMetaNode* pNode = 0);
	static wxPGProperty* GetMetaNodeEditor(IMetaNode* pNode, wxString sName = wxT(""), wxString sNameId = wxT(""));
	static wxString GetMetaNodeHelp(IMetaNode* pNode);

	void OnSize(wxSizeEvent& event);
	void OnPropertyChange(wxPropertyGridEvent& event);
	void OnTreeSelect(wxTreeEvent& event);
	void OnTreeExpanding(wxTreeEvent& event);
	void OnTreeRightClick(wxTreeEvent& event);
	void OnAddChild(wxCommandEvent& event);
	void OnDelete(wxCommandEvent& event);

	void DoSave();
	void OnSave(wxCommandEvent &event);

	void OnCopy(wxCommandEvent &event);
	void OnPaste(wxCommandEvent &event);
	void OnPasteInto(wxCommandEvent &event);

	void OnCloseWindow(wxCloseEvent& event);

	enum
	{
		IDC_PropertyGrid = wxID_HIGHEST + 1,
		IDC_TablesTree,
		IDC_AddChild,
		IDC_Delete,
		IDC_Copy,
		IDC_Paste,
		IDC_PasteInto,
		IDC_ToolSave
	};

	DECLARE_EVENT_TABLE()
};

#endif