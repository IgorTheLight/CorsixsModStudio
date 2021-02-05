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

#include "Tools.h"
#include "strings.h"

#include "frmUCSEditor.h"
#include "frmFileSelector.h"
#include "frmLocaleSelector.h"
#include "strconv.h"
#include "frmSgaMake.h"
#include "Tool_AESetup.h"
#include "Tool_AutoDPS.h"
#include "frmMessage.h"
#include "Utility.h"
#include "frmMassExtract.h"
#include <memory>
#include <Rainman_RGDDump.h>
#include "Common.h"

wxString CLocaleTool::GetName() {return AppStr(locale);}
wxString CLocaleTool::GetHelpString() {return wxT("");}
wxString CLocaleTool::GetBitmapName() {return wxT("IDB_TOOL_LOC");}
void CLocaleTool::DoAction()
{
	ConstructFrame::eLoadModGames eGame;
	switch(TheConstruct->GetModule()->GetModuleType())
	{
	case CModuleFile::MT_CompanyOfHeroesEarly:
	case CModuleFile::MT_CompanyOfHeroes:
		eGame = ConstructFrame::LM_CoH_OF;
		break;

	case CModuleFile::MT_DawnOfWar:
		eGame = ConstructFrame::LM_DoW_WA;
		break;

	default:
		eGame = ConstructFrame::LM_Any;
		break;
	};

	frmLocaleSelector *pLocaleSelect = new frmLocaleSelector(AppStr(localeselect_title), eGame);
	pLocaleSelect->ShowModal();
	delete pLocaleSelect;
	
	CRefreshFilesTool oReload;
	oReload.DoAction();
}

wxString CAESetupTool::GetName() {return AppStr(aesetup_name);}
wxString CAESetupTool::GetHelpString() {return AppStr(aesetup_help);}
wxString CAESetupTool::GetBitmapName() {return wxT("IDB_TOOL_AESETUP");}
void CAESetupTool::DoAction()
{
	frmUCSToDAT oMake;
	oMake.ShowModal();
}

wxString CUcsTool::GetName() {return AppStr(ucs_editor);}
wxString CUcsTool::GetHelpString() {return wxT("");}
wxString CUcsTool::GetBitmapName() {return wxT("IDB_TOOL_UCS");}

void CUcsTool::HandleSelectorResponse(frmUCSSelector* pSelector, wxAuiNotebook* pTabsDestination, unsigned long* pResult, bool bRegisterTabStrip)
{
	frmUCSEditor* pForm;

	switch(pSelector->ShowModal())
	{
	case wxID_NEW:
		{
		size_t iUCSCount = TheConstruct->GetModule()->GetUcsCount();
		for(size_t i = 0; i < iUCSCount; ++i)
		{
			CModuleFile::CUcsHandler* pUcs = TheConstruct->GetModule()->GetUcs(i);
			if(AsciiTowxString(pUcs->GetFileName()) == pSelector->GetAnswer())
			{
				wxMessageBox(AppStr(file_newucs_dup_caption),AppStr(file_newucs_title),wxICON_ERROR,TheConstruct);
				delete pSelector;
				return;
			}
		}
		wxString sNewFile = TheConstruct->GetModuleFile().BeforeLast('\\');
		sNewFile.Append(wxT("\\"));
		if(TheConstruct->GetModule()->GetModuleType() == CModuleFile::MT_CompanyOfHeroes)
		{
			sNewFile.Append(wxT("Engine\\Locale\\"));
		}
		else
		{
			const char* sLocFolder = TheConstruct->GetModule()->GetLocaleFolder();
			if(sLocFolder && *sLocFolder)
			{
				sNewFile.Append(AsciiTowxString(sLocFolder));
				sNewFile.Append(wxT("\\"));
			}
			else
			{
				sNewFile.Append(AsciiTowxString(TheConstruct->GetModule()->GetModFolder()));
				sNewFile.Append(wxT("\\Locale\\"));
			}
		}
		sNewFile.Append(AsciiTowxString(TheConstruct->GetModule()->GetLocale()));
		sNewFile.Append(wxT("\\"));
		sNewFile.Append(pSelector->GetAnswer());

		char* saNewFile = wxStringToAscii(sNewFile);
		CUcsFile *pNewUcs = new CUcsFile;
		try
		{
			pNewUcs->Save(saNewFile);
		}
		catch(CRainmanException *pE)
		{
			delete[] saNewFile;
			delete pNewUcs;
			delete pSelector;
			ErrorBoxE(pE);
			return;
		}
		delete[] saNewFile;
		saNewFile = wxStringToAscii(pSelector->GetAnswer());
		try
		{
			TheConstruct->GetModule()->NewUCS(saNewFile, pNewUcs);
		}
		catch(CRainmanException *pE)
		{
			delete pNewUcs;
			delete[] saNewFile;
			ErrorBoxE(pE);
			return;
		}
		pTabsDestination->AddPage(pForm = new frmUCSEditor(pTabsDestination, -1, false, wxDefaultPosition, wxDefaultSize, pResult), wxString().Append(AppStr(ucsedit_tabname)).Append(wxT(" [")).Append(pSelector->GetAnswer()).Append(wxT("]")), true);
		pForm->FillFromCUcsFile(pNewUcs);
		delete[] saNewFile;
		break;
		}
	case wxID_OPEN:
		{
		pTabsDestination->AddPage(pForm = new frmUCSEditor(pTabsDestination, -1, pSelector->IsAnswerUcsReadOnly(), wxDefaultPosition, wxDefaultSize, pResult), wxString().Append(AppStr(ucsedit_tabname)).Append(wxT(" [")).Append(pSelector->GetAnswer()).Append(wxT("]")), true);
		pForm->FillFromCUcsFile(pSelector->GetAnswerUcs());
		/*
		size_t iUCSCount = TheConstruct->GetModule()->GetUcsCount();
		for(size_t i = 0; i < iUCSCount; ++i)
		{
			CModuleFile::CUcsHandler* pUcs = TheConstruct->GetModule()->GetUcs(i);
			if(AsciiTowxString(pUcs->GetFileName()) == pSelector->GetAnswer())
			{
				pForm->FillFromCUcsFile(pUcs->GetUcsHandle());
			}
		}
		*/
		break;
		}
	default:
		return; // should never happen?
	};

	if(bRegisterTabStrip)
		pForm->SetTabStripForLoad(pTabsDestination);

	delete pSelector;
}

