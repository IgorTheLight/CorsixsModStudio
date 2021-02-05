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

static wxString OnlyFilename(wxString sName)
{
	return sName.AfterLast('\\');
}

class CTextViewAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("");
	}

	virtual wxString VGetAction() const
	{
		return wxT("View as text file");
	}
	
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile);
		frmScarEditor* pForm;
		TheConstruct->GetTabs()->AddPage(pForm = new frmScarEditor(oParent, sFile, TheConstruct->GetTabs(), -1), wxString().Append(OnlyFilename(sFile)), true);
		
		char* saFile = wxStringToAscii(sFile);
		IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
		if(!pStream)
		{
			delete[] saFile;
			ErrorBox("Cannot open file");
			return;
		}
		delete[] saFile;

		pForm->Load(pStream);

		delete pStream;
	}
};

class CFilePathCopyAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("");
	}

	virtual wxString VGetAction() const
	{
		return wxT("Copy filename to clipboard");
	}
	
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile); UNUSED(oParent);
		if (wxTheClipboard->Open())
		{
			wxTheClipboard->SetData( new wxTextDataObject(sFile) );
			wxTheClipboard->Close();
		}
	}
};

class CExtractAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("");
	}

	virtual wxString VGetAction() const
	{
		return wxT("Extract this file");
	}
	
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{
		CFilesTreeItemData *pData = (CFilesTreeItemData*)TheConstruct->GetFilesList()->GetTree()->GetItemData(oFile);
		if(stricmp(pData->sMod, TheConstruct->GetModule()->GetFileMapName()) == 0)
		{
			char* sTmp = strrchr(pData->sSource, '.');
			if(!sTmp || (stricmp(sTmp, ".sga") != 0))
			{
				wxMessageBox(AppStr(extract_already),VGetAction(),wxICON_INFORMATION,TheConstruct);
				return;
			}
		}
		char* saFile = wxStringToAscii(sFile);
		try
		{
			DoExtract(saFile);
		}
		catch(CRainmanException *pE)
		{
			ErrorBoxE(pE);
			delete[] saFile;
			return;
		}

		{
			char* saDir = strdup(saFile), *pSlash;
			pSlash = strrchr(saDir, '\\');
			if(pSlash)
				*pSlash = 0;
			else
				*saDir = 0;
			IDirectoryTraverser::IIterator *pDir = TheConstruct->GetModule()->VIterate(saDir);
			free(saDir);
			TheConstruct->GetFilesList()->UpdateDirectoryChildren(oParent, pDir);
			delete pDir;
			wxMessageBox(AppStr(extract_good),VGetAction(),wxICON_INFORMATION,TheConstruct);
		}
		delete[] saFile;
	}

	static void DoExtract(char* saFile, char* p4mbBuffer = 0)
	{
		char* pBuffer = p4mbBuffer;
		long iLen;
		IFileStore::IStream *pIn = 0;
		IFileStore::IOutputStream *pOut = 0;
		CModuleFile* pMod = TheConstruct->GetModule();
		try
		{
			pIn = pMod->VOpenStream(saFile);
			pOut = pMod->VOpenOutputStream(saFile, true);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			delete pOut;
			throw new CModStudioException(pE, __FILE__, __LINE__, "Unable to open streams for \'%s\'", saFile);
		}
		try
		{
			pIn->VSeek(0, IFileStore::IStream::SL_End);
			iLen = pIn->VTell();
			pIn->VSeek(0, IFileStore::IStream::SL_Root);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			delete pOut;
			throw new CModStudioException(pE, __FILE__, __LINE__, "Seek/Tell problem on streams for \'%s\'", saFile);
		}
		if(!pBuffer) pBuffer = new char[4194304]; //4mb
		if(!pBuffer)
		{
			delete pIn;
			delete pOut;
			throw new CModStudioException(__FILE__, __LINE__, "Cannot allocate memory");
		}
		while(iLen)
		{
			long iLen2 = iLen > 4194304 ? 4194304 : iLen;
			try
			{
				pIn->VRead(iLen2, 1, pBuffer);
				pOut->VWrite(iLen2, 1, pBuffer);
			}
			catch(CRainmanException *pE)
			{
				delete pIn;
				delete pOut;
				if(!p4mbBuffer) delete[] pBuffer;
				throw new CModStudioException(pE, __FILE__, __LINE__, "Read/Write problem on streams for \'%s\'", saFile);
			}
			iLen -= iLen2;
		}

		if(!p4mbBuffer) delete[] pBuffer;
		delete pIn;
		delete pOut;
	}
};

class CMakeCopyAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("");
	}

	virtual wxString VGetAction() const
	{
		return wxT("Make a copy of this file");
	}
	
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{
		//CFilesTreeItemData *pData = (CFilesTreeItemData*)TheConstruct->GetFilesList()->GetTree()->GetItemData(oFile);

		wxString sNewName = sFile.AfterLast('\\');
		sNewName = wxString(wxT("Copy of ")).Append(sNewName);
		sNewName = wxGetTextFromUser(wxT("New file will be called:"), VGetAction(), sNewName, TheConstruct, wxDefaultCoord, wxDefaultCoord, false);
		if(sNewName.IsEmpty()) return;
		sNewName = sFile.BeforeLast('\\') + wxT("\\") + sNewName;

		char* saSrcFile = wxStringToAscii(sFile);
		char* saFile = wxStringToAscii(sNewName);
		try
		{
			DoExtract(saSrcFile, saFile);
		}
		catch(CRainmanException *pE)
		{
			ErrorBoxE(pE);
			delete[] saFile;
			delete[] saSrcFile;
			return;
		}

		{
			char* saDir = strdup(saFile), *pSlash;
			pSlash = strrchr(saDir, '\\');
			if(pSlash)
				*pSlash = 0;
			else
				*saDir = 0;
			IDirectoryTraverser::IIterator *pDir = TheConstruct->GetModule()->VIterate(saDir);
			free(saDir);
			TheConstruct->GetFilesList()->UpdateDirectoryChildren(oParent, pDir);
			delete pDir;
			wxMessageBox(wxT("File copied"),VGetAction(),wxICON_INFORMATION,TheConstruct);
		}
		delete[] saFile;
		delete[] saSrcFile;
	}

	static void DoExtract(char* saFile, char* saDestFile, char* p4mbBuffer = 0)
	{
		char* pBuffer = p4mbBuffer;
		long iLen;
		IFileStore::IStream *pIn = 0;
		IFileStore::IOutputStream *pOut = 0;
		CModuleFile* pMod = TheConstruct->GetModule();
		try
		{
			pIn = pMod->VOpenStream(saFile);
			pOut = pMod->VOpenOutputStream(saDestFile, true);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			delete pOut;
			throw new CModStudioException(pE, __FILE__, __LINE__, "Unable to open streams for \'%s\'", saFile);
		}
		try
		{
			pIn->VSeek(0, IFileStore::IStream::SL_End);
			iLen = pIn->VTell();
			pIn->VSeek(0, IFileStore::IStream::SL_Root);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			delete pOut;
			throw new CModStudioException(pE, __FILE__, __LINE__, "Seek/Tell problem on streams for \'%s\'", saFile);
		}
		if(!pBuffer) pBuffer = new char[4194304]; //4mb
		if(!pBuffer)
		{
			delete pIn;
			delete pOut;
			throw new CModStudioException(__FILE__, __LINE__, "Cannot allocate memory");
		}
		while(iLen)
		{
			long iLen2 = iLen > 4194304 ? 4194304 : iLen;
			try
			{
				pIn->VRead(iLen2, 1, pBuffer);
				pOut->VWrite(iLen2, 1, pBuffer);
			}
			catch(CRainmanException *pE)
			{
				delete pIn;
				delete pOut;
				if(!p4mbBuffer) delete[] pBuffer;
				throw new CModStudioException(pE, __FILE__, __LINE__, "Read/Write problem on streams for \'%s\'", saFile);
			}
			iLen -= iLen2;
		}

		if(!p4mbBuffer) delete[] pBuffer;
		delete pIn;
		delete pOut;
	}
};

class CSmfWavAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("smf");
	}

	virtual wxString VGetAction() const
	{
		return wxT("Convert SMF to WAV file");
	}
	
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{
		//CFilesTreeItemData *pData = (CFilesTreeItemData*)TheConstruct->GetFilesList()->GetTree()->GetItemData(oFile);
		char* saFile = wxStringToAscii(sFile);
		try
		{
			DoExtract(saFile);
		}
		catch(CRainmanException *pE)
		{
			ErrorBoxE(pE);
			delete[] saFile;
			return;
		}

		{
			char* saDir = strdup(saFile), *pSlash;
			pSlash = strrchr(saDir, '\\');
			if(pSlash)
				*pSlash = 0;
			else
				*saDir = 0;
			IDirectoryTraverser::IIterator *pDir = TheConstruct->GetModule()->VIterate(saDir);
			free(saDir);
			TheConstruct->GetFilesList()->UpdateDirectoryChildren(oParent, pDir);
			delete pDir;
			wxMessageBox(AppStr(extract_good),VGetAction(),wxICON_INFORMATION,TheConstruct);
		}
		delete[] saFile;
	}

	static void DoExtract(char* saFile, char* p4mbBuffer = 0)
	{ UNUSED(saFile); UNUSED(p4mbBuffer);
		/*
		char* pBuffer = p4mbBuffer;
		long iLen;
		IFileStore::IStream *pIn = 0;
		IFileStore::IOutputStream *pOut = 0;
		CModuleFile* pMod = TheConstruct->GetModule();
		try
		{
			pIn = pMod->VOpenStream(saFile);
			char *sDotPos
			pOut = pMod->VOpenOutputStream(saFile, true);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			delete pOut;
			throw new CModStudioException(pE, __FILE__, __LINE__, "Unable to open streams for \'%s\'", saFile);
		}
		try
		{
			pIn->VSeek(0, IFileStore::IStream::SL_End);
			iLen = pIn->VTell();
			pIn->VSeek(0, IFileStore::IStream::SL_Root);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			delete pOut;
			throw new CModStudioException(pE, __FILE__, __LINE__, "Seek/Tell problem on streams for \'%s\'", saFile);
		}
		if(!pBuffer) pBuffer = new char[4194304]; //4mb
		if(!pBuffer)
		{
			delete pIn;
			delete pOut;
			throw new CModStudioException(__FILE__, __LINE__, "Cannot allocate memory");
		}
		while(iLen)
		{
			long iLen2 = iLen > 4194304 ? 4194304 : iLen;
			try
			{
				pIn->VRead(iLen2, 1, pBuffer);
				pOut->VWrite(iLen2, 1, pBuffer);
			}
			catch(CRainmanException *pE)
			{
				delete pIn;
				delete pOut;
				if(!p4mbBuffer) delete[] pBuffer;
				throw new CModStudioException(pE, __FILE__, __LINE__, "Read/Write problem on streams for \'%s\'", saFile);
			}
			iLen -= iLen2;
		}

		if(!p4mbBuffer) delete[] pBuffer;
		delete pIn;
		delete pOut;
		*/
	}
};

class CRGODeBurnAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("rgo");
	}

	virtual wxString VGetAction() const
	{
		return wxT("Convert RGO to DataGeneric-RGO file");
	}
	
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{
		//CFilesTreeItemData *pData = (CFilesTreeItemData*)TheConstruct->GetFilesList()->GetTree()->GetItemData(oFile);
		char* saFile = wxStringToAscii(sFile);
		try
		{
			DoDeBurn(saFile);
		}
		catch(CRainmanException *pE)
		{
			ErrorBoxE(pE);
			delete[] saFile;
			return;
		}

		{
			char* saDir = strdup(saFile), *pSlash;
			pSlash = strrchr(saDir, '\\');
			if(pSlash)
				*pSlash = 0;
			else
				*saDir = 0;
			IDirectoryTraverser::IIterator *pDir = TheConstruct->GetModule()->VIterate(saDir);
			free(saDir);
			TheConstruct->GetFilesList()->UpdateDirectoryChildren(oParent, pDir);
			delete pDir;
			wxMessageBox(AppStr(rgodeburn_good),VGetAction(),wxICON_INFORMATION,TheConstruct);
		}
		delete[] saFile;
	}

	static void DoDeBurn(char* saFile)
	{
		IFileStore::IStream *pIn = 0;
		CModuleFile* pMod = TheConstruct->GetModule();
		try
		{
			pIn = pMod->VOpenStream(saFile);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			throw new CModStudioException(pE, __FILE__, __LINE__, "Unable to open streams for \'%s\'", saFile);
		}
		CChunkyFile oChunky;
		try
		{
			oChunky.Load(pIn);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			throw new CModStudioException(pE, __FILE__, __LINE__, "Unable to load \'%s\' as chunky", saFile);
		}
		delete pIn; pIn = 0;
		// 1 : find and remove FOLDMODL\DATAINFO
		CChunkyFile::CChunk *pModl = oChunky.GetChildByName("MODL", CChunkyFile::CChunk::T_Folder);
		if(pModl)
		{
			size_t iL = pModl->GetChildCount();
			for(size_t i = 0; i < iL; ++i)
			{
				CChunkyFile::CChunk *pChild = pModl->GetChild(i);
				if(pChild->GetType() == CChunkyFile::CChunk::T_Data && strcmp(pChild->GetName(), "INFO") == 0)
				{
					pModl->RemoveChild(i);
					break;
				}
			}
		}
		// 2: find and remove DATAFBIF
		size_t iL = oChunky.GetChildCount();
		for(size_t i = 0; i < iL; ++i)
		{
			CChunkyFile::CChunk *pChild = oChunky.GetChild(i);
			if(pChild->GetType() == CChunkyFile::CChunk::T_Data && strcmp(pChild->GetName(), "FBIF") == 0)
			{
				oChunky.RemoveChild(i);
				break;
			}
		}
		// 3 : save
		IFileStore::IOutputStream* pOut = 0;
		try
		{
			pOut = pMod->VOpenOutputStream(saFile, true);
		}
		catch(CRainmanException *pE)
		{
			delete pOut;
			throw new CModStudioException(pE, __FILE__, __LINE__, "Unable to open output stream for \'%s\'", saFile);
		}
		try
		{
			oChunky.Save(pOut);
		}
		catch(CRainmanException *pE)
		{
			delete pOut;
			throw new CModStudioException(pE, __FILE__, __LINE__, "Unable to save \'%s\' as chunky", saFile);
		}
		delete pOut;
	}
};

class CRgdToLuaDumpAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("rgd");
	}

	virtual wxString VGetAction() const
	{
		return wxT("Dump RGD to Lua");
	}
	
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{
		//CFilesTreeItemData *pData = (CFilesTreeItemData*)TheConstruct->GetFilesList()->GetTree()->GetItemData(oFile);
		char* saFile = wxStringToAscii(sFile);
		try
		{
			DoConvert(saFile);
		}
		catch(CRainmanException *pE)
		{
			ErrorBoxE(pE);
			delete[] saFile;
			return;
		}

		char* saDir = strdup(saFile), *pSlash;
		pSlash = strrchr(saDir, '\\');
		if(pSlash)
			*pSlash = 0;
		else
			*saDir = 0;
		IDirectoryTraverser::IIterator *pDir = TheConstruct->GetModule()->VIterate(saDir);
		free(saDir);
		TheConstruct->GetFilesList()->UpdateDirectoryChildren(oParent, pDir);
		delete pDir;
		wxMessageBox(AppStr(luadump_good),VGetAction(),wxICON_INFORMATION,TheConstruct);
	}

	static void DoConvert(char* saFile)
	{
		char* saOutFile = strdup(saFile);
		if(!saOutFile) throw new CModStudioException(__FILE__, __LINE__, "Memory allocation error");
		strcpy(strchr(saOutFile, '.'), ".lua");

		IFileStore::IStream *pIn = 0;
		IFileStore::IOutputStream *pOut = 0;
		CModuleFile* pMod = TheConstruct->GetModule();
		try
		{
			pIn = pMod->VOpenStream(saFile);
			pOut = pMod->VOpenOutputStream(saOutFile, true);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			delete pOut;
			free(saOutFile);
			throw new CModStudioException(pE, __FILE__, __LINE__, "Cannot open streams for \'%s\'", saFile);
		}

		CRgdFile oRgd;
		oRgd.SetHashTable(TheConstruct->GetRgdHashTable());
		try
		{
			oRgd.Load(pIn);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			delete pOut;
			free(saOutFile);
			throw new CModStudioException(pE, __FILE__, __LINE__, "Cannot load file \'%s\'", saFile);
		}

		try
		{
			//MakeLuaFromRgdQuickly(&oRgd, pOut);
			MakeLuaFromRgdAndNil(&oRgd, 0, pMod, pOut, pMod);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			delete pOut;
			free(saOutFile);
			throw new CModStudioException(pE, __FILE__, __LINE__, "Cannot turn file into lua \'%s\'", saFile);
		}

		delete pIn;
		delete pOut;
		free(saOutFile);
	}
};

class CBfxToLuaDumpAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("bfx");
	}

	virtual wxString VGetAction() const
	{
		return wxT("Dump BFX to Lua");
	}
	
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{
		//CFilesTreeItemData *pData = (CFilesTreeItemData*)TheConstruct->GetFilesList()->GetTree()->GetItemData(oFile);
		char* saFile = wxStringToAscii(sFile);
		try
		{
			DoConvert(saFile);
		}
		catch(CRainmanException *pE)
		{
			ErrorBoxE(pE);
			delete[] saFile;
			return;
		}

		char* saDir = strdup(saFile), *pSlash;
		pSlash = strrchr(saDir, '\\');
		if(pSlash)
			*pSlash = 0;
		else
			*saDir = 0;
		IDirectoryTraverser::IIterator *pDir = TheConstruct->GetModule()->VIterate(saDir);
		free(saDir);
		TheConstruct->GetFilesList()->UpdateDirectoryChildren(oParent, pDir);
		delete pDir;
		wxMessageBox(AppStr(bfx_convertgood),VGetAction(),wxICON_INFORMATION,TheConstruct);
		delete[] saFile;
	}

	static void DoConvert(char* saFile, lua_State *L = 0)
	{
		char* saOutFile = strdup(saFile);
		if(!saOutFile) throw new CModStudioException(__FILE__, __LINE__, "Memory allocation error");
		strcpy(strchr(saOutFile, '.'), ".lua");

		IFileStore::IStream *pIn = 0;
		IFileStore::IOutputStream *pOut = 0;
		CModuleFile* pMod = TheConstruct->GetModule();
		try
		{
			pIn = pMod->VOpenStream(saFile);
			pOut = pMod->VOpenOutputStream(saOutFile, true);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			delete pOut;
			free(saOutFile);
			throw new CModStudioException(pE, __FILE__, __LINE__, "Cannot open streams for \'%s\'", saFile);
		}

		CBfxFile oRgd;
		oRgd.SetHashTable(TheConstruct->GetRgdHashTable());
		try
		{
			oRgd.Load(pIn);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			delete pOut;
			free(saOutFile);
			throw new CModStudioException(pE, __FILE__, __LINE__, "Cannot load file \'%s\'", saFile);
		}

		try
		{
			oRgd.SaveAsBfxLua(pOut, L);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			delete pOut;
			free(saOutFile);
			throw new CModStudioException(pE, __FILE__, __LINE__, "Cannot turn file into lua \'%s\'", saFile);
		}

		delete pIn;
		delete pOut;
		free(saOutFile);
	}
};

