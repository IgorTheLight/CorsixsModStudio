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

#include "frmImage.h"
#include "Construct.h"
#include "strings.h"
#include "config.h"
#include "Utility.h"
#include <wx/msgdlg.h>
#include <wx/mstream.h>
#include "Common.h"

BEGIN_EVENT_TABLE(frmImageViewer, wxWindow)
	EVT_SIZE(frmImageViewer::OnSize)
	EVT_RADIOBOX(IDC_SaveFileExt, frmImageViewer::OnRadioButtonSaveExt)
	EVT_BUTTON(wxID_SAVE, frmImageViewer::OnSave)
END_EVENT_TABLE()

class wxMyScrolledWindow : public wxScrolledWindow
{
public:
	wxMyScrolledWindow(wxWindow* parent, wxBitmap* pBitmap, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxHSCROLL | wxVSCROLL, const wxString& name = wxT("scrolledWindow"))
		: wxScrolledWindow(parent, id, pos, size, style, name) , m_pBitmap(pBitmap)
	{
		m_pDC = new wxMemoryDC(*m_pBitmap);

		SetVirtualSize(pBitmap->GetWidth(), pBitmap->GetHeight());
		SetScrollRate(8, 8);
	}

	~wxMyScrolledWindow()
	{
		delete m_pDC;
	}

	virtual void OnDraw(wxDC& dc)
	{
		dc.Blit(0, 0, m_pBitmap->GetWidth(), m_pBitmap->GetHeight(), m_pDC, 0, 0);
	}

protected:
	wxBitmap* m_pBitmap;
	wxMemoryDC* m_pDC;
};

/*
BEGIN_EVENT_TABLE(wxMyScrolledWindow, wxScrolledWindow)
	EVT_PAINT(wxMyScrolledWindow::OnPaint)
END_EVENT_TABLE()
*/

frmImageViewer::frmImageViewer(wxTreeItemId& oFileParent, wxString sFilename, wxWindow* parent, wxWindowID id, CRgtFile* pImage, bool bOwnImage, const wxPoint& pos, const wxSize& size)
	: m_oFileParent(oFileParent), m_sFilename(sFilename), wxWindow(parent, id, pos, size)
{
	wxBoxSizer *pTopSizer = new wxBoxSizer( wxVERTICAL );

	m_pRgtFile = pImage;
	m_pImageBitmap = 0;
	m_bOwnRgt = bOwnImage;

	{
		CMemoryStore::COutStream* pTgaSpace = CMemoryStore::OpenOutputStreamExt();
		pImage->SaveTGA(pTgaSpace);
		wxMemoryInputStream oTgaSpace(pTgaSpace->GetData(), pTgaSpace->GetDataLength());
		wxImage oTga(oTgaSpace, wxBITMAP_TYPE_TGA);
		m_pImageBitmap = new wxBitmap(oTga);
		delete pTgaSpace;
	}

	wxStaticBoxSizer *pImgSizer = new wxStaticBoxSizer( wxVERTICAL, this, wxT("Current Image") );

	wxArrayString aFileTypes, aCompressionTypes;

	aFileTypes.Add(wxT("RGT"));
	aFileTypes.Add(wxT("DDS"));
	aFileTypes.Add(wxT("TGA"));

	aCompressionTypes.Add(wxT("RGB (no alpha)"));
	aCompressionTypes.Add(wxT("RGBA (8 bit alpha)"));
	aCompressionTypes.Add(wxT("DXT1 (1 bit alpha)"));
	aCompressionTypes.Add(wxT("DXT3 (4 bit alpha)"));
	aCompressionTypes.Add(wxT("DXT5 (4 bit alpha)"));

	wxRadioBox* pRadioBox;
	pImgSizer->Add(m_pCurrentExt = pRadioBox = new wxRadioBox(this, -1, wxT("File"), wxDefaultPosition, wxDefaultSize, aFileTypes), 0, wxEXPAND | wxALL, 0);

	pRadioBox->SetSelection(0); // RGT
	pRadioBox->Enable(false);

	pImgSizer->Add(m_pCurrentCompression = pRadioBox = new wxRadioBox(this, -1, wxT("Compression"), wxDefaultPosition, wxDefaultSize, aCompressionTypes), 0, wxEXPAND | wxALL, 0);

	switch(pImage->GetImageFormat())
	{
	case CRgtFile::IF_Tga:
		pRadioBox->SetSelection(1); // RGBA
		break;
	case CRgtFile::IF_Dxtc:
		switch(pImage->GetProperty(CRgtFile::IP_CompressionLevel))
		{
		case 1:
			pRadioBox->SetSelection(2);
			break;
		case 3:
			pRadioBox->SetSelection(3);
			break;
		case 5:
			pRadioBox->SetSelection(4);
			break;
		}
		break;
	}
	pRadioBox->Enable(false);

	wxCheckBox* pCheckBox;
	pImgSizer->Add(pCheckBox = new wxCheckBox(this, -1, wxT("Mip Levels")), 0, wxEXPAND | wxALL, 3);

	pCheckBox->SetValue(pImage->GetProperty(CRgtFile::IP_MipLevelCount) > 1);
	pCheckBox->Enable(false);

	pImgSizer->Add(new wxMyScrolledWindow(this, m_pImageBitmap), 1, wxEXPAND | wxALL, 0);

	pTopSizer->Add(pImgSizer, 1, wxEXPAND | wxALL, 3);

	wxStaticBoxSizer *pSaveSizer = new wxStaticBoxSizer( wxVERTICAL, this, wxT("Save Copy") );

	pSaveSizer->Add(m_pSaveFileExt = new wxRadioBox(this, IDC_SaveFileExt, wxT("File"), wxDefaultPosition, wxDefaultSize, aFileTypes), 0, wxEXPAND | wxALL, 0);
	pSaveSizer->Add(m_pSaveFileCompression = new wxRadioBox(this, -1, wxT("Compression"), wxDefaultPosition, wxDefaultSize, aCompressionTypes), 0, wxEXPAND | wxALL, 0);
	pSaveSizer->Add(m_pSaveFileMips = new wxCheckBox(this, -1, wxT("Mip Levels")), 0, wxEXPAND | wxALL, 3);

	pSaveSizer->Add(new wxButton(this, wxID_SAVE,wxT("Save Copy")), 0, wxALL, 3);

	switch(pImage->GetImageFormat())
	{
	case CRgtFile::IF_Tga:
		m_pSaveFileExt->SetSelection(2);
		m_pSaveFileCompression->SetSelection(1);
		m_pSaveFileCompression->Enable(2, false);
		m_pSaveFileCompression->Enable(3, false);
		m_pSaveFileCompression->Enable(4, false);
		m_pSaveFileMips->Enable(false);
		break;
	case CRgtFile::IF_Dxtc:
		m_pSaveFileExt->SetSelection(1);
		m_pSaveFileCompression->Enable(0, false);
		m_pSaveFileCompression->Enable(1, false);
		m_pSaveFileCompression->SetSelection(pRadioBox->GetSelection());
		break;
	}

	pTopSizer->Add(pSaveSizer, 0, wxEXPAND | wxALL, 3);

	SetSizer(pTopSizer);
	pTopSizer->SetSizeHints( this );
}

