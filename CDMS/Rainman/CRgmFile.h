/*
Rainman Library
Copyright (C) 2006 Corsix <corsix@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _C_RGM_FILE_H_
#define _C_RGM_FILE_H_

#include "gnuc_defines.h"
#include "CChunkyFile.h"

class RAINMAN_API CRgmFile
{
public:
	class RAINMAN_API CMaterial
	{
	public:
		const char* GetName() const;
		const char* GetDxName() const;
		void SetName(const char* sValue);
		void SetDxName(const char* sValue);

		class RAINMAN_API CVariable
		{
		public:
			enum eValTypes
			{
				VT_Text,
				VT_Number,
				VT_NumberArray,
			};

			const char* GetName() const;
			eValTypes GetType() const;
			const char* GetValueText() const;
			float GetValueNumber() const;

			void SetName(const char* sName);
			void SetValueText(const char* sValue);
			void SetValueNumber(float fValue);

		protected:
			friend class CRgmFile::CMaterial;
			CVariable(CChunkyFile::CChunk* pChunk);
			CVariable();
			~CVariable();
			void _Free();
			void _WriteChunk();

			CChunkyFile::CChunk* m_pOurChunk;
			eValTypes m_eValType;
			union
			{
				char* m_sValue;
				float m_fValue;
			};
			char* m_sName;
		};

		size_t GetVariableCount() const;
		CVariable* GetVariable(size_t i);
		void DeleteVariable(size_t i);
		CVariable* NewVariable();

	protected:
		friend class CRgmFile;
		char* m_sName;
		char* m_sDxName;
		std::vector<CVariable*> m_vVariables;
		CChunkyFile::CChunk* m_pOurChunk;

		CMaterial(CChunkyFile::CChunk* pChunk);
		~CMaterial();

		void _Free();
		void _ParseInfo(CChunkyFile::CChunk* pChunk);
		void _WriteChunk();
	};

	CRgmFile();
	~CRgmFile();

	void Load(IFileStore::IStream* pStream);
	void Save(IFileStore::IOutputStream* pStream);

	size_t GetMaterialCount() const;
	CMaterial* GetMaterial(size_t i);

protected:
	CChunkyFile* m_pChunky;
	std::vector<CMaterial*> m_vMaterials;

	void _Free();
	void _ParseChunk(CChunkyFile::CChunk* pChunk);
	bool _TSETPrune(CChunkyFile::CChunk* pChunk, std::vector<std::pair<bool, const char*> > &vUsedTextures, size_t &iTSETCount);
};

#endif