class CBFXRGTDeBurnAction : public frmFiles::IHandler
{
protected:
	wxProgressDialog* m_pProgress;
public:
	CBFXRGTDeBurnAction()
		: m_pProgress(0)
	{
	}
	virtual wxString VGetExt() const
	{
		return wxT("");
	}

	virtual wxString VGetAction() const
	{
		return wxT("Unburn all BFX/RGT files in this folder");
	}

	virtual size_t Recurse(wxString sFolder, wxTreeItemId& oParent, bool bCountOnly, size_t iCountBase, size_t iCountDiv)
	{
		size_t iCount = 0;
		wxTreeItemIdValue oCookie;
		wxTreeCtrl *pTree = TheConstruct->GetFilesList()->GetTree();
		wxTreeItemId oChild = pTree->GetFirstChild(oParent, oCookie);
		while(oChild.IsOk())
		{
			CFilesTreeItemData *pData = (CFilesTreeItemData*)pTree->GetItemData(oChild);
			if(pData->sMod)
			{
				wxString sFile = pTree->GetItemText(oChild);
				if(sFile.AfterLast('.').IsSameAs(wxT("bfx"), false) || sFile.AfterLast('.').IsSameAs(wxT("rgt"), false))
				{
					++iCount;
					if(!bCountOnly)
					{
						int iPVal = (int)((iCountBase + iCount) / iCountDiv);
						m_pProgress->Update(iPVal, sFile);

						sFile = sFolder + wxT("\\") + pTree->GetItemText(oChild);
						char* saFile = wxStringToAscii(sFile);

						if(sFile.AfterLast('.').IsSameAs(wxT("bfx"), false))
						{
							CBfxToLuaDumpAction::DoConvert(saFile, L);
						}
						else
						{
							IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
							if(!pStream)
							{
								delete[] saFile;
								ErrorBox("Cannot open file");
								return iCount;
							}

							CRgtFile* pRgt = new CRgtFile;
							pRgt->Load(pStream);
							pRgt->setDXTCodec((CRgtFile::tfnDXTCodec)TheConstruct->oSquishLibrary.GetSymbol(wxT("CompressImage")), (CRgtFile::tfnDXTCodec)TheConstruct->oSquishLibrary.GetSymbol(wxT("DecompressImage")));

							strcpy(strrchr(saFile,'.'),".tga");
							IFileStore::IOutputStream* pOutStr = TheConstruct->GetModule()->VOpenOutputStream(saFile, true);
							pRgt->SaveTGA(pOutStr);

							delete pRgt;
							delete pOutStr;
							delete pStream;
						}

						delete[] saFile;
					}
				}
			}
			else
			{
				wxString sNewPath = sFolder;
				sNewPath.Append('\\');
				sNewPath.Append(pTree->GetItemText(oChild));
				iCount += Recurse(sNewPath, oChild, bCountOnly, iCountBase + iCount, iCountDiv);
			}
			oChild = pTree->GetNextChild(oChild, oCookie);
		}
		if(!bCountOnly)
		{
			if(pTree->IsExpanded(oParent))
			{
				char* saPath = wxStringToAscii(sFolder);
				IDirectoryTraverser::IIterator *pDir = TheConstruct->GetModule()->VIterate(saPath);
				delete[] saPath;
				TheConstruct->GetFilesList()->UpdateDirectoryChildren(oParent, pDir);
				delete pDir;
			}
		}
		return iCount;
	}

	lua_State *L;
	void load_lua()
	{
		L = lua_open();
		luaopen_base(L);

		FILE* f = _wfopen(AppStr(app_bfxmapfile), wxT("rb"));
		unsigned long iSizeComp, iSizeDecomp;
		fread(&iSizeDecomp, sizeof(unsigned long), 1, f);
		fread(&iSizeComp, sizeof(unsigned long), 1, f);
		unsigned char *pCompressed = new unsigned char[iSizeComp];
		fread(pCompressed, 1, iSizeComp, f);
		fclose(f);

		unsigned char *pUncompressed = new unsigned char[iSizeDecomp];
		uncompress(pUncompressed, &iSizeDecomp, pCompressed, iSizeComp);
		delete[] pCompressed;

		int iE = luaL_loadbuffer(L, (const char*)pUncompressed, iSizeDecomp, "map");
		delete[] pUncompressed;

		if(iE == 0)
		{
			iE = lua_pcall(L, 0, 0, 0);
			if(iE == 0) lua_getglobal(L, "map");
		}
		if(iE)
		{
			const char* sE = lua_tostring(L, -1);
			sE = sE;
		}
	}

	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile);
		load_lua();

		size_t iCount = Recurse(sFile, oParent, true, 0, 1);
		size_t iDiv = 1;
		while((iCount / iDiv) > (size_t)0x200)
		{
			++iDiv;
		}
		m_pProgress = new wxProgressDialog(VGetAction(), wxT(""), (int)(iCount / iDiv) + 1, TheConstruct, wxPD_SMOOTH | wxPD_AUTO_HIDE | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME);
		Recurse(sFile, oParent, false, 0, iDiv);
		wxMessageBox(AppStr(bfxrgt_convertgood),VGetAction(),wxICON_INFORMATION,TheConstruct);
		delete m_pProgress;

		lua_close(L);
	}
};

class CRgtViewAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("rgt");
	}

	virtual wxString VGetAction() const
	{
		return wxT("View RGT");
	}

	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile);
		char* saFile = wxStringToAscii(sFile);
		IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
		if(!pStream)
		{
			delete[] saFile;
			ErrorBox("Cannot open file");
			return;
		}
		delete[] saFile;

		CRgtFile* pRgt = new CRgtFile;
		pRgt->Load(pStream);
		pRgt->setDXTCodec((CRgtFile::tfnDXTCodec)TheConstruct->oSquishLibrary.GetSymbol(wxT("CompressImage")), (CRgtFile::tfnDXTCodec)TheConstruct->oSquishLibrary.GetSymbol(wxT("DecompressImage")));

		delete pStream;

		frmImageViewer* pForm;
		TheConstruct->GetTabs()->AddPage(pForm = new frmImageViewer(oParent, sFile, TheConstruct->GetTabs(), -1, pRgt, true), wxString().Append(wxT("RGT")).Append(wxT(" [")).Append(OnlyFilename(sFile)).Append(wxT("]")), true);
	}
};

class CDdsViewAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("dds");
	}

	virtual wxString VGetAction() const
	{
		return wxT("View DDS");
	}

	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile);
		char* saFile = wxStringToAscii(sFile);
		IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
		if(!pStream)
		{
			delete[] saFile;
			ErrorBox("Cannot open file");
			return;
		}
		delete[] saFile;

		CRgtFile* pRgt = new CRgtFile;
		pRgt->LoadDDS(pStream);
		pRgt->setDXTCodec((CRgtFile::tfnDXTCodec)TheConstruct->oSquishLibrary.GetSymbol(wxT("CompressImage")), (CRgtFile::tfnDXTCodec)TheConstruct->oSquishLibrary.GetSymbol(wxT("DecompressImage")));

		delete pStream;

		frmImageViewer* pForm;
		TheConstruct->GetTabs()->AddPage(pForm = new frmImageViewer(oParent, sFile, TheConstruct->GetTabs(), -1, pRgt, true), wxString().Append(wxT("DDS")).Append(wxT(" [")).Append(OnlyFilename(sFile)).Append(wxT("]")), true);
		pForm->SetIsDds();
	}
};

class CTgaViewAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("tga");
	}

	virtual wxString VGetAction() const
	{
		return wxT("View TGA");
	}

	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile);
		char* saFile = wxStringToAscii(sFile);
		IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
		if(!pStream)
		{
			delete[] saFile;
			ErrorBox("Cannot open file");
			return;
		}
		delete[] saFile;

		bool bIs32 = true;
		CRgtFile* pRgt = new CRgtFile;
		pRgt->LoadTGA(pStream, false, &bIs32);
		pRgt->setDXTCodec((CRgtFile::tfnDXTCodec)TheConstruct->oSquishLibrary.GetSymbol(wxT("CompressImage")), (CRgtFile::tfnDXTCodec)TheConstruct->oSquishLibrary.GetSymbol(wxT("DecompressImage")));

		delete pStream;

		frmImageViewer* pForm;
		TheConstruct->GetTabs()->AddPage(pForm = new frmImageViewer(oParent, sFile, TheConstruct->GetTabs(), -1, pRgt, true), wxString().Append(wxT("TGA")).Append(wxT(" [")).Append(OnlyFilename(sFile)).Append(wxT("]")), true);
		pForm->SetIsTga(!bIs32);
	}
};

class CRgtToGenericAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("rgt");
	}

	virtual wxString VGetAction() const
	{
		return wxT("Dump RGT to TGA/DDS");
	}
	
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{
		//CFilesTreeItemData *pData = (CFilesTreeItemData*)TheConstruct->GetFilesList()->GetTree()->GetItemData(oFile);
		char* saFile = wxStringToAscii(sFile);
		wxString sConvertedTo;
		try
		{
			sConvertedTo = DoConvert(saFile);
		}
		catch(CRainmanException *pE)
		{
			ErrorBoxE(pE);
			delete[] saFile;
			return;
		}

		char* saDir = strdup(saFile), *pSlash;
		pSlash = strrchr(saDir, '\\');
		if(pSlash)
			*pSlash = 0;
		else
			*saDir = 0;
		IDirectoryTraverser::IIterator *pDir = TheConstruct->GetModule()->VIterate(saDir);
		free(saDir);
		TheConstruct->GetFilesList()->UpdateDirectoryChildren(oParent, pDir);
		delete pDir;

		wxMessageBox(wxString(wxT("")).Append(AppStr(rgt_convertgood)).Append(sConvertedTo) ,VGetAction(),wxICON_INFORMATION,TheConstruct);
	}

	/*!
		\return Returns a string describing the type of type created, eg. "DDS DXT1", "TGA 32bit RGBA"
	*/
	static wxString DoConvert(char* saFile)
	{
		IFileStore::IStream *pIn = 0;
		IFileStore::IOutputStream *pOut = 0;
		wxString sOutFormat;
		CModuleFile* pMod = TheConstruct->GetModule();
		try
		{
			pIn = pMod->VOpenStream(saFile);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			throw new CModStudioException(pE, __FILE__, __LINE__, "Cannot open input stream for \'%s\'", saFile);
		}

		CRgtFile oRgt;
		try
		{
			oRgt.Load(pIn);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			throw new CModStudioException(pE, __FILE__, __LINE__, "Cannot load file \'%s\'", saFile);
		}

		char* saOutFile = strdup(saFile);
		if(!saOutFile) throw new CModStudioException(__FILE__, __LINE__, "Memory allocation error");
		switch(oRgt.GetImageFormat())
		{
		case CRgtFile::IF_Tga:
			strcpy(strchr(saOutFile, '.'), ".tga");
			sOutFormat = wxT("TGA 32bit RGBA");
			break;
		case CRgtFile::IF_Dxtc:
			oRgt.setDXTCodec((CRgtFile::tfnDXTCodec)TheConstruct->oSquishLibrary.GetSymbol(wxT("CompressImage")), (CRgtFile::tfnDXTCodec)TheConstruct->oSquishLibrary.GetSymbol(wxT("DecompressImage")));
			strcpy(strchr(saOutFile, '.'), ".dds");
			sOutFormat = wxT("DDS ");
			switch(oRgt.GetProperty(CRgtFile::IP_CompressionLevel))
			{
			case 1:
				sOutFormat.Append(wxT("DXT1"));
				break;

			case 2:
				sOutFormat.Append(wxT("DXT2"));
				break;

			case 3:
				sOutFormat.Append(wxT("DXT3"));
				break;

			case 4:
				sOutFormat.Append(wxT("DXT4"));
				break;

			case 5:
				sOutFormat.Append(wxT("DXT5"));
				break;

			default:
				sOutFormat.Append(wxT("unknown compression"));
				break;
			};
			break;
		default:
			delete pIn;
			free(saOutFile);
			throw new CModStudioException(0, __FILE__, __LINE__, "Uknown image format of \'%s\'", saFile);
		}
		if(oRgt.GetProperty(CRgtFile::IP_MipLevelCount) > 1)
		{
			sOutFormat.Append(wxT(" with mip levels"));
		}

		try
		{
			pOut = pMod->VOpenOutputStream(saOutFile, true);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			free(saOutFile);
			throw new CModStudioException(pE, __FILE__, __LINE__, "Cannot open output stream for \'%s\'", saFile);
		}

		try
		{
			//oRgt.SaveTGA(pOut);
			oRgt.SaveGeneric(pOut);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			delete pOut;
			free(saOutFile);
			throw new CModStudioException(pE, __FILE__, __LINE__, "Cannot save generic of \'%s\'", saFile);
		}

		delete pIn;
		delete pOut;
		free(saOutFile);
		return sOutFormat;
	}
};

class CDdsToRgtAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("dds");
	}

	virtual wxString VGetAction() const
	{
		return wxT("Convert DDS to RGT");
	}
	
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{
		//CFilesTreeItemData *pData = (CFilesTreeItemData*)TheConstruct->GetFilesList()->GetTree()->GetItemData(oFile);
		char* saFile = wxStringToAscii(sFile);
		wxString sConvertedTo;
		try
		{
			DoConvert(saFile);
		}
		catch(CRainmanException *pE)
		{
			ErrorBoxE(pE);
			delete[] saFile;
			return;
		}

		char* saDir = strdup(saFile), *pSlash;
		pSlash = strrchr(saDir, '\\');
		if(pSlash)
			*pSlash = 0;
		else
			*saDir = 0;
		IDirectoryTraverser::IIterator *pDir = TheConstruct->GetModule()->VIterate(saDir);
		free(saDir);
		TheConstruct->GetFilesList()->UpdateDirectoryChildren(oParent, pDir);
		delete pDir;

		wxMessageBox(AppStr(rgt_makefromddsgood) ,VGetAction(),wxICON_INFORMATION,TheConstruct);
	}

	static void DoConvert(char* saFile)
	{
		IFileStore::IStream *pIn = 0;
		IFileStore::IOutputStream *pOut = 0;
		wxString sOutFormat;
		CModuleFile* pMod = TheConstruct->GetModule();
		try
		{
			pIn = pMod->VOpenStream(saFile);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			throw new CModStudioException(pE, __FILE__, __LINE__, "Cannot open input stream for \'%s\'", saFile);
		}

		CRgtFile oRgt;
		try
		{
			oRgt.LoadDDS(pIn);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			throw new CModStudioException(pE, __FILE__, __LINE__, "Cannot load file \'%s\'", saFile);
		}

		char* saOutFile = strdup(saFile);
		if(!saOutFile) throw new CModStudioException(__FILE__, __LINE__, "Memory allocation error");
		strcpy(strchr(saOutFile, '.'), ".rgt");

		try
		{
			pOut = pMod->VOpenOutputStream(saOutFile, true);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			free(saOutFile);
			throw new CModStudioException(pE, __FILE__, __LINE__, "Cannot open output stream for \'%s\'", saFile);
		}

		try
		{
			oRgt.Save(pOut);
		}
		catch(CRainmanException *pE)
		{
			delete pIn;
			delete pOut;
			free(saOutFile);
			throw new CModStudioException(pE, __FILE__, __LINE__, "Cannot save file \'%s\'", saFile);
		}

		delete pIn;
		delete pOut;
		free(saOutFile);
	}
};

class CExtractFolderAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("");
	}

	virtual wxString VGetAction() const
	{
		return wxT("Extract all files in this folder");
	}

	virtual void VHandle(wxString sFolder, wxTreeItemId& oParent, wxTreeItemId& oFolder)
	{ UNUSED(oParent);
		frmMassExtract oMassExtract(sFolder, oFolder);
		oMassExtract.ShowModal();
	}
};

class CRgdMacroAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("");
	}

	virtual wxString VGetAction() const
	{
		return wxT("Run macro over the RGDs in this folder");
	}

	virtual void VHandle(wxString sFolder, wxTreeItemId& oParent, wxTreeItemId& oFolder)
	{ UNUSED(oParent);
		frmRgdMacro oMacro(sFolder, oFolder);
		oMacro.ShowModal();
	}
};

class CRGDAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("rgd");
	}

	virtual wxString VGetAction() const
	{
		return wxT("View as RGD file");
	}
	
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile);
		frmRGDEditor* pForm;
		TheConstruct->GetTabs()->AddPage(pForm = new frmRGDEditor(oParent, sFile, TheConstruct->GetTabs(), -1), wxString().Append(wxT("RGD")).Append(wxT(" [")).Append(OnlyFilename(sFile)).Append(wxT("]")), true);
		
		char* saFile = wxStringToAscii(sFile);
		IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
		if(!pStream)
		{
			delete[] saFile;
			ErrorBox("Cannot open file");
			return;
		}
		delete[] saFile;

		CRgdFile *pRgd = new CRgdFile;
		pRgd->SetHashTable(TheConstruct->GetRgdHashTable());
		pRgd->Load(pStream);

		delete pStream;

		if(!pForm->FillFromMetaNode(pRgd))
		{
			ErrorBox("Cannot load file");
			delete pRgd;
			return;
		}
	}
};

class CRGMMaterialAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("rgm");
	}

	virtual wxString VGetAction() const
	{
		return wxT("Set RGM textures");
	}
	
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile);
		frmRgmMaterialEditor* pForm;
		TheConstruct->GetTabs()->AddPage(pForm = new frmRgmMaterialEditor(oParent, sFile, TheConstruct->GetTabs(), -1), wxString().Append(wxT("RGM")).Append(wxT(" [")).Append(OnlyFilename(sFile)).Append(wxT("]")), true);
		
		char* saFile = wxStringToAscii(sFile);
		IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
		if(!pStream)
		{
			delete[] saFile;
			ErrorBox("Cannot open file");
			return;
		}
		delete[] saFile;

		CRgmFile *pRgm = new CRgmFile;
		pRgm->Load(pStream);

		delete pStream;

		pForm->SetObject(pRgm, true);
	}
};

class CMuaxAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("muax");
	}

	virtual wxString VGetAction() const
	{
		return wxT("View as MUAX file");
	}
	
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile);
		frmRGDEditor* pForm;
		TheConstruct->GetTabs()->AddPage(pForm = new frmRGDEditor(oParent, sFile, TheConstruct->GetTabs(), -1), wxString().Append(wxT("MUAX")).Append(wxT(" [")).Append(OnlyFilename(sFile)).Append(wxT("]")), true);
		
		char* saFile = wxStringToAscii(sFile);
		IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
		if(!pStream)
		{
			delete[] saFile;
			ErrorBox("Cannot open file");
			return;
		}
		delete[] saFile;

		CRgdFile *pRgd = new CRgdFile;
		pRgd->SetHashTable(TheConstruct->GetRgdHashTable());
		pRgd->Load(pStream);

		delete pStream;

		if(!pForm->FillFromMetaNode(pRgd))
		{
			ErrorBox("Cannot load file");
			delete pRgd;
			return;
		}
	}
};

class CBfxAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("bfx");
	}

	virtual wxString VGetAction() const
	{
		return wxT("View as BFX file");
	}
	
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile);
		frmRGDEditor* pForm;
		TheConstruct->GetTabs()->AddPage(pForm = new frmRGDEditor(oParent, sFile, TheConstruct->GetTabs(), -1), wxString().Append(wxT("BFX")).Append(wxT(" [")).Append(OnlyFilename(sFile)).Append(wxT("]")), true);
		
		char* saFile = wxStringToAscii(sFile);
		IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
		if(!pStream)
		{
			delete[] saFile;
			ErrorBox("Cannot open file");
			return;
		}
		delete[] saFile;

		CRgdFile *pRgd = new CRgdFile;
		pRgd->SetHashTable(TheConstruct->GetRgdHashTable());
		pRgd->Load(pStream);

		delete pStream;

		if(!pForm->FillFromMetaNode(pRgd))
		{
			ErrorBox("Cannot load file");
			delete pRgd;
			return;
		}
	}
};

wxString CLuaAction::VGetExt() const
{
	return wxT("lua");
}

wxString CLuaAction::VGetAction() const
{
	return wxT("View as LUA file");
}

CLuaFile* CLuaAction::DoLoad(IFileStore::IStream *pStream, const char* sFile, CLuaFileCache* pCache)
{
	CLuaFile *pLua = new CLuaFile;
	if(!pLua)
	{
		ErrorBoxAS(err_memory);
		return 0;
	}

	pLua->AssignCache(pCache);
	try
	{
		pLua->Load(pStream, TheConstruct->GetModule(), sFile);
	}
	catch(CRainmanException *pE)
	{
		ErrorBoxE(new CModStudioException(pE, __FILE__, __LINE__, "Cannot load Lua file \'%s\'", sFile));
		delete pLua;
		return 0;
	}

	return pLua;
}

CLuaFile2* CLuaAction::DoLoad2(IFileStore::IStream *pStream, const char* sFile, CLuaFileCache* pCache)
{
	CLuaFile2 *pLua = new CLuaFile2;
	if(!pLua)
	{
		ErrorBoxAS(err_memory);
		return 0;
	}

	pLua->setCache(pCache, false);
	if(sFile[0] == 'G' || sFile[0] == 'g') pLua->setRootFolder("generic\\attrib\\");
	else if(sFile[0] == 'A' || sFile[0] == 'a') pLua->setRootFolder("attrib\\attrib\\");
	else pLua->setRootFolder("data\\attrib\\");

	try
	{
		pLua->loadFile(pStream, TheConstruct->GetModule(), sFile);
	}
	catch(CRainmanException *pE)
	{
		ErrorBoxE(pE);
		delete pLua;
		return 0;
	}

	return pLua;
}

void CLuaAction::VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
{ UNUSED(oFile);
	char* saFile = wxStringToAscii(sFile);
	IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
	if(!pStream)
	{
		delete[] saFile;
		ErrorBox("Cannot open file");
		return;
	}
	CLuaFile2 *pLua = DoLoad2(pStream, saFile);
	delete[] saFile;
	delete pStream;
	if(pLua)
	{
		frmRGDEditor* pForm;
		TheConstruct->GetTabs()->AddPage(pForm = new frmRGDEditor(oParent, sFile, TheConstruct->GetTabs(), -1), wxString().Append(wxT("LUA")).Append(wxT(" [")).Append(OnlyFilename(sFile)).Append(wxT("]")), true);
		if(!pForm->FillFromLua2(pLua))
		{
			ErrorBox("Cannot load file");
			delete pLua;
			return;
		}
	}
}

class CAbpAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("abp");
	}

	virtual wxString VGetAction() const
	{
		return wxT("View as LUA/ABP file");
	}
	
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile);
		/*
		char* saFile = wxStringToAscii(sFile);
		IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
		if(!pStream)
		{
			delete[] saFile;
			ErrorBox("Cannot open file");
			return;
		}
		CLuaFile *pLua = CLuaAction::DoLoad(pStream, saFile);
		delete[] saFile;
		delete pStream;
		if(pLua)
		{
			frmRGDEditor* pForm;
			TheConstruct->GetTabs()->AddPage(pForm = new frmRGDEditor(oParent, sFile, TheConstruct->GetTabs(), -1), wxString().Append(wxT("ABP")).Append(wxT(" [")).Append(OnlyFilename(sFile)).Append(wxT("]")), true);
			if(!pForm->FillFromMetaTable(pLua))
			{
				ErrorBox("Cannot load file");
				delete pLua;
				return;
			}
		}
		*/
		frmScarEditor* pForm;
		TheConstruct->GetTabs()->AddPage(pForm = new frmScarEditor(oParent, sFile, TheConstruct->GetTabs(), -1), wxString().Append(wxT("ABP")).Append(wxT(" [")).Append(OnlyFilename(sFile)).Append(wxT("]")), true);
		
		char* saFile = wxStringToAscii(sFile);
		IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
		if(!pStream)
		{
			delete[] saFile;
			ErrorBox("Cannot open file");
			return;
		}
		delete[] saFile;

		pForm->Load(pStream);

		delete pStream;
	}
};

class CMuaAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("mua");
	}

	virtual wxString VGetAction() const
	{
		return wxT("View as LUA/MUA file");
	}
	
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile);
		char* saFile = wxStringToAscii(sFile);
		IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
		if(!pStream)
		{
			delete[] saFile;
			ErrorBox("Cannot open file");
			return;
		}
		CLuaFile *pLua = CLuaAction::DoLoad(pStream, saFile);
		delete[] saFile;
		delete pStream;
		if(pLua)
		{
			frmRGDEditor* pForm;
			TheConstruct->GetTabs()->AddPage(pForm = new frmRGDEditor(oParent, sFile, TheConstruct->GetTabs(), -1), wxString().Append(wxT("MUA")).Append(wxT(" [")).Append(OnlyFilename(sFile)).Append(wxT("]")), true);
			if(!pForm->FillFromMetaTable(pLua))
			{
				ErrorBox("Cannot load file");
				delete pLua;
				return;
			}
		}
	}
};

class CSuaAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("sua");
	}

	virtual wxString VGetAction() const
	{
		return wxT("View as LUA/SUA file");
	}
	
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile);
		char* saFile = wxStringToAscii(sFile);
		IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
		if(!pStream)
		{
			delete[] saFile;
			ErrorBox("Cannot open file");
			return;
		}
		CLuaFile *pLua = CLuaAction::DoLoad(pStream, saFile);
		delete[] saFile;
		delete pStream;
		if(pLua)
		{
			frmRGDEditor* pForm;
			TheConstruct->GetTabs()->AddPage(pForm = new frmRGDEditor(oParent, sFile, TheConstruct->GetTabs(), -1), wxString().Append(wxT("SUA")).Append(wxT(" [")).Append(OnlyFilename(sFile)).Append(wxT("]")), true);
			if(!pForm->FillFromMetaTable(pLua))
			{
				ErrorBox("Cannot load file");
				delete pLua;
				return;
			}
		}
	}
};

