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

#include "CLuaScript_Interface.h"

class Lua_Istream
{
public:
	static int __gc(lua_State *L)
	{
		IFileStore::IStream* obj = *(IFileStore::IStream**)lua_touserdata(L, 1);
		delete obj;
		return 0;
	}
};

void LuaBind_Istream(lua_State *L, IFileStore::IStream* pObj, bool bOwn)
{
	IFileStore::IStream** pUD = (IFileStore::IStream**)lua_newuserdata(L, sizeof(IFileStore::IStream*) );
	*pUD = pObj;
	lua_newtable(L);
	lua_pushlightuserdata(L, lua_touserdata(L, -1));
	lua_pushlightuserdata(L, (void*)0x00321168);
	lua_settable(L, LUA_REGISTRYINDEX);
	lua_pushstring(L, "__index");
	lua_pushstring(L, "Istream");
	lua_gettable(L, LUA_GLOBALSINDEX);
	lua_settable(L, -3);
	if(bOwn)
	{
		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, Lua_Istream::__gc);
		lua_settable(L, -3);
	}
	lua_setmetatable(L, -2);
}

class Lua_IoutputStream
{
public:
	static int __gc(lua_State *L)
	{
		IFileStore::IOutputStream* obj = *(IFileStore::IOutputStream**)lua_touserdata(L, 1);
		delete obj;
		return 0;
	}
};

void LuaBind_IoutputStream(lua_State *L, IFileStore::IOutputStream* pObj, bool bOwn)
{
	IFileStore::IOutputStream** pUD = (IFileStore::IOutputStream**)lua_newuserdata(L, sizeof(IFileStore::IOutputStream*) );
	*pUD = pObj;
	lua_newtable(L);
	lua_pushlightuserdata(L, lua_touserdata(L, -1));
	lua_pushlightuserdata(L, (void*)0x003212F0);
	lua_settable(L, LUA_REGISTRYINDEX);
	lua_pushstring(L, "__index");
	lua_pushstring(L, "IoutputStream");
	lua_gettable(L, LUA_GLOBALSINDEX);
	lua_settable(L, -3);
	if(bOwn)
	{
		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, Lua_IoutputStream::__gc);
		lua_settable(L, -3);
	}
	lua_setmetatable(L, -2);
}

class Lua_CsgaFile
{
public:
	static int __gc(lua_State *L)
	{
		CSgaFile* obj = *(CSgaFile**)lua_touserdata(L, 1);
		delete obj;
		return 0;
	}
};

void LuaBind_CsgaFile(lua_State *L, CSgaFile* pObj, bool bOwn)
{
	CSgaFile** pUD = (CSgaFile**)lua_newuserdata(L, sizeof(CSgaFile*) );
	*pUD = pObj;
	lua_newtable(L);
	lua_pushlightuserdata(L, lua_touserdata(L, -1));
	lua_pushlightuserdata(L, (void*)0x00321488);
	lua_settable(L, LUA_REGISTRYINDEX);
	lua_pushstring(L, "__index");
	lua_pushstring(L, "CsgaFile");
	lua_gettable(L, LUA_GLOBALSINDEX);
	lua_settable(L, -3);
	if(bOwn)
	{
		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, Lua_CsgaFile::__gc);
		lua_settable(L, -3);
	}
	lua_setmetatable(L, -2);
}

class Lua_CucsFile
{
public:
	static int __gc(lua_State *L)
	{
		CUcsFile* obj = *(CUcsFile**)lua_touserdata(L, 1);
		delete obj;
		return 0;
	}
};

void LuaBind_CucsFile(lua_State *L, CUcsFile* pObj, bool bOwn)
{
	CUcsFile** pUD = (CUcsFile**)lua_newuserdata(L, sizeof(CUcsFile*) );
	*pUD = pObj;
	lua_newtable(L);
	lua_pushlightuserdata(L, lua_touserdata(L, -1));
	lua_pushlightuserdata(L, (void*)0x00321608);
	lua_settable(L, LUA_REGISTRYINDEX);
	lua_pushstring(L, "__index");
	lua_pushstring(L, "CucsFile");
	lua_gettable(L, LUA_GLOBALSINDEX);
	lua_settable(L, -3);
	if(bOwn)
	{
		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, Lua_CucsFile::__gc);
		lua_settable(L, -3);
	}
	lua_setmetatable(L, -2);
}

