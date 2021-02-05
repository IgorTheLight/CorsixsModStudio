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

#ifndef _C_LUA_FILE_H_
#define _C_LUA_FILE_H_
#include "gnuc_defines.h"
#include "IMetaTable.h"
#include "IFileStore.h"
#include "CLuaFileCache.h"
extern "C" {
#include <lua.h>
};
#include <deque>
#include <vector>

class RAINMAN_API CLuaFile : public IMetaNode::IMetaTable
{
public:
	CLuaFile(void);
	virtual ~CLuaFile(void);

	/*
	enum eValues
	{
		// Errors
		E_NoStream = 0, // No stream was passed into the function
		E_NoState = 0, // No state was passed into the function
		E_MemoryAllocateError, // Couldn't allocate enough memory
		E_ReadError, // Could not read data from stream
		E_WriteError, // Could not write data to stream
		E_NoData, // No GameData table found
		E_SyntaxError = 6, // Error processing LUA
		E_LuaApiError = 11,
		E_CircularReference,
		E_LuaRuntimeError,
		E_LuaProcessingError,

		// No Errors
		E_OK = 9, // Everything went fine

		//Other
		E_UnknownError // Um, OMGWTFHAX?!?
	};
	*/

	void New();
	void Load(IFileStore::IStream *pStream, IFileStore* pFiles, const char* sFileName = 0);
	void Save(IFileStore::IStream *pStream);
	const char* GetLuaError(); // obsolete?
	//static const char* eValueToString(eValues e);

	char* GetReferencedFile(IFileStore::IStream *pStream);

	// MetaTable
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

	void AssignCache(CLuaFileCache* pCache);
protected:
	friend struct tLuaTableProtector;
	friend class RAINMAN_API CRgdFile; // For fast copy
	friend class CLuaAction; // For the next two
	friend class CLuaBurnFolderAction; // For memory
	friend class CLuaBurnFolderIncReqAction; // For memory

	lua_State *m_pLua;
	char* m_sLuaError;
	std::deque<const char*>* m_pRefQueue;
	CLuaFileCache* m_pCache;
	bool m_bOwnCache;
	void _Clean();
	void _CacheLua(const char* sName);

	static int _Inherit(lua_State* L);
	static int _InheritMeta(lua_State* L);
	static int _Reference(lua_State* L);
	static int _Reference_Grabby(lua_State* L);
	static int _Debug(lua_State* L);
	static int _meta_index(lua_State* L);

	struct _NodeLocator
	{
		const _NodeLocator* pParent; // If 0, parent is _G
		union {
		char* sName;
		double fName;
		bool bName;
		};
		enum {
			NT_String,
			NT_Double,
			NT_Boolean,
		} eNameType;

		_NodeLocator();
		_NodeLocator(const _NodeLocator& Other);
		_NodeLocator(lua_State* L, int iIndex, const _NodeLocator* pParent); // init from key in stack + parent

		void GetValue(lua_State* L) const; // push the value onto the stack
		void SetValue(lua_State* L) const; // pop value from stack and set to this
	};

	static bool _SortCMetaTableChildren(_NodeLocator* p1, _NodeLocator* p2);
	typedef std::vector<_NodeLocator*> _NodeList;

	_NodeList m_lstGlobals; // it was once a list - honest!
	_NodeLocator* m_pDowModStudio;

	// Uses table at top of stack and generates a list of children
	static void _TableToNodeList(_NodeList& lstNodeList, lua_State* L,const _NodeLocator* pParent, bool bSkip_G = false);
	void _PopMyMetaFromNodeList();

	friend class CMetaNode;
	friend class CMetaTable;

public:
	class CMetaTable;

	class RAINMAN_API CMetaNode : public IMetaNode
	{
	protected:
		friend class CLuaFile;
		friend class CLuaFile::CMetaTable;
		CMetaNode(const _NodeLocator* pNode, lua_State* L);

		const _NodeLocator* m_pNode;
		char* m_sName;
		bool m_bOwnName; // true = need to free() m_sName
		unsigned long m_iNameHash;
		lua_State* m_pLua;

		char* m_sValue;
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
		/*!
			\todo Implement set routines for lua classes
		*/
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
		CMetaTable(lua_State* L);
		void _Init(const _NodeLocator* pParent); // pParent should have already been Get()ted

		CLuaFile::_NodeList m_vecChildren;
		const _NodeLocator* m_pRef;
		const _NodeLocator* m_pDowModStudio;
		lua_State* m_pLua;
		char* m_sRef;

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
};

#endif

