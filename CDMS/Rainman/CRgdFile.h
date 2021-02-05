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

#ifndef _C_RGD_FILE_H_
#define _C_RGD_FILE_H_

#include "gnuc_defines.h"
#include <vector>
#include "IFileStore.h"
#include "IMetaTable.h"
#include "CRgdHashTable.h"
#include "CLuaFile.h"
#include "Api.h"

class RAINMAN_API CRgdFile : public IMetaNode
{
public:
	static const int sk_TableInt = 101;
	struct _RgdEntry;

	union _RgdEntryData
	{
		float f;
		unsigned long i;
		char* s;
		bool b;
		wchar_t* ws;
		std::vector<_RgdEntry*>* t;
	};

	struct _RgdEntry
	{
		CRgdFile* pParentFile;
		unsigned long iHash;
		const char* sName;
		eDataTypes Type;
		_RgdEntryData Data;
		_RgdEntry* pExt;
	};

	static bool _SortOutEntries(_RgdEntry* a, _RgdEntry* b);
	static bool _SortOutEntriesNum(_RgdEntry* a, _RgdEntry* b);
public:
	CRgdFile(void);
	virtual ~CRgdFile(void);

	/*
	enum eValues
	{
		// Errors
		E_NoStream = 0, // No stream was passed into the function
		E_MemoryAllocateError, // Couldn't allocate enough memory
		E_ReadError, // Could not read data from stream
		E_WriteError, // Could not write data to stream
		E_NoData, // No DATAAEGD segment found
		E_MemoryReadError, // Couldn't read from memory
		E_SyntaxError, // Couldn't process binary data into a table
		E_InvalidDataType = 15, // Invalid LUA data type

		// Warnings (Can continue but may not work)
		E_UnrecognisedHeader = 7, // The RGD file had an unrecognised header
		E_InvalidCRC, // The CRC doesn't match

		// No Errors
		E_OK, // Everything went fine

		//Other
		E_UnknownError // Um, OMGWTFHAX?!?
	};
	*/

	void New(long iVersion = 1);
	void Load(IFileStore::IStream *pStream);
	void Load(CLuaFile *pLuaFile, long iVersion = 1);
	/*!
		\todo check DT_NoData and DT_WString references
	*/
	void Save(IFileStore::IOutputStream *pStream);

	//! Creates a lua_State from the RGD state.
	/*!
		Use MakeLuaFromRgdAndNil() or MakeLuaFromRgdQuickly() to make LUA
		files from an RGD. Use this to make a LUA state which can then be
		used to run scripts over.
	*/
	lua_State* CreateLuaState();

	CRgdHashTable* GetHashTable();
	void SetHashTable(CRgdHashTable *pTable);

	// These only manipulate the (first) DATAAEGD chunk (hopefully, RGDs only _need_ that chunk)
	const char* GetDescriptorString();
	void SetDescriptorString(const char* sString);
	long GetChunkVersion();
	void SetChunkVersion(long iVersion);

	// All node types
	virtual eDataTypes VGetType();
	virtual const char* VGetName();

	// Floats, Bools, Strings , WStrings
	virtual float VGetValueFloat();
	virtual unsigned long VGetValueInteger();
	virtual bool VGetValueBool();
	virtual const char* VGetValueString();
	virtual const wchar_t* VGetValueWString();
	virtual IMetaTable* VGetValueMetatable();

	// Set
	virtual void VSetType(eDataTypes eType);
	virtual void VSetName(const char* sName);
	virtual void VSetNameHash(long iHash);
	virtual void VSetValueFloat(float fValue);
	virtual void VSetValueInteger(unsigned long iValue);
	virtual void VSetValueBool(bool bValue);
	virtual void VSetValueString(const char* sValue);
	virtual void VSetValueWString(const wchar_t* wsValue);

	virtual CMemoryStore::COutStream* VGetNodeAsRainmanRgd();
	virtual void SGetNodeFromRainmanRgd(IFileStore::IStream* pInput, bool bSetName = false);

	class CMetaTable;

	class RAINMAN_API CMetaNode : public IMetaNode
	{
	protected:
		friend class CRgdFile;
		friend class CMetaTable;
		CMetaNode(_RgdEntry* pData);
		_RgdEntry* m_pData;
	public:
		virtual ~CMetaNode();
		virtual eDataTypes VGetType();
		virtual const char* VGetName();
		virtual unsigned long VGetNameHash();