void frmImageViewer::SetIsTga(bool bOnlyRGB)
{
	m_pCurrentExt->SetSelection(2);
	m_pCurrentCompression->SetSelection(bOnlyRGB ? 0 : 1);
	m_pSaveFileExt->SetSelection(0);

	m_pSaveFileCompression->Enable(0, false);
	m_pSaveFileCompression->Enable(1, true);
	m_pSaveFileCompression->Enable(2, true);
	m_pSaveFileCompression->Enable(3, true);
	m_pSaveFileCompression->Enable(4, true);
	if(m_pSaveFileCompression->GetSelection() == 0) m_pSaveFileCompression->SetSelection(1);
	m_pSaveFileMips->Enable(true);
}

void frmImageViewer::SetIsDds()
{
	m_pCurrentExt->SetSelection(1);
	m_pSaveFileExt->SetSelection(0);

	m_pSaveFileCompression->Enable(0, false);
	m_pSaveFileCompression->Enable(1, false);
	m_pSaveFileCompression->Enable(2, true);
	m_pSaveFileCompression->Enable(3, true);
	m_pSaveFileCompression->Enable(4, true);
	if(m_pSaveFileCompression->GetSelection() == 0) m_pSaveFileCompression->SetSelection(2);
	if(m_pSaveFileCompression->GetSelection() == 1) m_pSaveFileCompression->SetSelection(3);
	m_pSaveFileMips->Enable(true);
}

frmImageViewer::~frmImageViewer()
{
	if(m_bOwnRgt && m_pRgtFile) delete m_pRgtFile;
}

void frmImageViewer::OnSize(wxSizeEvent& event)
{  UNUSED(event);
	Layout();
}