void CUcsTool::DoAction()
{
	HandleSelectorResponse(new frmUCSSelector(AppStr(ucsselect_title)), TheConstruct->GetTabs());
}

wxString CAttribSnapshotTool::GetName() {return AppStr(xml_export);}
wxString CAttribSnapshotTool::GetHelpString() {return wxT("");}
wxString CAttribSnapshotTool::GetBitmapName() {return wxT("IDB_SNAPSHOT");}
void CAttribSnapshotTool::DoAction()
{
}

wxString CSgaPackerTool::GetName() {return AppStr(sgapack_title);}
wxString CSgaPackerTool::GetHelpString() {return wxT("");}
wxString CSgaPackerTool::GetBitmapName() {return wxT("IDB_SGAPACK");}
void CSgaPackerTool::DoAction()
{
	frmSgaMake oMake;
	oMake.ShowModal();
}

wxString CExtractAllTool::GetName() {return AppStr(massext_toolname);}
wxString CExtractAllTool::GetHelpString() {return wxT("");}
wxString CExtractAllTool::GetBitmapName() {return wxT("IDB_TOOL_EXTALL");}
void CExtractAllTool::DoAction()
{
	frmMassExtract oMassExtract(wxT(""), TheConstruct->GetFilesList()->GetTree()->GetRootItem(), true );
	oMassExtract.ShowModal();
}