		// Floats, Bools, Strings , WStrings
		virtual float VGetValueFloat();
		virtual unsigned long VGetValueInteger();
		virtual bool VGetValueBool();
		virtual const char* VGetValueString();
		virtual const wchar_t* VGetValueWString();
		virtual IMetaNode::IMetaTable* VGetValueMetatable();

		// Set
		virtual void VSetType(eDataTypes eType);
		virtual void VSetName(const char* sName);
		virtual void VSetNameHash(unsigned long iHash);
		virtual void VSetValueFloat(float fValue);
		virtual void VSetValueInteger(unsigned long iValue);
		virtual void VSetValueBool(bool bValue);
		virtual void VSetValueString(const char* sValue);
		virtual void VSetValueWString(const wchar_t* wsValue);

		virtual CMemoryStore::COutStream* VGetNodeAsRainmanRgd();
		virtual void SGetNodeFromRainmanRgd(IFileStore::IStream* pInput, bool bSetName = false);
	};

	class RAINMAN_API CMetaTable : public IMetaNode::IMetaTable
	{
	protected:
		friend class CMetaNode;
		friend class CRgdFile;
		CMetaTable(_RgdEntry* pData);
		_RgdEntry* m_pData;
		std::vector<_RgdEntry*> m_vecChildren;
	public:
		virtual ~CMetaTable();

		virtual unsigned long VGetChildCount();
		virtual IMetaNode* VGetChild(unsigned long iIndex);

		virtual IMetaNode::eDataTypes VGetReferenceType();
		virtual const char* VGetReferenceString();
		virtual const wchar_t* VGetReferenceWString();

		// Set
		virtual void VSetReferenceType(IMetaNode::eDataTypes eType);
		virtual void VSetReferenceString(const char* sValue);
		virtual void VSetReferenceWString(const wchar_t* wsValue);
		virtual IMetaNode* VAddChild(const char* sName);
		virtual void VDeleteChild(unsigned long iIndex);
	};

	static void _CleanRgdTable(_RgdEntry *pTable); // Erases the RGD table from memory
protected:
	friend class CMetaNode;
	friend class CMetaTable;
	friend class CRgdFileMacro;

	struct _RgdHeader // aka. Chunky header?
	{
		char *sHeader; // 16 bytes "Relic Chunky\x0D\x0A\x1A\x00"
		long iVersion; // 1 - is the same in all (is 3 in CoH)
		long iUnknown3; // 1 - is the same in all

		// v3 only:
		long iUnknown4;
		long iUnknown5;
		long iUnknown6;
	};

	struct _RgdChunk
	{
		char *sChunkyType; // 8 bytes "DATAAEGD"
		long iVersion; // CoH = 1. DoW = ?
		long iChunkLength; // iStringLength  + sizeof(iCRC) + sizeof(iDataLength) + iDataLength
		long iStringLength;
		char* sString; // variable length

		// these two only in v3 chunky
		unsigned long iUnknown1;
		unsigned long iUnknown2;

		unsigned long iCRC; // CRC32 of pData
		long iDataLength;
		char* pData;
		_RgdEntry RootEntry;
	};

	_RgdHeader m_RgdHeader;
	std::vector<_RgdChunk*> m_vRgdChunks;
	_RgdChunk* m_pDataChunk;
	CRgdHashTable* m_pHashTable;
	bool m_bConvertTableIntToTable;

	void _Clean(); // Erases the currently loaded RGD file from memory

	/*!
		Sets the top of the lus_State's stack to the specified table
	*/
	static void _RgdTableToLuaState(lua_State* L, _RgdEntry *pTable);

	void _ProcessRawRgdData(IFileStore::IStream *pStream, _RgdEntry *pDestination);
	void _WriteRawRgdData(IFileStore::IOutputStream *pStream, _RgdEntry *pSource, bool bTable101 = false);
	friend bool _SortCMetaTableChildren(CRgdFile::_RgdEntry* p1, CRgdFile::_RgdEntry* p2);
	void _LoadLua(lua_State* L, _RgdEntry* pDest, bool bSkipThisLevelRef = false);

	static void _WriteRainmanRgdData(CMemoryStore::COutStream* pOutput, _RgdEntry* pSource);
	static void _ReadRainmanRgdData(IFileStore::IStream* pInput, _RgdEntry* pDestination, bool bSetName);
};

#endif