class Lua_CdowModule
{
public:
	static int f0_setLocale(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* arg2 = lua_tostring(L, 2);
		bool ret1 = arg1->SetLocale(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f1_getLocale(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* ret1 = arg1->GetLocale();
		lua_pushstring(L, ret1);
		return 1;
	}
	static int f2_new(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		bool ret1 = arg1->New();
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f3_load(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* arg2 = lua_tostring(L, 2);
		bool ret1 = arg1->Load(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f4_save(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* arg2 = lua_tostring(L, 2);
		bool ret1 = arg1->Save(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f5_rebuildFileview(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* arg2 = lua_tostring(L, 2);
		bool ret1 = arg1->RebuildFileview(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f6_getFileviewHash(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		unsigned long ret1 = arg1->GetFileviewHash();
		lua_pushnumber(L, (lua_Number)ret1);
		return 1;
	}
	static int f7_getFilesModName(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* ret1 = arg1->GetFilesModName();
		lua_pushstring(L, ret1);
		return 1;
	}
	static int f8_getFilesEngineName(lua_State *L)
	{
		const char* ret1 = CDoWModule::GetEngineModName();
		lua_pushstring(L, ret1);
		return 1;
	}
	static int f9_Vinit(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		bool ret1 = arg1->VInit();
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f10_VopenStream(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* arg2 = lua_tostring(L, 2);
		IFileStore::IStream* ret1 = arg1->VOpenStream(arg2);
		LuaBind_Istream(L, ret1, true);
		return 1;
	}
	static int f11_VopenOutputStream(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* arg2 = lua_tostring(L, 2);
		bool arg3 = (lua_toboolean(L, 3) ? true : false);
		IFileStore::IOutputStream* ret1 = arg1->VOpenOutputStream(arg2, arg3);
		LuaBind_IoutputStream(L, ret1, true);
		return 1;
	}
	static int f12_getUiName(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* ret1 = arg1->GetUIName();
		lua_pushstring(L, ret1);
		return 1;
	}
	static int f13_getDescription(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* ret1 = arg1->GetDescription();
		lua_pushstring(L, ret1);
		return 1;
	}
	static int f14_getDllName(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* ret1 = arg1->GetDllName();
		lua_pushstring(L, ret1);
		return 1;
	}
	static int f15_getModFolder(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* ret1 = arg1->GetModFolder();
		lua_pushstring(L, ret1);
		return 1;
	}
	static int f16_getTextureFe(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* ret1 = arg1->GetTextureFE();
		lua_pushstring(L, ret1);
		return 1;
	}
	static int f17_getTextureIcon(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* ret1 = arg1->GetTextureIcon();
		lua_pushstring(L, ret1);
		return 1;
	}
	static int f18_getVersionMajor(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long ret1 = arg1->GetVersionMajor();
		lua_pushnumber(L, (lua_Number)ret1);
		return 1;
	}
	static int f19_getVersionMinor(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long ret1 = arg1->GetVersionMinor();
		lua_pushnumber(L, (lua_Number)ret1);
		return 1;
	}
	static int f20_getVersionRevision(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long ret1 = arg1->GetVersionRevision();
		lua_pushnumber(L, (lua_Number)ret1);
		return 1;
	}
	static int f21_setUiName(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* arg2 = lua_tostring(L, 2);
		bool ret1 = arg1->SetUIName(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f22_setDescription(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* arg2 = lua_tostring(L, 2);
		bool ret1 = arg1->SetDescription(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f23_setDllName(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* arg2 = lua_tostring(L, 2);
		bool ret1 = arg1->SetDllName(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f24_setModFolder(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* arg2 = lua_tostring(L, 2);
		bool ret1 = arg1->SetModFolder(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f25_setTextureFe(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* arg2 = lua_tostring(L, 2);
		bool ret1 = arg1->SetTextureFE(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f26_setTextureIcon(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* arg2 = lua_tostring(L, 2);
		bool ret1 = arg1->SetTextureIcon(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f27_setVersionMajor(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		bool ret1 = arg1->SetVersionMajor(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f28_setVersionMinor(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		bool ret1 = arg1->SetVersionMinor(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f29_setVersionRevision(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		bool ret1 = arg1->SetVersionRevision(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f30_getDataFolderCount(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long ret1 = arg1->GetDataFolderCount();
		lua_pushnumber(L, (lua_Number)ret1);
		return 1;
	}
	static int f31_getDataFolderName(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		const char* ret1 = arg1->GetDataFolder(arg2);
		lua_pushstring(L, ret1);
		return 1;
	}
	static int f32_getDataFolderId(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		long ret1 = arg1->GetDataFolderID(arg2);
		lua_pushnumber(L, (lua_Number)ret1);
		return 1;
	}
	static int f33_dataFolderSwap(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		long arg3 = (long)(lua_tonumber(L, 3) + 0.5f);
		bool ret1 = arg1->SwapDataFolders(arg2, arg3);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f34_dataFolderAdd(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* arg2 = lua_tostring(L, 2);
		bool ret1 = arg1->AddDataFolder(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f35_dataFolderRemove(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		bool ret1 = arg1->RemoveDataFolder(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f36_getDataArchiveCount(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long ret1 = arg1->GetArchiveCount();
		lua_pushnumber(L, (lua_Number)ret1);
		return 1;
	}
	static int f37_getDataArchiveName(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		const char* ret1 = arg1->GetArchive(arg2);
		lua_pushstring(L, ret1);
		return 1;
	}
	static int f38_getDataArchiveId(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		long ret1 = arg1->GetArchiveID(arg2);
		lua_pushnumber(L, (lua_Number)ret1);
		return 1;
	}
	static int f39_getDataArchiveHandle(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		CSgaFile* ret1 = arg1->GetArchiveHandle(arg2);
		LuaBind_CsgaFile(L, ret1, false);
		return 1;
	}
	static int f40_dataArchiveSwap(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		long arg3 = (long)(lua_tonumber(L, 3) + 0.5f);
		bool ret1 = arg1->SwapArchives(arg2, arg3);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f41_dataArchiveAdd(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* arg2 = lua_tostring(L, 2);
		bool ret1 = arg1->AddArchive(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f42_dataArchiveRemove(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		bool ret1 = arg1->RemoveArchive(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f43_getRequiredModCount(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long ret1 = arg1->GetRequiredCount();
		lua_pushnumber(L, (lua_Number)ret1);
		return 1;
	}
	static int f44_getRequiredModName(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		const char* ret1 = arg1->GetRequired(arg2);
		lua_pushstring(L, ret1);
		return 1;
	}
	static int f45_getRequiredModId(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		long ret1 = arg1->GetRequiredID(arg2);
		lua_pushnumber(L, (lua_Number)ret1);
		return 1;
	}
	static int f46_getRequiredModHandle(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		CDoWModule* ret1 = arg1->GetRequiredHandle(arg2);
		LuaBind_CdowModule(L, ret1, false);
		return 1;
	}
	static int f47_requiredModSwap(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		long arg3 = (long)(lua_tonumber(L, 3) + 0.5f);
		bool ret1 = arg1->SwapRequireds(arg2, arg3);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f48_requiredModAdd(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* arg2 = lua_tostring(L, 2);
		bool ret1 = arg1->AddRequired(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f49_requiredModRemove(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		bool ret1 = arg1->RemoveRequired(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f50_getCompatibleModCount(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long ret1 = arg1->GetCompatableCount();
		lua_pushnumber(L, (lua_Number)ret1);
		return 1;
	}
	static int f51_getCompatibleModName(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		const char* ret1 = arg1->GetCompatable(arg2);
		lua_pushstring(L, ret1);
		return 1;
	}
	static int f52_getCompatibleModId(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		long ret1 = arg1->GetCompatableID(arg2);
		lua_pushnumber(L, (lua_Number)ret1);
		return 1;
	}
	static int f53_compatibleModSwap(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		long arg3 = (long)(lua_tonumber(L, 3) + 0.5f);
		bool ret1 = arg1->SwapCompatables(arg2, arg3);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f54_compatibleModAdd(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* arg2 = lua_tostring(L, 2);
		bool ret1 = arg1->AddCompatable(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f55_compatibleModRemove(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		bool ret1 = arg1->RemoveCompatable(arg2);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int f56_getUcsByString(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* arg2 = lua_tostring(L, 2);
		const wchar_t* ret1 = arg1->ResolveUCS(arg2);
		size_t ret1_l = wcslen(ret1) + 1;
		char* ret1_a = new char[ret1_l];
		for(size_t i = 0; i < ret1_l; ++i) ret1_a[i] = (char)ret1[i];
		lua_pushstring(L, ret1_a);
		delete[] ret1_a;
		return 1;
	}
	static int f57_getUcsByInt(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		unsigned long arg2 = (unsigned long)(lua_tonumber(L, 2) + 0.5f);
		const wchar_t* ret1 = arg1->ResolveUCS(arg2);
		size_t ret1_l = wcslen(ret1) + 1;
		char* ret1_a = new char[ret1_l];
		for(size_t i = 0; i < ret1_l; ++i) ret1_a[i] = (char)ret1[i];
		lua_pushstring(L, ret1_a);
		delete[] ret1_a;
		return 1;
	}
	static int f58_getUcsFileCount(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long ret1 = arg1->GetUcsFileCount();
		lua_pushnumber(L, (lua_Number)ret1);
		return 1;
	}
	static int f59_getUcsFileName(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		const char* ret1 = arg1->GetUcsFileName(arg2);
		lua_pushstring(L, ret1);
		return 1;
	}
	static int f60_getUcsFileHandle(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		long arg2 = (long)(lua_tonumber(L, 2) + 0.5f);
		CUcsFile* ret1 = arg1->GetUcsFile(arg2);
		LuaBind_CucsFile(L, ret1, false);
		return 1;
	}
	static int f61_ucsAdd(lua_State *L)
	{
		CDoWModule* arg1 = *(CDoWModule**)lua_touserdata(L, 1);
		const char* arg2 = lua_tostring(L, 2);
		CUcsFile* arg3= *(CUcsFile**)lua_touserdata(L, 3);
		lua_getmetatable(L, 3);
		lua_pushstring(L, "__gc");
		lua_gettable(L, -2);
		if(lua_isnil(L, -1))
		{
			lua_pop(L, 2);
			lua_pushstring(L, "Cannot pass argument #3 as it is not owned by lua");
			lua_error(L);
		}
		else
		{
			lua_pop(L, 1);
			lua_pushstring(L, "__gc");
			lua_pushnil(L);
			lua_settable(L, -3);
			lua_pop(L, 1);
		}
		bool ret1 = arg1->RegisterNewUCS(arg2, arg3);
		lua_pushboolean(L, ret1 ? 1 : 0);
		return 1;
	}
	static int __gc(lua_State *L)
	{
		CDoWModule* obj = *(CDoWModule**)lua_touserdata(L, 1);
		delete obj;
		return 0;
	}
};

void LuaBind_CdowModule(lua_State *L, CDoWModule* pObj, bool bOwn)
{
	CDoWModule** pUD = (CDoWModule**)lua_newuserdata(L, sizeof(CDoWModule*) );
	*pUD = pObj;
	lua_newtable(L);
	lua_pushlightuserdata(L, lua_touserdata(L, -1));
	lua_pushlightuserdata(L, (void*)0x00321A58);
	lua_settable(L, LUA_REGISTRYINDEX);
	lua_pushstring(L, "__index");
	lua_pushstring(L, "CdowModule");
	lua_gettable(L, LUA_GLOBALSINDEX);
	lua_settable(L, -3);
	if(bOwn)
	{
		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, Lua_CdowModule::__gc);
		lua_settable(L, -3);
	}
	lua_setmetatable(L, -2);
}

int Lua_f0_isUcsReference(lua_State *L)
{
	const char* arg1 = lua_tostring(L, 1);
	bool ret1 = CDoWModule::IsDollarString(arg1);
	lua_pushboolean(L, ret1 ? 1 : 0);
	return 1;
}
void LuaBind_Globals(lua_State *L)
{
	lua_pushstring(L, "Istream");
	lua_newtable(L);
	lua_settable(L, LUA_GLOBALSINDEX);
	lua_pushstring(L, "IoutputStream");
	lua_newtable(L);
	lua_settable(L, LUA_GLOBALSINDEX);
	lua_pushstring(L, "CsgaFile");
	lua_newtable(L);
	lua_settable(L, LUA_GLOBALSINDEX);
	lua_pushstring(L, "CucsFile");
	lua_newtable(L);
	lua_settable(L, LUA_GLOBALSINDEX);
	lua_pushstring(L, "CdowModule");
	lua_newtable(L);
	lua_pushstring(L, "setLocale");
	lua_pushcfunction(L, Lua_CdowModule::f0_setLocale);
	lua_settable(L, -3);
	lua_pushstring(L, "getLocale");
	lua_pushcfunction(L, Lua_CdowModule::f1_getLocale);
	lua_settable(L, -3);
	lua_pushstring(L, "new");
	lua_pushcfunction(L, Lua_CdowModule::f2_new);
	lua_settable(L, -3);
	lua_pushstring(L, "load");
	lua_pushcfunction(L, Lua_CdowModule::f3_load);
	lua_settable(L, -3);
	lua_pushstring(L, "save");
	lua_pushcfunction(L, Lua_CdowModule::f4_save);
	lua_settable(L, -3);
	lua_pushstring(L, "rebuildFileview");
	lua_pushcfunction(L, Lua_CdowModule::f5_rebuildFileview);
	lua_settable(L, -3);
	lua_pushstring(L, "getFileviewHash");
	lua_pushcfunction(L, Lua_CdowModule::f6_getFileviewHash);
	lua_settable(L, -3);
	lua_pushstring(L, "getFilesModName");
	lua_pushcfunction(L, Lua_CdowModule::f7_getFilesModName);
	lua_settable(L, -3);
	lua_pushstring(L, "getFilesEngineName");
	lua_pushcfunction(L, Lua_CdowModule::f8_getFilesEngineName);
	lua_settable(L, -3);
	lua_pushstring(L, "Vinit");
	lua_pushcfunction(L, Lua_CdowModule::f9_Vinit);
	lua_settable(L, -3);
	lua_pushstring(L, "VopenStream");
	lua_pushcfunction(L, Lua_CdowModule::f10_VopenStream);
	lua_settable(L, -3);
	lua_pushstring(L, "VopenOutputStream");
	lua_pushcfunction(L, Lua_CdowModule::f11_VopenOutputStream);
	lua_settable(L, -3);
	lua_pushstring(L, "getUiName");
	lua_pushcfunction(L, Lua_CdowModule::f12_getUiName);
	lua_settable(L, -3);
	lua_pushstring(L, "getDescription");
	lua_pushcfunction(L, Lua_CdowModule::f13_getDescription);
	lua_settable(L, -3);
	lua_pushstring(L, "getDllName");
	lua_pushcfunction(L, Lua_CdowModule::f14_getDllName);
	lua_settable(L, -3);
	lua_pushstring(L, "getModFolder");
	lua_pushcfunction(L, Lua_CdowModule::f15_getModFolder);
	lua_settable(L, -3);
	lua_pushstring(L, "getTextureFe");
	lua_pushcfunction(L, Lua_CdowModule::f16_getTextureFe);
	lua_settable(L, -3);
	lua_pushstring(L, "getTextureIcon");
	lua_pushcfunction(L, Lua_CdowModule::f17_getTextureIcon);
	lua_settable(L, -3);
	lua_pushstring(L, "getVersionMajor");
	lua_pushcfunction(L, Lua_CdowModule::f18_getVersionMajor);
	lua_settable(L, -3);
	lua_pushstring(L, "getVersionMinor");
	lua_pushcfunction(L, Lua_CdowModule::f19_getVersionMinor);
	lua_settable(L, -3);
	lua_pushstring(L, "getVersionRevision");
	lua_pushcfunction(L, Lua_CdowModule::f20_getVersionRevision);
	lua_settable(L, -3);
	lua_pushstring(L, "setUiName");
	lua_pushcfunction(L, Lua_CdowModule::f21_setUiName);
	lua_settable(L, -3);
	lua_pushstring(L, "setDescription");
	lua_pushcfunction(L, Lua_CdowModule::f22_setDescription);
	lua_settable(L, -3);
	lua_pushstring(L, "setDllName");
	lua_pushcfunction(L, Lua_CdowModule::f23_setDllName);
	lua_settable(L, -3);
	lua_pushstring(L, "setModFolder");
	lua_pushcfunction(L, Lua_CdowModule::f24_setModFolder);
	lua_settable(L, -3);
	lua_pushstring(L, "setTextureFe");
	lua_pushcfunction(L, Lua_CdowModule::f25_setTextureFe);
	lua_settable(L, -3);
	lua_pushstring(L, "setTextureIcon");
	lua_pushcfunction(L, Lua_CdowModule::f26_setTextureIcon);
	lua_settable(L, -3);
	lua_pushstring(L, "setVersionMajor");
	lua_pushcfunction(L, Lua_CdowModule::f27_setVersionMajor);
	lua_settable(L, -3);
	lua_pushstring(L, "setVersionMinor");
	lua_pushcfunction(L, Lua_CdowModule::f28_setVersionMinor);
	lua_settable(L, -3);
	lua_pushstring(L, "setVersionRevision");
	lua_pushcfunction(L, Lua_CdowModule::f29_setVersionRevision);
	lua_settable(L, -3);
	lua_pushstring(L, "getDataFolderCount");
	lua_pushcfunction(L, Lua_CdowModule::f30_getDataFolderCount);
	lua_settable(L, -3);
	lua_pushstring(L, "getDataFolderName");
	lua_pushcfunction(L, Lua_CdowModule::f31_getDataFolderName);
	lua_settable(L, -3);
	lua_pushstring(L, "getDataFolderId");
	lua_pushcfunction(L, Lua_CdowModule::f32_getDataFolderId);
	lua_settable(L, -3);
	lua_pushstring(L, "dataFolderSwap");
	lua_pushcfunction(L, Lua_CdowModule::f33_dataFolderSwap);
	lua_settable(L, -3);
	lua_pushstring(L, "dataFolderAdd");
	lua_pushcfunction(L, Lua_CdowModule::f34_dataFolderAdd);
	lua_settable(L, -3);
	lua_pushstring(L, "dataFolderRemove");
	lua_pushcfunction(L, Lua_CdowModule::f35_dataFolderRemove);
	lua_settable(L, -3);
	lua_pushstring(L, "getDataArchiveCount");
	lua_pushcfunction(L, Lua_CdowModule::f36_getDataArchiveCount);
	lua_settable(L, -3);
	lua_pushstring(L, "getDataArchiveName");
	lua_pushcfunction(L, Lua_CdowModule::f37_getDataArchiveName);
	lua_settable(L, -3);
	lua_pushstring(L, "getDataArchiveId");
	lua_pushcfunction(L, Lua_CdowModule::f38_getDataArchiveId);
	lua_settable(L, -3);
	lua_pushstring(L, "getDataArchiveHandle");
	lua_pushcfunction(L, Lua_CdowModule::f39_getDataArchiveHandle);
	lua_settable(L, -3);
	lua_pushstring(L, "dataArchiveSwap");
	lua_pushcfunction(L, Lua_CdowModule::f40_dataArchiveSwap);
	lua_settable(L, -3);
	lua_pushstring(L, "dataArchiveAdd");
	lua_pushcfunction(L, Lua_CdowModule::f41_dataArchiveAdd);
	lua_settable(L, -3);
	lua_pushstring(L, "dataArchiveRemove");
	lua_pushcfunction(L, Lua_CdowModule::f42_dataArchiveRemove);
	lua_settable(L, -3);
	lua_pushstring(L, "getRequiredModCount");
	lua_pushcfunction(L, Lua_CdowModule::f43_getRequiredModCount);
	lua_settable(L, -3);
	lua_pushstring(L, "getRequiredModName");
	lua_pushcfunction(L, Lua_CdowModule::f44_getRequiredModName);
	lua_settable(L, -3);
	lua_pushstring(L, "getRequiredModId");
	lua_pushcfunction(L, Lua_CdowModule::f45_getRequiredModId);
	lua_settable(L, -3);
	lua_pushstring(L, "getRequiredModHandle");
	lua_pushcfunction(L, Lua_CdowModule::f46_getRequiredModHandle);
	lua_settable(L, -3);
	lua_pushstring(L, "requiredModSwap");
	lua_pushcfunction(L, Lua_CdowModule::f47_requiredModSwap);
	lua_settable(L, -3);
	lua_pushstring(L, "requiredModAdd");
	lua_pushcfunction(L, Lua_CdowModule::f48_requiredModAdd);
	lua_settable(L, -3);
	lua_pushstring(L, "requiredModRemove");
	lua_pushcfunction(L, Lua_CdowModule::f49_requiredModRemove);
	lua_settable(L, -3);
	lua_pushstring(L, "getCompatibleModCount");
	lua_pushcfunction(L, Lua_CdowModule::f50_getCompatibleModCount);
	lua_settable(L, -3);
	lua_pushstring(L, "getCompatibleModName");
	lua_pushcfunction(L, Lua_CdowModule::f51_getCompatibleModName);
	lua_settable(L, -3);
	lua_pushstring(L, "getCompatibleModId");
	lua_pushcfunction(L, Lua_CdowModule::f52_getCompatibleModId);
	lua_settable(L, -3);
	lua_pushstring(L, "compatibleModSwap");
	lua_pushcfunction(L, Lua_CdowModule::f53_compatibleModSwap);
	lua_settable(L, -3);
	lua_pushstring(L, "compatibleModAdd");
	lua_pushcfunction(L, Lua_CdowModule::f54_compatibleModAdd);
	lua_settable(L, -3);
	lua_pushstring(L, "compatibleModRemove");
	lua_pushcfunction(L, Lua_CdowModule::f55_compatibleModRemove);
	lua_settable(L, -3);
	lua_pushstring(L, "getUcsByString");
	lua_pushcfunction(L, Lua_CdowModule::f56_getUcsByString);
	lua_settable(L, -3);
	lua_pushstring(L, "getUcsByInt");
	lua_pushcfunction(L, Lua_CdowModule::f57_getUcsByInt);
	lua_settable(L, -3);
	lua_pushstring(L, "getUcsFileCount");
	lua_pushcfunction(L, Lua_CdowModule::f58_getUcsFileCount);
	lua_settable(L, -3);
	lua_pushstring(L, "getUcsFileName");
	lua_pushcfunction(L, Lua_CdowModule::f59_getUcsFileName);
	lua_settable(L, -3);
	lua_pushstring(L, "getUcsFileHandle");
	lua_pushcfunction(L, Lua_CdowModule::f60_getUcsFileHandle);
	lua_settable(L, -3);
	lua_pushstring(L, "ucsAdd");
	lua_pushcfunction(L, Lua_CdowModule::f61_ucsAdd);
	lua_settable(L, -3);
	lua_settable(L, LUA_GLOBALSINDEX);
	lua_pushstring(L, "isUcsReference");
	lua_pushcfunction(L, Lua_f0_isUcsReference);
	lua_settable(L, LUA_GLOBALSINDEX);
}