wxString CDpsCalculatorTool::GetName() {return AppStr(dpscalculator_title);}
wxString CDpsCalculatorTool::GetHelpString() {return wxT("");}
wxString CDpsCalculatorTool::GetBitmapName() {return wxT("IDB_TOOL_CALCULATOR");}
void CDpsCalculatorTool::DoAction()
{
	wxString sOutFile = wxFileSelector(AppStr(dpscalculator_outselect), wxT(""), wxT(""), wxT("html"), AppStr(dpscalculator_filter) , wxSAVE | wxOVERWRITE_PROMPT, TheConstruct);

	if(sOutFile.Len() == 0) return;

	char* sOut = wxStringToAscii(sOutFile);

	frmMessage *pMsg = new frmMessage(wxT("IDB_TOOL_CALCULATOR"), AppStr(dpscalculator_message));
	pMsg->Show(TRUE);
	wxSafeYield(pMsg);

	CModuleFile* pMod = TheConstruct->GetModule();

	IDirectoryTraverser::IIterator* pItr = 0;
	AutoDPS_Internal::tAutoDPS_Package* pPack = 0;

	try
	{
		try
		{
			pItr = pMod->VIterate("data\\attrib\\weapon");
		}
		catch(CRainmanException *pE)
		{
			throw new CModStudioException(__FILE__, __LINE__, "Cannot iterate over weapon files", pE);
		}

		try
		{
			pPack = AutoDPS::AutoDPS_Scan(pItr);
		}
		catch(CRainmanException *pE)
		{
			throw new CModStudioException(__FILE__, __LINE__, "Error scanning weapon files", pE);
		}

		try
		{
			AutoDPS::AutoDPS_Analyse(pPack);
		}
		catch(CRainmanException *pE)
		{
			throw new CModStudioException(__FILE__, __LINE__, "Error calculating DPS values", pE);
		}

		try
		{
			AutoDPS::AutoDPS_AddUnitList(pPack, pMod);
		}
		catch(CRainmanException *pE)
		{
			throw new CModStudioException(__FILE__, __LINE__, "Error getting unit list", pE);
		}

		try
		{
			AutoDPS::AutoDPS_OutputHTML(pPack, sOut);
		}
		catch(CRainmanException *pE)
		{
			throw new CModStudioException(pE, __FILE__, __LINE__, "Error saving output to \'%s\'", sOut);
		}

		wxMessageBox(AppStr(dpscalculator_done),AppStr(dpscalculator_title),wxICON_INFORMATION,TheConstruct);
	}
	catch(CRainmanException *pE)
	{
		ErrorBoxE(pE);
	}

	if(pPack) delete pPack;
	if(pItr) delete pItr;
	delete[] sOut;
	delete pMsg;
}

wxString CMakeLuaInheritTree::GetName() {return wxT("Make Lua Inheritance Tree");}
wxString CMakeLuaInheritTree::GetHelpString() {return wxT("");}
wxString CMakeLuaInheritTree::GetBitmapName() {return wxT("IDB_REDBUTTON");}
bool CMakeLuaInheritTree::_DoesExist(const char* sFol)
{
	IDirectoryTraverser::IIterator *pItr = 0;

	try
	{
		pItr = TheConstruct->GetModule()->VIterate(sFol);
	}
	catch(CRainmanException* pE)
	{
		pE->destroy();
		return false;
	}

	delete pItr;
	return true;
}

void CMakeLuaInheritTree::_ForEach(IDirectoryTraverser::IIterator* pItr, void* pTag)
{
	const char* sN = pItr->VGetName();
	sN = strrchr(sN, '.');
	if(sN)
	{
		if( (strnicmp(sN, ".lua", 4) == 0) || (strnicmp(sN, ".nil", 4) == 0) )
		{
			CMakeLuaInheritTree* pC = (CMakeLuaInheritTree*)pTag;
			pC->_DoLua(pItr);
		}
	}
}

void CMakeLuaInheritTree::_DoLua(IDirectoryTraverser::IIterator* pItr)
{
	IFileStore::IStream* pStream = pItr->VOpenFile();
	char* sRef = 0;

	CLuaFile oLua;
	try
	{
		sRef = oLua.GetReferencedFile(pStream);
	}
	catch(CRainmanException* pE)
	{
		pE->destroy();
		sRef = 0;
	}

	if(sRef == 0)
	{
		unsigned long iChildCount;
		try
		{
			pStream->VSeek(0, IFileStore::IStream::SL_Root);
			oLua.Load(pStream, TheConstruct->GetModule(), pItr->VGetFullPath());
			iChildCount = oLua.VGetChildCount();
		}
		catch(CRainmanException* pE)
		{
			delete pStream;
			ErrorBoxE(pE);
			return;
		}

		for(unsigned long i = 0; i < iChildCount; ++i)
		{
			IMetaNode* pChild = 0;
			IMetaNode::IMetaTable* pTable = 0;

			try
			{
				pChild = oLua.VGetChild(i);
				if(strcmp(pChild->VGetName(), "GameData") != 0)
				{
					delete pChild;
					continue;
				}
				pTable = pChild->VGetValueMetatable();
				if(pTable->VGetReferenceType() == IMetaNode::DT_String)
				{
					sRef = strdup(pTable->VGetReferenceString());
				}
				else if(pTable->VGetReferenceType() == IMetaNode::DT_WString)
				{
					const wchar_t* pS = pTable->VGetReferenceWString();
					signed long iS = -1;
					sRef = (char*)malloc(wcslen(pS) + 1);
					do
					{
						++iS;
						sRef[iS] = pS[iS];
					}while(pS[iS]);
				}
				else
				{
					sRef = strdup("");
				}
			}
			catch(CRainmanException* pE)
			{
				delete pStream;
				delete pChild;
				delete pTable;
				ErrorBoxE(pE);
				return;
			}
			delete pChild;
			delete pTable;
			break;
		}
		if(sRef == 0)
		{
			delete pStream;
			QUICK_THROW("Cannot find Reference for file");
		}
	}
	delete pStream;

	CInheritTable::CNode* pThis = pTable->findOrMake(pItr->VGetFullPath() + iAttribL);
	CInheritTable::CNode* pReff = pTable->findOrMake(sRef);
	pThis->setParent(pReff);

	free(sRef);
}