class CLuaBurnAction : public frmFiles::IHandler
{
public:
	static bool ConvertLuaFilenameToRgd(char* sFilename)
	{
		char* sTmp = strrchr(sFilename, '.');
		strcpy(sTmp, ".rgd");

		if(strnicmp(sFilename, "generic", 7) == 0)
		{
			sFilename[0] = 'd';
			sFilename[1] = 'a';
			sFilename[2] = 't';
			sFilename[3] = 'a';
			memmove(sFilename + 4, sFilename + 7, strlen(sFilename + 6));
			return true;
		}
		return false;
	}

	virtual wxString VGetExt() const
	{
		return wxT("lua");
	}

	virtual wxString VGetAction() const
	{
		return wxT("Convert LUA to RGD");
	}

	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile);
		long iRGDVersion = 1;
		if(TheConstruct->GetModule()->GetModuleType() != CModuleFile::MT_DawnOfWar) iRGDVersion = 3;

		char* saFile = wxStringToAscii(sFile);
		IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
		if(!pStream)
		{
			delete[] saFile;
			ErrorBox("Cannot open file");
			return;
		}

		CLuaFile *pLua = CLuaAction::DoLoad(pStream, saFile);
		delete pStream;
		if(pLua)
		{
			CRgdFile *pRgd = new CRgdFile;
			if(pRgd)
			{
				pRgd->SetHashTable(TheConstruct->GetRgdHashTable());
				try
				{
					pRgd->Load(pLua, iRGDVersion);
				}
				catch(CRainmanException *pE)
				{
					ErrorBoxE(pE);
					goto after_rgd_loaded_cleany_code;
				}
					{
						bool bMovedTOC = ConvertLuaFilenameToRgd(saFile);
						BackupFile(TheConstruct->GetModule(), AsciiTowxString(saFile));
						IFileStore::IOutputStream* pOutStream = TheConstruct->GetModule()->VOpenOutputStream(saFile, true);
						if(pOutStream)
						{
							char* saDir = strdup(saFile), *pSlash;
							pSlash = strrchr(saDir, '\\');
							if(pSlash)
								*pSlash = 0;
							else
								*saDir = 0;
							IDirectoryTraverser::IIterator *pDir = TheConstruct->GetModule()->VIterate(saDir);
							free(saDir);
							frmFiles *pFiles = TheConstruct->GetFilesList();
							if(bMovedTOC)
								pFiles->UpdateDirectoryChildren(pFiles->FindFile(AsciiTowxString(saFile), true), pDir);
							else
								pFiles->UpdateDirectoryChildren(oParent, pDir);
							delete pDir;
							try
							{
								pRgd->Save(pOutStream);
								wxMessageBox(AppStr(rgd_burngood),VGetAction(),wxICON_INFORMATION,TheConstruct);
							}
							catch(CRainmanException *pE)
							{
								ErrorBoxE(pE);
								RestoreBackupFile(TheConstruct->GetModule(), AsciiTowxString(saFile));
							}
							delete pOutStream;
						}
						else
						{
							ErrorBoxAS(err_write);
						}
					}
				after_rgd_loaded_cleany_code:
				delete pRgd;
			}
			else
			{
				ErrorBoxAS(err_memory);
			}
		}
		delete[] saFile;
		delete pLua;
	}
};

class CLuaBurnFolderAction : public frmFiles::IHandler
{
protected:
	wxProgressDialog* m_pProgress;
	CLuaFileCache* m_pCache;
	long m_iRGDVersion;
	bool m_bCheckSkip;
public:
	CLuaBurnFolderAction()
		: m_pProgress(0)
	{
		m_bCheckSkip = true;
		m_iRGDVersion = 1;
		if(TheConstruct->GetModule()->GetModuleType() != CModuleFile::MT_DawnOfWar) m_iRGDVersion = 3;
	}
	virtual wxString VGetExt() const
	{
		return wxT("");
	}

	virtual wxString VGetAction() const
	{
		return wxT("Convert all LUAs (But not those in ReqiuredMods) in this folder to RGD");
	}

	virtual size_t Recurse(wxString sFolder, wxTreeItemId& oParent, bool bCountOnly, size_t iCountBase, size_t iCountDiv)
	{
		size_t iCount = 0;
		wxTreeItemIdValue oCookie;
		wxTreeCtrl *pTree = TheConstruct->GetFilesList()->GetTree();
		wxTreeItemId oChild = pTree->GetFirstChild(oParent, oCookie);
		const char* sThisMod = TheConstruct->GetModule()->GetFileMapName();
		bool bAnyMovedToC = false;
		while(oChild.IsOk())
		{
			CFilesTreeItemData *pData = (CFilesTreeItemData*)pTree->GetItemData(oChild);
			if(pData->sMod && (strcmp(pData->sMod, sThisMod) == 0))
			{
				wxString sFile = pTree->GetItemText(oChild);
				if(sFile.AfterLast('.').IsSameAs(wxT("lua"), false))
				{
					++iCount;
					if(!bCountOnly)
					{
						if(((iCountBase + iCount) % 256) == 0) m_pCache->Clear();
						int iPVal = (int)((iCountBase + iCount) / iCountDiv);
						m_pProgress->Update(iPVal, sFile);

						sFile = sFolder + wxT("\\") + pTree->GetItemText(oChild);
						char* saFile = wxStringToAscii(sFile);
						char* saRgd = strdup(saFile);
						bAnyMovedToC = CLuaBurnAction::ConvertLuaFilenameToRgd(saRgd) || bAnyMovedToC;
						
						if(m_bCheckSkip)
						{
							tLastWriteTime oLuaWriteTime = TheConstruct->GetModule()->VGetLastWriteTime(saFile);
							if(IsValidWriteTime(oLuaWriteTime))
							{
									try
								{
									tLastWriteTime oRgdWriteTime = TheConstruct->GetModule()->VGetLastWriteTime(saRgd);
									if(IsValidWriteTime(oRgdWriteTime))
									{
										if(IsWriteTimeNewer(oRgdWriteTime, oLuaWriteTime))
										{
											delete[] saFile;
											free(saRgd);
											goto rgd_is_newer;
										}
									}
								}
								catch(CRainmanException *pE)
								{
									pE->destroy();
								}
							}
						}
						

						IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
						wxString sError = wxT("");
						if(!pStream)
						{
							delete[] saFile;
							free(saRgd);
							sError = wxT("Cannot open file");
						}
						else
						{
							CLuaFile *pLua = CLuaAction::DoLoad(pStream, saFile, m_pCache);
							delete pStream;
							if(pLua)
							{
								CRgdFile *pRgd = new CRgdFile;
								if(pRgd)
								{
									pRgd->SetHashTable(TheConstruct->GetRgdHashTable());
									try
									{
										pRgd->Load(pLua, m_iRGDVersion);
									}
									catch(CRainmanException *pE)
									{
										ErrorBoxE(pE);
										goto skip_recurse_lua_load_ok_code;
									}
									BackupFile(TheConstruct->GetModule(), AsciiTowxString(saRgd));
									IFileStore::IOutputStream* pOutStream = 0;
									try
									{
										pOutStream = TheConstruct->GetModule()->VOpenOutputStream(saRgd, true);
									}
									catch(CRainmanException *pE)
									{
										ErrorBoxE(pE);
										pOutStream = 0;
									}
									if(pOutStream)
									{					
										try
										{
											pRgd->Save(pOutStream);
										}
										catch(CRainmanException *pE)
										{
											ErrorBoxE(pE);
											RestoreBackupFile(TheConstruct->GetModule(), AsciiTowxString(saRgd));
										}
										delete pOutStream;
									}
									else
									{
										sError = AppStr(err_write);
									}
skip_recurse_lua_load_ok_code:
									delete pRgd;
								}
								else
								{
									sError = AppStr(err_memory);
								}
							}
							delete[] saFile;
							free(saRgd);
							if(pLua)
								m_pCache->FreeState(pLua->m_pLua);
							delete pLua;
						}
						if(sError != wxT(""))
						{
							sError.Append(wxT("\n"));
							sError.Append(sFile);
							ErrorBoxS(sError);
						}
					}
				}
			}
			else
			{
				wxString sNewPath = sFolder;
				sNewPath.Append('\\');
				sNewPath.Append(pTree->GetItemText(oChild));
				iCount += Recurse(sNewPath, oChild, bCountOnly, iCountBase + iCount, iCountDiv);
			}
rgd_is_newer:
			oChild = pTree->GetNextChild(oChild, oCookie);
		}
		if(!bCountOnly)
		{
			if(bAnyMovedToC)
			{
				sFolder.Replace(sFolder.Left(7), wxT("Data"), false);
				oParent = TheConstruct->GetFilesList()->FindFile(sFolder, false);
			}
			if(pTree->IsExpanded(oParent))
			{
				char* saPath = wxStringToAscii(sFolder);
				IDirectoryTraverser::IIterator *pDir = TheConstruct->GetModule()->VIterate(saPath);
				delete[] saPath;
				TheConstruct->GetFilesList()->UpdateDirectoryChildren(oParent, pDir);
				delete pDir;
			}
		}
		return iCount;
	}

	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile);
		m_bCheckSkip = (wxMessageBox(wxT("Skip burning a LUA if it\'s RGD is newer?"), VGetAction(), wxYES_NO, TheConstruct) == wxYES);

		size_t iCount = Recurse(sFile, oParent, true, 0, 1);
		size_t iDiv = 1;
		while((iCount / iDiv) > (size_t)0x200)
		{
			++iDiv;
		}
		m_pProgress = new wxProgressDialog(VGetAction(), wxT(""), (int)(iCount / iDiv) + 1, TheConstruct, wxPD_SMOOTH | wxPD_AUTO_HIDE | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME);
		m_pCache = new CLuaFileCache;
		Recurse(sFile, oParent, false, 0, iDiv);
		delete m_pCache;
		wxMessageBox(AppStr(rgd_massburngood),VGetAction(),wxICON_INFORMATION,TheConstruct);
		delete m_pProgress;
	}
};

