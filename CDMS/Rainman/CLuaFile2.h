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

#ifndef _C_LUA_FILE_2_H_
#define _C_LUA_FILE_2_H_

#include "gnuc_defines.h"
#include "IMetaTable.h"
#include "IFileStore.h"
#include "CLuaFileCache.h"
extern "C" {
#include <lua.h>
};
#include <map>
#include <vector>

class CLuaStateNode;
class CLuaStateStackNode;

class RAINMAN_API CLuaStateTable : public IMetaNode::IMetaTable
{
protected:
	lua_State *mL;
	std::vector<CLuaStateNode*> m_vNodes;
	bool bDeleteNodes;
	friend class CLuaStateStackNode;

public:
	CLuaStateTable(lua_State *L, bool bCustom = false);
	virtual ~CLuaStateTable();

	void add(CLuaStateNode* p);

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

class RAINMAN_API CLuaFile2
{
public:
	CLuaFile2();
	virtual ~CLuaFile2();

	void newFile(const char* sFileName);
	void loadFile(IFileStore::IStream *pStream, IFileStore* pFiles, const char* sFileName);
	void saveFile(IFileStore::IOutputStream *pStream, const char* sFileName);

	bool setCache(CLuaFileCache* pCache, bool bOwn);
	void setRootFolder(const char* sRootFolder);

	IMetaNode::IMetaTable* asMetaTable();

	class RAINMAN_API _LuaLocator
	{
	protected:
		unsigned long* pRefCount;
	public:
		_LuaLocator(_LuaLocator* P);
		_LuaLocator(lua_State* L, int Index);
		void kill(lua_State* L);
		void push(lua_State* L);
	};

	class CTable;

	class RAINMAN_API CNode : public IMetaNode
	{
	protected:
		friend class CLuaFile2::CTable;
		CLuaFile2::_LuaLocator m_oTablePtr;
		CLuaFile2::_LuaLocator m_oKeyPtr;
		char* m_sName;
		unsigned long m_iNodeHash;
		int m_iType;
		lua_State* L;

		void _pushTable();
		void _pushKey();
		void _pushVal();

		CNode(lua_State* Lua, int iKey, int iTable);
		CNode(lua_State* Lua, _LuaLocator* pTable, _LuaLocator* pKey);
	public:
		virtual ~CNode();
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

	class RAINMAN_API CTable : public IMetaNode::IMetaTable
	{
	protected:
		lua_State *L;
		CLuaFile2::_LuaLocator m_oTablePtr;
		std::vector<CLuaFile2::CNode*> m_vNodes;
		char* m_sRef;
		bool m_bGlobals;

		void _DoLoad();
		static bool _SortNodes(CLuaFile2::CNode* p1, CLuaFile2::CNode* p2);
	public:
		CTable(lua_State *pL, bool bG = false);
		virtual ~CTable();

		virtual unsigned long VGetChildCount();
		virtual IMetaNode* VGetChild(unsigned long iIndex);
		virtual IMetaNode::eDataTypes VGetReferenceType();
		virtual const char* VGetReferenceString();
		virtual const wchar_t* VGetReferenceWString();
		virtual void VSetReferenceType(IMetaNode::eDataTypes eType);
		virtual void VSetReferenceString(const char* sValue);
		virtual void VSetReferenceWString(const wchar_t* wsValue);
		virtual IMetaNode* VAddChild(const char* sName);
		virtual void VDeleteChild(unsigned long iIndex);

		virtual bool VSupportsRefresh();
		virtual void VDoRefresh();
	};

protected:
	CLuaFileCache* m_pCache;
	bool m_bOwnCache;
	char* m_sRootFolder;
	lua_State* L;
	char* m_sFileName;

	typedef std::map<unsigned long, int> _tRefMap;
	_tRefMap* m_pRefMap;
	bool m_bOwnRefMap;

	void _saveTable(const char* sPrefix, IFileStore::IOutputStream* pStream, int iMode);
	void _appendEscapedString(char* sDest, const char* sSrc);

	void _clean();
	bool _checkCache();

	struct _SaveKey
	{
		union
		{
			int b; // 3
			const char* s; // 2
			lua_Number f; // 1
		};
		int iWhat;
		static bool _Sort(_SaveKey* p1, _SaveKey* p2);
	};

	struct _AutoClean
	{
		CLuaFile2* p;
		_AutoClean(CLuaFile2* pp) : p(pp) {}
		~_AutoClean() {p->_clean();}
	};

	int _luaParent(const char* sFnName, const char* sTableToGrab);

	static int _luaInherit(lua_State* L);
	static int _luaInheritMeta(lua_State* L);
	static int _luaReference(lua_State* L);
	static int _luaIndexEvent(lua_State* L);

	static CLuaFile2* _getOwner(lua_State* L);
};

class RAINMAN_API CLuaStateNode : public IMetaNode
{
protected:
	lua_State* L;
	CLuaFile2::_LuaLocator m_L;
	char* sName;
	friend class CLuaStateTable;
	friend class CLuaStateStackNode;
	CLuaStateNode(lua_State* pL, int iVal, int iKey);

public:
	virtual ~CLuaStateNode();
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

class RAINMAN_API CLuaStateStackNode : public IMetaNode
{
protected:
	std::vector<CLuaStateNode*> m_vNodes;
	lua_State* m_L;

public:
	CLuaStateStackNode(lua_State* L);
	virtual ~CLuaStateStackNode();
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

#endif