CMakeLuaInheritTree::CMakeLuaInheritTree()
{
	pTable = 0;
}

void CMakeLuaInheritTree::Do(const char* sAttrib)
{
	iAttribL = strlen(sAttrib);

	IDirectoryTraverser::IIterator *pItr = 0;

	try
	{
		pItr = TheConstruct->GetModule()->VIterate(sAttrib);
	}
	catch(CRainmanException* pE)
	{
		throw new CModStudioException(__FILE__, __LINE__, "Cannot iterate attrib", pE);
	}

	try
	{
		Rainman_ForEach(pItr, _ForEach, (void*)this, true);
	}
	catch(CRainmanException* pE)
	{
		delete pItr;
		throw new CModStudioException(__FILE__, __LINE__, "Error processing luas", pE);
	}

	pTable->assignOrphansTo(pTable->getRoot());
	delete pItr;
}

void CMakeLuaInheritTree::DoAction()
{
	pTable = new CInheritTable;

	const char* sAttrib = _DoesExist("attrib\\") ? "attrib\\attrib\\" : "data\\attrib\\";
	try
	{
		Do(sAttrib);
	}
	catch(CRainmanException* pE)
	{
		ErrorBoxE(pE);
		delete pTable;
		return;
	}

	delete pTable;
}

wxString CRedButtonTool::GetName() {return AppStr(redbutton_toolname);}
wxString CRedButtonTool::GetHelpString() {return wxT("");}
wxString CRedButtonTool::GetBitmapName() {return wxT("IDB_REDBUTTON");}
void CRedButtonTool::DoAction()
{
	CFileSystemStore oFSO;
	oFSO.VInit();
	try
	{
		delete oFSO.VOpenOutputStream("I:\\j\\k\\l\\m\\n\\o\\p\\q.test", true);
	}
	catch(CRainmanException *pE)
	{
		ErrorBoxE(pE);
	}
	/*
	CIniConfigFile oConfig;
	oConfig.loadFile("E:\\Company of Heroes\\RelicCOH.module");
	oConfig.saveFile("E:\\Company of Heroes\\RelicCOH_.module");
	*/

	/*
	wxString sOutFile = wxFileSelector(wxT("Select strings list"), wxT(""), wxT(""), wxT("txt"), wxT("Text File (*.txt)|*.txt") , wxOPEN, TheConstruct);

	if(sOutFile.Len() == 0) return;

	char* sOut = wxStringToAscii(sOutFile);

	TheConstruct->GetRgdHashTable()->XRefWithStringList(sOut);

	delete[] sOut;
	*/
	/*
	char* sOut = wxStringToAscii(oF.GetBaseFolder() + oF.GetFile());
	IFileStore::IStream* pS = TheConstruct->GetModule()->VOpenStream(sOut);
	delete[] sOut;
	RgdDump::CRgd oRgd;

	oRgd.loadFromStream(pS);
	delete pS;
	*/
}

wxString CRefreshFilesTool::GetName() {return AppStr(refreshfiles_name);}
wxString CRefreshFilesTool::GetHelpString() {return wxT("");}
wxString CRefreshFilesTool::GetBitmapName() {return wxT("IDB_REFRESH");}
void CRefreshFilesTool::DoAction()
{
	CModuleFile* pMod = TheConstruct->GetModule();

	frmLoading* m_pLoadingForm = new frmLoading(AppStr(mod_loading));
	TheConstruct->SetLoadingForm(m_pLoadingForm);

	m_pLoadingForm->Show(TRUE);
	m_pLoadingForm->SetMessage(wxString(wxT("Initializing")));
	wxSafeYield(m_pLoadingForm);
	
	pMod->ReloadResources(CModuleFile::RR_All, CModuleFile::RR_All, CModuleFile::RR_All, ConstructFrame::LoadModCallback);

	m_pLoadingForm->SetMessage(wxString(wxT("Refreshing GUI")));

	TheConstruct->GetFilesList()->FillFromIDirectoryTraverser(pMod);

	m_pLoadingForm->Close(TRUE);
	delete m_pLoadingForm;
	TheConstruct->SetLoadingForm(0);
}