class CLuaBurnFolderIncReqAction : public frmFiles::IHandler
{
protected:
	wxProgressDialog* m_pProgress;
	CLuaFileCache* m_pCache;
	long m_iRGDVersion;
	bool m_bCheckSkip;
public:
	CLuaBurnFolderIncReqAction()
		: m_pProgress(0)
	{
		m_iRGDVersion = 1;
		m_bCheckSkip = true;
		if(TheConstruct->GetModule()->GetModuleType() != CModuleFile::MT_DawnOfWar) m_iRGDVersion = 3;
	}
	virtual wxString VGetExt() const
	{
		return wxT("");
	}

	virtual wxString VGetAction() const
	{
		return wxT("Convert all LUAs (Including those in ReqiuredMods) in this folder to RGD");
	}

	virtual size_t Recurse(wxString sFolder, wxTreeItemId& oParent, bool bCountOnly, size_t iCountBase, size_t iCountDiv)
	{
		bool bAnyMovedToC = false;
		size_t iCount = 0;
		wxTreeItemIdValue oCookie;
		wxTreeCtrl *pTree = TheConstruct->GetFilesList()->GetTree();
		wxTreeItemId oChild = pTree->GetFirstChild(oParent, oCookie);
		while(oChild.IsOk())
		{
			CFilesTreeItemData *pData = (CFilesTreeItemData*)pTree->GetItemData(oChild);
			if(pData->sMod)
			{
				wxString sFile = pTree->GetItemText(oChild);
				if(sFile.AfterLast('.').IsSameAs(wxT("lua"), false))
				{
					++iCount;
					if(!bCountOnly)
					{
						if(((iCountBase + iCount) % 256) == 0) m_pCache->Clear();
						int iPVal = (int)((iCountBase + iCount) / iCountDiv);
						m_pProgress->Update(iPVal, sFile);

						sFile = sFolder + wxT("\\") + pTree->GetItemText(oChild);
						char* saFile = wxStringToAscii(sFile);
						char* saRgd = strdup(saFile);
						bAnyMovedToC = CLuaBurnAction::ConvertLuaFilenameToRgd(saRgd) || bAnyMovedToC;
						
						if(m_bCheckSkip)
						{
							tLastWriteTime oLuaWriteTime = TheConstruct->GetModule()->VGetLastWriteTime(saFile);
							if(IsValidWriteTime(oLuaWriteTime))
							{
								try
								{
									tLastWriteTime oRgdWriteTime = TheConstruct->GetModule()->VGetLastWriteTime(saRgd);
									if(IsValidWriteTime(oRgdWriteTime))
									{
										if(IsWriteTimeNewer(oRgdWriteTime, oLuaWriteTime))
										{
											delete[] saFile;
											free(saRgd);
											goto rgd_is_newer;
										}
									}
								}
								catch(CRainmanException *pE)
								{
									pE->destroy();
								}
							}
						}

						IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
						wxString sError = wxT("");
						if(!pStream)
						{
							delete[] saFile;
							free(saRgd);
							sError = wxT("Cannot open file");
						}
						else
						{
							CLuaFile *pLua = CLuaAction::DoLoad(pStream, saFile, m_pCache);
							delete pStream;
							if(pLua)
							{
								CRgdFile *pRgd = new CRgdFile;
								if(pRgd)
								{
									pRgd->SetHashTable(TheConstruct->GetRgdHashTable());
									try
									{
										pRgd->Load(pLua, m_iRGDVersion);
									}
									catch(CRainmanException *pE)
									{
										ErrorBoxE(pE);
										goto skip_recurse_lua_load_ok_code;
									}
									BackupFile(TheConstruct->GetModule(), AsciiTowxString(saRgd));
									IFileStore::IOutputStream* pOutStream = 0;
									try
									{
										pOutStream = TheConstruct->GetModule()->VOpenOutputStream(saRgd, true);
									}
									catch(CRainmanException *pE)
									{
										ErrorBoxE(pE);
										pOutStream = 0;
									}
									if(pOutStream)
									{					
										try
										{
											pRgd->Save(pOutStream);
										}
										catch(CRainmanException *pE)
										{
											ErrorBoxE(pE);
											RestoreBackupFile(TheConstruct->GetModule(), AsciiTowxString(saRgd));
										}
										delete pOutStream;
									}
									else
									{
										sError = AppStr(err_write);
									}
skip_recurse_lua_load_ok_code:
									delete pRgd;
								}
								else
								{
									sError = AppStr(err_memory);
								}
							}
							delete[] saFile;
							free(saRgd);
							if(pLua)
								m_pCache->FreeState(pLua->m_pLua);
							delete pLua;
						}
						if(sError != wxT(""))
						{
							sError.Append(wxT("\n"));
							sError.Append(sFile);
							ErrorBoxS(sError);
						}
					}
				}
			}
			else
			{
				wxString sNewPath = sFolder;
				sNewPath.Append('\\');
				sNewPath.Append(pTree->GetItemText(oChild));
				iCount += Recurse(sNewPath, oChild, bCountOnly, iCountBase + iCount, iCountDiv);
			}
rgd_is_newer:
			oChild = pTree->GetNextChild(oChild, oCookie);
		}
		if(!bCountOnly)
		{
			if(bAnyMovedToC)
			{
				sFolder.Replace(sFolder.Left(7), wxT("Data"), false);
				oParent = TheConstruct->GetFilesList()->FindFile(sFolder, false);
			}
			if(pTree->IsExpanded(oParent))
			{
				char* saPath = wxStringToAscii(sFolder);
				IDirectoryTraverser::IIterator *pDir = TheConstruct->GetModule()->VIterate(saPath);
				delete[] saPath;
				TheConstruct->GetFilesList()->UpdateDirectoryChildren(oParent, pDir);
				delete pDir;
			}
		}
		return iCount;
	}

	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile);
		m_bCheckSkip = (wxMessageBox(wxT("Skip burning a LUA if it\'s RGD is newer?"), VGetAction(), wxYES_NO, TheConstruct) == wxYES);

		size_t iCount = Recurse(sFile, oParent, true, 0, 1);
		size_t iDiv = 1;
		while((iCount / iDiv) > (size_t)0x200)
		{
			++iDiv;
		}
		m_pProgress = new wxProgressDialog(VGetAction(), wxT(""), (int)(iCount / iDiv) + 1, TheConstruct, wxPD_SMOOTH | wxPD_AUTO_HIDE | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME);
		m_pCache = new CLuaFileCache;
		Recurse(sFile, oParent, false, 0, iDiv);
		delete m_pCache;
		wxMessageBox(AppStr(rgd_massburngood),VGetAction(),wxICON_INFORMATION,TheConstruct);
		delete m_pProgress;
	}
};

class CLuaDumpFolder : public frmFiles::IHandler
{
protected:
	wxProgressDialog* m_pProgress;
public:
	CLuaDumpFolder()
		: m_pProgress(0)
	{
	}
	virtual wxString VGetExt() const
	{
		return wxT("");
	}

	virtual wxString VGetAction() const
	{
		return wxT("Dump all RGDs in this folder to Lua");
	}