void frmImageViewer::OnRadioButtonSaveExt(wxCommandEvent& event)
{
	switch(event.GetSelection())
	{
	case 0: // RGT
		m_pSaveFileCompression->Enable(0, false);
		m_pSaveFileCompression->Enable(1, true);
		m_pSaveFileCompression->Enable(2, true);
		m_pSaveFileCompression->Enable(3, true);
		m_pSaveFileCompression->Enable(4, true);
		if(m_pSaveFileCompression->GetSelection() == 0) m_pSaveFileCompression->SetSelection(1);
		m_pSaveFileMips->Enable(true);
		break;
	case 1: // DDS
		m_pSaveFileCompression->Enable(0, false);
		m_pSaveFileCompression->Enable(1, false);
		m_pSaveFileCompression->Enable(2, true);
		m_pSaveFileCompression->Enable(3, true);
		m_pSaveFileCompression->Enable(4, true);
		if(m_pSaveFileCompression->GetSelection() == 0) m_pSaveFileCompression->SetSelection(2);
		if(m_pSaveFileCompression->GetSelection() == 1) m_pSaveFileCompression->SetSelection(3);
		m_pSaveFileMips->Enable(true);
		break;
	case 2: // TGA
		m_pSaveFileCompression->Enable(0, true);
		m_pSaveFileCompression->Enable(1, true);
		m_pSaveFileCompression->Enable(2, false);
		m_pSaveFileCompression->Enable(3, false);
		m_pSaveFileCompression->Enable(4, false);
		if(m_pSaveFileCompression->GetSelection() > 1) m_pSaveFileCompression->SetSelection(1);
		m_pSaveFileMips->Enable(false);
		m_pSaveFileMips->SetValue(false);
		break;
	}
}

void frmImageViewer::OnSave(wxCommandEvent& event)
{  UNUSED(event);
	if(m_pSaveFileExt->GetSelection() == m_pCurrentExt->GetSelection())
	{
		if(::wxMessageBox(wxT("The options selected will result in the original file being overwritten. Continue?"), wxT("Save"), wxOK | wxCANCEL) == wxCANCEL) return;
	}

	wxString sNewFile = m_sFilename.BeforeLast('.');
	sNewFile.Append(wxT("."));
	sNewFile.Append(m_pSaveFileExt->GetStringSelection().Lower());

	char* saNewFile = wxStringToAscii(sNewFile);
	char* saDir = strdup(saNewFile), *pSlash;
	pSlash = strrchr(saDir, '\\');
	if(pSlash)
		*pSlash = 0;
	else
		*saDir = 0;

	// START
	IFileStore::IOutputStream* pStream = 0;
	try
	{
		pStream = TheConstruct->GetModule()->VOpenOutputStream(saNewFile, true);
	}
	catch(CRainmanException *pE)
	{
		ErrorBoxE(pE);
		free(saDir);
		delete pStream;
		delete[] saNewFile;
		return;
	}

	IDirectoryTraverser::IIterator *pDir;
	try
	{
		pDir = TheConstruct->GetModule()->VIterate(saDir);
	}
	catch(CRainmanException *pE)
	{
		ErrorBoxE(pE);
		free(saDir);
		delete pStream;
		delete[] saNewFile;
		return;
	}
	free(saDir);

	try
	{
		TheConstruct->GetFilesList()->UpdateDirectoryChildren(m_oFileParent, pDir);
	}
	catch(CRainmanException *pE)
	{
		ErrorBoxE(pE);
		delete pDir;
		delete pStream;
		delete[] saNewFile;
		return;
	}
	delete pDir;
	// END

	if(!pStream)
	{
		ErrorBoxAS(err_couldnotopenoutput);
		delete[] saNewFile;
		return;
	}
	delete[] saNewFile;

	if(m_pSaveFileExt->GetSelection() == 0)
	{
		// RGT
		if(m_pSaveFileCompression->GetSelection() == 1)
		{
			// RGT -> RGBA
			CMemoryStore::COutStream *pTempOut = CMemoryStore::OpenOutputStreamExt();
			m_pRgtFile->SaveTGA(pTempOut, true);
			pTempOut->VSeek(0, IFileStore::IStream::SL_Root);
			CRgtFile oRgtOut;
			oRgtOut.LoadTGA(pTempOut, m_pSaveFileMips->GetValue());
			delete pTempOut;
			oRgtOut.Save(pStream);
		}
		else
		{
			// RGT -> DXTC
			CMemoryStore::COutStream *pTempOut = CMemoryStore::OpenOutputStreamExt();
			m_pRgtFile->SaveDDS(pTempOut, m_pSaveFileCompression->GetSelection() * 2 - 3, m_pSaveFileMips->GetValue());
			pTempOut->VSeek(0, IFileStore::IStream::SL_Root);
			CRgtFile oRgtOut;
			oRgtOut.LoadDDS(pTempOut);
			delete pTempOut;
			oRgtOut.Save(pStream);
		}
	}
	else if(m_pSaveFileExt->GetSelection() == 1)
	{
		// DDS
		m_pRgtFile->SaveDDS(pStream, m_pSaveFileCompression->GetSelection() * 2 - 3, m_pSaveFileMips->GetValue());
	}
	else if(m_pSaveFileExt->GetSelection() == 2)
	{
		// TGA
		m_pRgtFile->SaveTGA(pStream, m_pSaveFileCompression->GetSelection() == 1);
	}

	delete pStream;
}