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

#include "ModuleFileWriter.h"
#include <stdio.h>
#include <string.h>
#include "PathUtil.h"

namespace
{
	struct _tManagedKey
	{
		const char* sKey;
		const char* sValue;
		bool bFound;
	};

	bool _ReadWholeFile(const char* sFile, std::string& sContent, std::string& sError)
	{
		FILE* fIn = PathUtil_fopen(sFile, "rb");
		if(fIn == 0)
		{
			sError = "Unable to open the .module file for reading.";
			return false;
		}
		char aBuffer[4096];
		size_t iRead;
		while((iRead = fread(aBuffer, 1, sizeof(aBuffer), fIn)) > 0)
		{
			sContent.append(aBuffer, iRead);
		}
		fclose(fIn);
		return true;
	}

	std::string _FormatVersion(long iMajor, long iMinor, long iRevision)
	{
		char aVersion[64];
		sprintf(aVersion, "%li.%li.%li", iMajor, iMinor, iRevision);
		return std::string(aVersion);
	}
}

bool Rainman_UpdateModuleFile(const char* sFile,
	const char* sUIName, const char* sDescription,
	const char* sTextureFE, const char* sTextureIcon,
	long iVersionMajor, long iVersionMinor, long iVersionRevision,
	std::string& sError)
{
	if(sFile == 0 || sFile[0] == 0)
	{
		sError = "No .module file was specified.";
		return false;
	}

	std::string sInput;
	if(!_ReadWholeFile(sFile, sInput, sError))
	{
		return false;
	}

	const std::string sVersion = _FormatVersion(iVersionMajor, iVersionMinor, iVersionRevision);

	// Values live as UTF-8 inside the program but .module files are ANSI, so
	// convert at the file boundary (unrepresentable characters become '?').
	std::string aAnsiValues[4];
	{
		const char* apSources[4] = { sUIName, sDescription, sTextureFE, sTextureIcon };
		for(int i = 0; i < 4; ++i)
		{
			char* sAnsi = PathUtil_Utf8ToAnsi(apSources[i] ? apSources[i] : "");
			aAnsiValues[i] = sAnsi;
			delete[] sAnsi;
		}
	}

	_tManagedKey aKeys[] =
	{
		{ "UIName",      aAnsiValues[0].c_str(), false },
		{ "Description", aAnsiValues[1].c_str(), false },
		{ "TextureFE",   aAnsiValues[2].c_str(), false },
		{ "TextureIcon", aAnsiValues[3].c_str(), false },
		{ "ModVersion",  sVersion.c_str(),       false },
	};
	const size_t iKeyCount = sizeof(aKeys) / sizeof(aKeys[0]);

	std::string sOutput;
	sOutput.reserve(sInput.size() + 128);
	size_t iGlobalInsert = std::string::npos;
	const char* sGlobalEol = "\r\n";

	const char* pLine = sInput.c_str();
	const char* pBufferEnd = pLine + sInput.size();
	while(pLine < pBufferEnd)
	{
		const char* pNewline = (const char*)memchr(pLine, '\n', (size_t)(pBufferEnd - pLine));
		size_t iLineLength = pNewline ? (size_t)(pNewline - pLine) : (size_t)(pBufferEnd - pLine);
		size_t iContent = iLineLength;
		bool bCrLf = false;
		if(iContent > 0 && pLine[iContent - 1] == '\r')
		{
			--iContent;
			bCrLf = true;
		}

		bool bUpdated = false;
		const char* pEquals = (const char*)memchr(pLine, '=', iContent);
		if(pEquals)
		{
			size_t iKeyBegin = 0;
			while(iKeyBegin < (size_t)(pEquals - pLine) && (pLine[iKeyBegin] == ' ' || pLine[iKeyBegin] == '\t'))
			{
				++iKeyBegin;
			}
			size_t iKeyEnd = (size_t)(pEquals - pLine);
			while(iKeyEnd > iKeyBegin && (pLine[iKeyEnd - 1] == ' ' || pLine[iKeyEnd - 1] == '\t'))
			{
				--iKeyEnd;
			}
			size_t iKeyLength = iKeyEnd - iKeyBegin;
			for(size_t i = 0; i < iKeyCount; ++i)
			{
				if(strlen(aKeys[i].sKey) == iKeyLength && _strnicmp(pLine + iKeyBegin, aKeys[i].sKey, iKeyLength) == 0)
				{
					sOutput.append(pLine, iKeyBegin);
					sOutput.append(aKeys[i].sKey);
					sOutput.append(" = ");
					sOutput.append(aKeys[i].sValue);
					aKeys[i].bFound = true;
					bUpdated = true;
					break;
				}
			}
		}

		if(!bUpdated)
		{
			sOutput.append(pLine, iContent);
		}
		sOutput.append(bCrLf ? "\r\n" : "\n");

		if(iGlobalInsert == std::string::npos && iContent == 8 && _strnicmp(pLine, "[global]", 8) == 0)
		{
			iGlobalInsert = sOutput.size();
			sGlobalEol = bCrLf ? "\r\n" : "\n";
		}

		if(pNewline == 0)
		{
			break;
		}
		pLine = pNewline + 1;
	}

	// Add the keys the file did not have yet; they are optional in 0.5.5 modules.
	if(iGlobalInsert != std::string::npos)
	{
		std::string sMissing;
		for(size_t i = 0; i < iKeyCount; ++i)
		{
			if(!aKeys[i].bFound && aKeys[i].sValue && aKeys[i].sValue[0])
			{
				sMissing.append(aKeys[i].sKey);
				sMissing.append(" = ");
				sMissing.append(aKeys[i].sValue);
				sMissing.append(sGlobalEol);
			}
		}
		if(!sMissing.empty())
		{
			sOutput.insert(iGlobalInsert, sMissing);
		}
	}

	const std::string sTempName = std::string(sFile) + ".modstudio.tmp";
	FILE* fOut = PathUtil_fopen(sTempName.c_str(), "wb");
	if(fOut == 0)
	{
		sError = "Unable to open a temporary file next to the .module for writing.";
		return false;
	}
	size_t iWritten = fwrite(sOutput.data(), 1, sOutput.size(), fOut);
	fclose(fOut);
	if(iWritten != sOutput.size())
	{
		PathUtil_remove(sTempName.c_str());
		sError = "The temporary .module file could not be written completely.";
		return false;
	}
	if(!PathUtil_moveReplace(sTempName.c_str(), sFile))
	{
		PathUtil_remove(sTempName.c_str());
		sError = "The .module file could not be replaced.";
		return false;
	}
	return true;
}