	virtual size_t Recurse(wxString sFolder, wxTreeItemId& oParent, bool bCountOnly, size_t iCountBase, size_t iCountDiv)
	{
		size_t iCount = 0;
		wxTreeItemIdValue oCookie;
		wxTreeCtrl *pTree = TheConstruct->GetFilesList()->GetTree();
		wxTreeItemId oChild = pTree->GetFirstChild(oParent, oCookie);
		//const char* sThisMod = TheConstruct->GetModule()->GetFileMapName();
		while(oChild.IsOk())
		{
			CFilesTreeItemData *pData = (CFilesTreeItemData*)pTree->GetItemData(oChild);
			if(pData->sMod)
			{
				wxString sFile = pTree->GetItemText(oChild);
				if(sFile.AfterLast('.').IsSameAs(wxT("rgd"), false))
				{
					++iCount;
					if(!bCountOnly)
					{
						int iPVal = (int)((iCountBase + iCount) / iCountDiv);
						m_pProgress->Update(iPVal, sFile);

						sFile = sFolder + wxT("\\") + pTree->GetItemText(oChild);
						char* saFile = wxStringToAscii(sFile);
						try
						{
							CRgdToLuaDumpAction::DoConvert(saFile);
						}
						catch(CRainmanException *pE)
						{
							ErrorBoxE(pE);
						}
						delete[] saFile;
					}
				}
			}
			else
			{
				wxString sNewPath = sFolder;
				sNewPath.Append('\\');
				sNewPath.Append(pTree->GetItemText(oChild));
				iCount += Recurse(sNewPath, oChild, bCountOnly, iCountBase + iCount, iCountDiv);
			}
			oChild = pTree->GetNextChild(oChild, oCookie);
		}
		if(!bCountOnly)
		{
			if(pTree->IsExpanded(oParent))
			{
				char* saPath = wxStringToAscii(sFolder);
				IDirectoryTraverser::IIterator *pDir = TheConstruct->GetModule()->VIterate(saPath);
				delete[] saPath;
				TheConstruct->GetFilesList()->UpdateDirectoryChildren(oParent, pDir);
				delete pDir;
			}
		}
		return iCount;
	}

	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile);
		size_t iCount = Recurse(sFile, oParent, true, 0, 1);
		size_t iDiv = 1;
		while((iCount / iDiv) > (size_t)0x200)
		{
			++iDiv;
		}
		m_pProgress = new wxProgressDialog(VGetAction(), wxT(""), (int)(iCount / iDiv) + 1, TheConstruct, wxPD_SMOOTH | wxPD_AUTO_HIDE | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME);
		Recurse(sFile, oParent, false, 0, iDiv);
		wxMessageBox(wxT("Batch dump complete"),VGetAction(),wxICON_INFORMATION,TheConstruct);
		delete m_pProgress;
	}
};

class CScanHashesAction : public frmFiles::IHandler
{
protected:
	wxProgressDialog* m_pProgress;
public:
	CScanHashesAction()
		: m_pProgress(0)
	{
	}
	virtual wxString VGetExt() const
	{
		return wxT("");
	}

	virtual wxString VGetAction() const
	{
		return wxT("Scan RGDs in this folder for unknown hashes");
	}

	virtual size_t Recurse(wxString sFolder, wxTreeItemId& oParent, bool bCountOnly, size_t iCountBase, size_t iCountDiv)
	{
		size_t iCount = 0;
		wxTreeItemIdValue oCookie;
		wxTreeCtrl *pTree = TheConstruct->GetFilesList()->GetTree();
		wxTreeItemId oChild = pTree->GetFirstChild(oParent, oCookie);
		//const char* sThisMod = TheConstruct->GetModule()->GetFileMapName();
		while(oChild.IsOk())
		{
			CFilesTreeItemData *pData = (CFilesTreeItemData*)pTree->GetItemData(oChild);
			if(pData->sMod)
			{
				wxString sFile = pTree->GetItemText(oChild);
				if(sFile.AfterLast('.').IsSameAs(wxT("rgd"), false) || sFile.AfterLast('.').IsSameAs(wxT("bfx"), false))
				{
					++iCount;
					if(!bCountOnly)
					{
						int iPVal = (int)((iCountBase + iCount) / iCountDiv);
						m_pProgress->Update(iPVal, sFile);

						sFile = sFolder + wxT("\\") + pTree->GetItemText(oChild);
						char* saFile = wxStringToAscii(sFile);
						
						IFileStore::IStream *pStream = TheConstruct->GetModule()->VOpenStream(saFile);
						delete[] saFile;
						if(pStream)
						{
							CRgdFile oRGD;
							oRGD.SetHashTable(TheConstruct->GetRgdHashTable());
							oRGD.Load(pStream);
							delete pStream;
						}
					}
				}
			}
			else
			{
				wxString sNewPath = sFolder;
				sNewPath.Append('\\');
				sNewPath.Append(pTree->GetItemText(oChild));
				iCount += Recurse(sNewPath, oChild, bCountOnly, iCountBase + iCount, iCountDiv);
			}
			oChild = pTree->GetNextChild(oChild, oCookie);
		}
		if(!bCountOnly)
		{
			if(pTree->IsExpanded(oParent))
			{
				char* saPath = wxStringToAscii(sFolder);
				IDirectoryTraverser::IIterator *pDir = TheConstruct->GetModule()->VIterate(saPath);
				delete[] saPath;
				TheConstruct->GetFilesList()->UpdateDirectoryChildren(oParent, pDir);
				delete pDir;
			}
		}
		return iCount;
	}

	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile);
		size_t iCount = Recurse(sFile, oParent, true, 0, 1);
		size_t iDiv = 1;
		while((iCount / iDiv) > (size_t)0x200)
		{
			++iDiv;
		}
		m_pProgress = new wxProgressDialog(VGetAction(), wxT(""), (int)(iCount / iDiv) + 1, TheConstruct, wxPD_SMOOTH | wxPD_AUTO_HIDE);
		Recurse(sFile, oParent, false, 0, iDiv);
		delete m_pProgress;

		wxFileDialog* pFileDialog = 0;
		pFileDialog = new wxFileDialog(TheConstruct, AppStr(mod_select_dll),
			ConfGetDoWFolder(), wxT(""), AppStr(mod_dll_filter), wxOPEN);
		if(!pFileDialog)
		{
			return;
		}
		if(pFileDialog->ShowModal() == wxID_OK)
		{
			std::vector<unsigned long> oHashList;
			TheConstruct->GetRgdHashTable()->FillUnknownList(oHashList);

			unsigned char *sStr = new unsigned char[4096], *sStr2 = new unsigned char[4096];
			unsigned long iHash;
			size_t iL = 0;
			unsigned char iByte;
			char* sFile = UnicodeToAscii(pFileDialog->GetPath());
			FILE* fIn = fopen(sFile, "rb");
			fseek(fIn, 0, SEEK_END);
			iCount = ftell(fIn);
			fseek(fIn, 0, SEEK_SET);
			size_t iDiv = 1;
			while((iCount / iDiv) > (size_t)0x200)
			{
				++iDiv;
			}
			m_pProgress = new wxProgressDialog(VGetAction(), wxT("Scanning file"), (int)(iCount / iDiv) + 2, TheConstruct, wxPD_SMOOTH | wxPD_AUTO_HIDE);
			iCount = 0;
			delete[] sFile;
			while(!feof(fIn))
			{
				if((++iCount % iDiv) == 0) m_pProgress->Update( (int)(iCount / iDiv), wxT("Scanning file"));
				fread(&iByte,1,1,fIn);
				if(iByte > 31 && iByte < 128)
				{
					sStr[iL] = iByte;
					++iL;
				}
				else
				{
					if(iL)
					{
						sStr[iL] = 0;
						for(size_t i = 0; i < iL; ++i)
						{
							for(size_t j = i; j < iL; ++j)
							{
								iHash = hash((ub1*)(sStr + i), (ub4)(j-i+1), (ub4)0);
								for(std::vector<unsigned long>::iterator itr = oHashList.begin(); itr != oHashList.end(); ++itr)
								{
									if(*itr == iHash)
									{
										sStr[j + 1] = 0;
										TheConstruct->GetRgdHashTable()->ValueToHash((const char*)sStr + i);
										break;
									}
								}
							}
						}
						iL = 0;
					}
				}
			}
			fclose(fIn);
			wxMessageBox(wxT("Scan complete"),VGetAction(),wxICON_INFORMATION,TheConstruct);
			delete[] sStr;
			delete[] sStr2;
			delete m_pProgress;
		}
	}
};

class CNilAction : public CLuaAction
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("nil");
	}

	virtual wxString VGetAction() const
	{
		return wxT("View as NIL file");
	}
};

class CScarAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("scar");
	}

	virtual wxString VGetAction() const
	{
		return wxT("View as SCAR file");
	}
	
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile);
		frmScarEditor* pForm;
		TheConstruct->GetTabs()->AddPage(pForm = new frmScarEditor(oParent, sFile, TheConstruct->GetTabs(), -1), wxString().Append(wxT("SCAR")).Append(wxT(" [")).Append(OnlyFilename(sFile)).Append(wxT("]")), true);
		
		char* saFile = wxStringToAscii(sFile);
		IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
		if(!pStream)
		{
			delete[] saFile;
			ErrorBox("Cannot open file");
			return;
		}
		delete[] saFile;

		pForm->Load(pStream);

		delete pStream;
	}
};

class CAiAction : public frmFiles::IHandler
{
public:
	virtual wxString VGetExt() const
	{
		return wxT("ai");
	}

	virtual wxString VGetAction() const
	{
		return wxT("View as AI file");
	}
	
	virtual void VHandle(wxString sFile, wxTreeItemId& oParent, wxTreeItemId& oFile)
	{ UNUSED(oFile);
		frmScarEditor* pForm;
		TheConstruct->GetTabs()->AddPage(pForm = new frmScarEditor(oParent, sFile, TheConstruct->GetTabs(), -1, wxDefaultPosition, wxDefaultSize, 0), wxString().Append(wxT("AI")).Append(wxT(" [")).Append(OnlyFilename(sFile)).Append(wxT("]")), true);
		
		char* saFile = wxStringToAscii(sFile);
		IFileStore::IStream* pStream = TheConstruct->GetModule()->VOpenStream(saFile);
		if(!pStream)
		{
			delete[] saFile;
			ErrorBox("Cannot open file");
			return;
		}
		delete[] saFile;

		pForm->Load(pStream);

		delete pStream;
	}
};