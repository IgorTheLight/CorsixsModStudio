#include <windows.h>
#include "Api.h"
extern "C" {
#include <stdarg.h>
#include <stddef.h>

#define LUA_API
#define LUALIB_API
#define lua51I_openlib	lua51L_openlib

typedef struct lua_Debug lua_Debug;  /* activation record */
typedef struct lua_State lua_State;
typedef int (*lua_CFunction) (lua_State *L);
typedef const char * (*lua_Reader) (lua_State *L, void *ud, size_t *sz);
typedef int (*lua_Writer) (lua_State *L, const void* p, size_t sz, void* ud);
typedef void * (*lua_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);
typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);
typedef double lua_Number;
typedef ptrdiff_t lua_Integer;

typedef struct luaL_Reg luaL_Reg;
typedef struct luaL_Buffer luaL_Buffer;

LUA_API lua_State *(* lua51_newstate) (lua_Alloc f, void *ud);
LUA_API void       (* lua51_close) (lua_State *L);
LUA_API lua_State *(* lua51_newthread) (lua_State *L);

LUA_API lua_CFunction (* lua51_atpanic) (lua_State *L, lua_CFunction panicf);

LUA_API int   (* lua51_gettop) (lua_State *L);
LUA_API void  (* lua51_settop) (lua_State *L, int idx);
LUA_API void  (* lua51_pushvalue) (lua_State *L, int idx);
LUA_API void  (* lua51_remove) (lua_State *L, int idx);
LUA_API void  (* lua51_insert) (lua_State *L, int idx);
LUA_API void  (* lua51_replace) (lua_State *L, int idx);
LUA_API int   (* lua51_checkstack) (lua_State *L, int sz);

LUA_API void  (* lua51_xmove) (lua_State *from, lua_State *to, int n);

LUA_API int             (* lua51_isnumber) (lua_State *L, int idx);
LUA_API int             (* lua51_isstring) (lua_State *L, int idx);
LUA_API int             (* lua51_iscfunction) (lua_State *L, int idx);
LUA_API int             (* lua51_isuserdata) (lua_State *L, int idx);
LUA_API int             (* lua51_type) (lua_State *L, int idx);
LUA_API const char     *(* lua51_typename) (lua_State *L, int tp);

LUA_API int            (* lua51_equal) (lua_State *L, int idx1, int idx2);
LUA_API int            (* lua51_rawequal) (lua_State *L, int idx1, int idx2);
LUA_API int            (* lua51_lessthan) (lua_State *L, int idx1, int idx2);

LUA_API lua_Number      (* lua51_tonumber) (lua_State *L, int idx);
LUA_API lua_Integer     (* lua51_tointeger) (lua_State *L, int idx);
LUA_API int             (* lua51_toboolean) (lua_State *L, int idx);
LUA_API const char     *(* lua51_tolstring) (lua_State *L, int idx, size_t *len);
LUA_API size_t          (* lua51_objlen) (lua_State *L, int idx);
LUA_API lua_CFunction   (* lua51_tocfunction) (lua_State *L, int idx);
LUA_API void	       *(* lua51_touserdata) (lua_State *L, int idx);
LUA_API lua_State      *(* lua51_tothread) (lua_State *L, int idx);
LUA_API const void     *(* lua51_topointer) (lua_State *L, int idx);

LUA_API void  (* lua51_pushnil) (lua_State *L);
LUA_API void  (* lua51_pushnumber) (lua_State *L, lua_Number n);
LUA_API void  (* lua51_pushinteger) (lua_State *L, lua_Integer n);
LUA_API void  (* lua51_pushlstring) (lua_State *L, const char *s, size_t l);
LUA_API void  (* lua51_pushstring) (lua_State *L, const char *s);
LUA_API const char *(* lua51_pushvfstring) (lua_State *L, const char *fmt,
                                                      va_list argp);
LUA_API const char *(* lua51_pushfstring) (lua_State *L, const char *fmt, ...);
LUA_API void  (* lua51_pushcclosure) (lua_State *L, lua_CFunction fn, int n);
LUA_API void  (* lua51_pushboolean) (lua_State *L, int b);
LUA_API void  (* lua51_pushlightuserdata) (lua_State *L, void *p);
LUA_API int   (* lua51_pushthread) (lua_State *L);

LUA_API void  (* lua51_gettable) (lua_State *L, int idx);
LUA_API void  (* lua51_getfield) (lua_State *L, int idx, const char *k);
LUA_API void  (* lua51_rawget) (lua_State *L, int idx);
LUA_API void  (* lua51_rawgeti) (lua_State *L, int idx, int n);
LUA_API void  (* lua51_createtable) (lua_State *L, int narr, int nrec);
LUA_API void *(* lua51_newuserdata) (lua_State *L, size_t sz);
LUA_API int   (* lua51_getmetatable) (lua_State *L, int objindex);
LUA_API void  (* lua51_getfenv) (lua_State *L, int idx);

LUA_API void  (* lua51_settable) (lua_State *L, int idx);
LUA_API void  (* lua51_setfield) (lua_State *L, int idx, const char *k);
LUA_API void  (* lua51_rawset) (lua_State *L, int idx);
LUA_API void  (* lua51_rawseti) (lua_State *L, int idx, int n);
LUA_API int   (* lua51_setmetatable) (lua_State *L, int objindex);
LUA_API int   (* lua51_setfenv) (lua_State *L, int idx);

LUA_API void  (* lua51_call) (lua_State *L, int nargs, int nresults);
LUA_API int   (* lua51_pcall) (lua_State *L, int nargs, int nresults, int errfunc);
LUA_API int   (* lua51_cpcall) (lua_State *L, lua_CFunction func, void *ud);
LUA_API int   (* lua51_load) (lua_State *L, lua_Reader reader, void *dt,
                                        const char *chunkname);

LUA_API int (* lua51_dump) (lua_State *L, lua_Writer writer, void *data);

LUA_API int  (* lua51_yield) (lua_State *L, int nresults);
LUA_API int  (* lua51_resume) (lua_State *L, int narg);
LUA_API int  (* lua51_status) (lua_State *L);

LUA_API int (* lua51_gc) (lua_State *L, int what, int data);

LUA_API int   (* lua51_error) (lua_State *L);

LUA_API int   (* lua51_next) (lua_State *L, int idx);

LUA_API void  (* lua51_concat) (lua_State *L, int n);

LUA_API lua_Alloc (* lua51_getallocf) (lua_State *L, void **ud);
LUA_API void (* lua51_setallocf) (lua_State *L, lua_Alloc f, void *ud);

LUA_API int (*lua51_getstack) (lua_State *L, int level, lua_Debug *ar);
LUA_API int (*lua51_getinfo) (lua_State *L, const char *what, lua_Debug *ar);
LUA_API const char (*lua51_getlocal) (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char (*lua51_setlocal) (lua_State *L, const lua_Debug *ar, int n);
LUA_API const char (*lua51_getupvalue) (lua_State *L, int funcindex, int n);
LUA_API const char (*lua51_setupvalue) (lua_State *L, int funcindex, int n);

LUA_API int (*lua51_sethook) (lua_State *L, lua_Hook func, int mask, int count);
LUA_API lua_Hook (*lua51_gethook) (lua_State *L);
LUA_API int (*lua51_gethookmask) (lua_State *L);
LUA_API int (*lua51_gethookcount) (lua_State *L);

LUALIB_API int (* lua51open_base) (lua_State *L);
LUALIB_API int (* lua51open_table) (lua_State *L);
LUALIB_API int (* lua51open_io) (lua_State *L);
LUALIB_API int (* lua51open_os) (lua_State *L);
LUALIB_API int (* lua51open_string) (lua_State *L);
LUALIB_API int (* lua51open_math) (lua_State *L);
LUALIB_API int (* lua51open_debug) (lua_State *L);
LUALIB_API int (* lua51open_package) (lua_State *L);

LUALIB_API void (* lua51L_openlibs) (lua_State *L);

LUALIB_API int (* lua51L_getn) (lua_State *L, int t);
LUALIB_API void (* lua51L_setn) (lua_State *L, int t, int n);

LUALIB_API void (* lua51I_openlib) (lua_State *L, const char *libname,
                                const luaL_Reg *l, int nup);
LUALIB_API void (* lua51L_register) (lua_State *L, const char *libname,
                                const luaL_Reg *l);
LUALIB_API int (* lua51L_getmetafield) (lua_State *L, int obj, const char *e);
LUALIB_API int (* lua51L_callmeta) (lua_State *L, int obj, const char *e);
LUALIB_API int (* lua51L_typerror) (lua_State *L, int narg, const char *tname);
LUALIB_API int (* lua51L_argerror) (lua_State *L, int numarg, const char *extramsg);
LUALIB_API const char *(* lua51L_checklstring) (lua_State *L, int numArg,
                                                          size_t *l);
LUALIB_API const char *(* lua51L_optlstring) (lua_State *L, int numArg,
                                          const char *def, size_t *l);
LUALIB_API lua_Number (* lua51L_checknumber) (lua_State *L, int numArg);
LUALIB_API lua_Number (* lua51L_optnumber) (lua_State *L, int nArg, lua_Number def);

LUALIB_API lua_Integer (* lua51L_checkinteger) (lua_State *L, int numArg);
LUALIB_API lua_Integer (* lua51L_optinteger) (lua_State *L, int nArg,
                                          lua_Integer def);

LUALIB_API void (* lua51L_checkstack) (lua_State *L, int sz, const char *msg);
LUALIB_API void (* lua51L_checktype) (lua_State *L, int narg, int t);
LUALIB_API void (* lua51L_checkany) (lua_State *L, int narg);

LUALIB_API int   (* lua51L_newmetatable) (lua_State *L, const char *tname);
LUALIB_API void *(* lua51L_checkudata) (lua_State *L, int ud, const char *tname);

LUALIB_API void (* lua51L_where) (lua_State *L, int lvl);
LUALIB_API int (* lua51L_error) (lua_State *L, const char *fmt, ...);

LUALIB_API int (* lua51L_checkoption) (lua_State *L, int narg, const char *def,
                                   const char *const lst[]);

LUALIB_API int (* lua51L_ref) (lua_State *L, int t);
LUALIB_API void (* lua51L_unref) (lua_State *L, int t, int ref);

LUALIB_API int (* lua51L_loadfile) (lua_State *L, const char *filename);
LUALIB_API int (* lua51L_loadbuffer) (lua_State *L, const char *buff, size_t sz,
                                  const char *name);
LUALIB_API int (* lua51L_loadstring) (lua_State *L, const char *s);

LUALIB_API lua_State *(* lua51L_newstate) (void);


LUALIB_API const char *(* lua51L_gsub) (lua_State *L, const char *s, const char *p,
                                                  const char *r);

LUALIB_API const char *(* lua51L_findtable) (lua_State *L, int idx,
                                         const char *fname, int szhint);


LUALIB_API void (* lua51L_buffinit) (lua_State *L, luaL_Buffer *B);
LUALIB_API char *(* lua51L_prepbuffer) (luaL_Buffer *B);
LUALIB_API void (* lua51L_addlstring) (luaL_Buffer *B, const char *s, size_t l);
LUALIB_API void (* lua51L_addstring) (luaL_Buffer *B, const char *s);
LUALIB_API void (* lua51L_addvalue) (luaL_Buffer *B);
LUALIB_API void (* lua51L_pushresult) (luaL_Buffer *B);

};

HMODULE g_hLua51Dll;

template <class T>
inline bool Lua51_LoadFunction(T& pDest, const char* sName)
{
	return (pDest = (T)GetProcAddress(g_hLua51Dll, sName)) != 0;
}

RAINMAN_API bool Lua51_Load(const wchar_t* sDll)
{
	g_hLua51Dll = LoadLibraryW(sDll);

	if(!g_hLua51Dll) return false;
	bool bGood = true;

	bGood = bGood && Lua51_LoadFunction(lua51_atpanic, "lua_atpanic");
	bGood = bGood && Lua51_LoadFunction(lua51_call, "lua_call");
	bGood = bGood && Lua51_LoadFunction(lua51_checkstack, "lua_checkstack");
	bGood = bGood && Lua51_LoadFunction(lua51_close, "lua_close");
	bGood = bGood && Lua51_LoadFunction(lua51_concat, "lua_concat");
	bGood = bGood && Lua51_LoadFunction(lua51_cpcall, "lua_cpcall");
	bGood = bGood && Lua51_LoadFunction(lua51_createtable, "lua_createtable");
	bGood = bGood && Lua51_LoadFunction(lua51_dump, "lua_dump");
	bGood = bGood && Lua51_LoadFunction(lua51_equal, "lua_equal");
	bGood = bGood && Lua51_LoadFunction(lua51_error, "lua_error");
	bGood = bGood && Lua51_LoadFunction(lua51_gc, "lua_gc");
	bGood = bGood && Lua51_LoadFunction(lua51_getallocf, "lua_getallocf");
	bGood = bGood && Lua51_LoadFunction(lua51_getfenv, "lua_getfenv");
	bGood = bGood && Lua51_LoadFunction(lua51_getfield, "lua_getfield");
	bGood = bGood && Lua51_LoadFunction(lua51_gethook, "lua_gethook");
	bGood = bGood && Lua51_LoadFunction(lua51_gethookcount, "lua_gethookcount");
	bGood = bGood && Lua51_LoadFunction(lua51_gethookmask, "lua_gethookmask");
	bGood = bGood && Lua51_LoadFunction(lua51_getinfo, "lua_getinfo");
	bGood = bGood && Lua51_LoadFunction(lua51_getlocal, "lua_getlocal");
	bGood = bGood && Lua51_LoadFunction(lua51_getmetatable, "lua_getmetatable");
	bGood = bGood && Lua51_LoadFunction(lua51_getstack, "lua_getstack");
	bGood = bGood && Lua51_LoadFunction(lua51_gettable, "lua_gettable");
	bGood = bGood && Lua51_LoadFunction(lua51_gettop, "lua_gettop");
	bGood = bGood && Lua51_LoadFunction(lua51_getupvalue, "lua_getupvalue");
	bGood = bGood && Lua51_LoadFunction(lua51_insert, "lua_insert");
	bGood = bGood && Lua51_LoadFunction(lua51_iscfunction, "lua_iscfunction");
	bGood = bGood && Lua51_LoadFunction(lua51_isnumber, "lua_isnumber");
	bGood = bGood && Lua51_LoadFunction(lua51_isstring, "lua_isstring");
	bGood = bGood && Lua51_LoadFunction(lua51_isuserdata, "lua_isuserdata");
	bGood = bGood && Lua51_LoadFunction(lua51_lessthan, "lua_lessthan");
	bGood = bGood && Lua51_LoadFunction(lua51_load, "lua_load");
	bGood = bGood && Lua51_LoadFunction(lua51_newstate, "lua_newstate");
	bGood = bGood && Lua51_LoadFunction(lua51_newthread, "lua_newthread");
	bGood = bGood && Lua51_LoadFunction(lua51_newuserdata, "lua_newuserdata");
	bGood = bGood && Lua51_LoadFunction(lua51_next, "lua_next");
	bGood = bGood && Lua51_LoadFunction(lua51_objlen, "lua_objlen");
	bGood = bGood && Lua51_LoadFunction(lua51_pcall, "lua_pcall");
	bGood = bGood && Lua51_LoadFunction(lua51_pushboolean, "lua_pushboolean");
	bGood = bGood && Lua51_LoadFunction(lua51_pushcclosure, "lua_pushcclosure");
	bGood = bGood && Lua51_LoadFunction(lua51_pushfstring, "lua_pushfstring");
	bGood = bGood && Lua51_LoadFunction(lua51_pushinteger, "lua_pushinteger");
	bGood = bGood && Lua51_LoadFunction(lua51_pushlightuserdata, "lua_pushlightuserdata");
	bGood = bGood && Lua51_LoadFunction(lua51_pushlstring, "lua_pushlstring");
	bGood = bGood && Lua51_LoadFunction(lua51_pushnil, "lua_pushnil");
	bGood = bGood && Lua51_LoadFunction(lua51_pushnumber, "lua_pushnumber");
	bGood = bGood && Lua51_LoadFunction(lua51_pushstring, "lua_pushstring");
	bGood = bGood && Lua51_LoadFunction(lua51_pushthread, "lua_pushthread");
	bGood = bGood && Lua51_LoadFunction(lua51_pushvalue, "lua_pushvalue");
	bGood = bGood && Lua51_LoadFunction(lua51_pushvfstring, "lua_pushvfstring");
	bGood = bGood && Lua51_LoadFunction(lua51_rawequal, "lua_rawequal");
	bGood = bGood && Lua51_LoadFunction(lua51_rawget, "lua_rawget");
	bGood = bGood && Lua51_LoadFunction(lua51_rawgeti, "lua_rawgeti");
	bGood = bGood && Lua51_LoadFunction(lua51_rawset, "lua_rawset");
	bGood = bGood && Lua51_LoadFunction(lua51_rawseti, "lua_rawseti");
	bGood = bGood && Lua51_LoadFunction(lua51_remove, "lua_remove");
	bGood = bGood && Lua51_LoadFunction(lua51_replace, "lua_replace");
	bGood = bGood && Lua51_LoadFunction(lua51_resume, "lua_resume");
	bGood = bGood && Lua51_LoadFunction(lua51_setallocf, "lua_setallocf");
	bGood = bGood && Lua51_LoadFunction(lua51_setfenv, "lua_setfenv");
	bGood = bGood && Lua51_LoadFunction(lua51_setfield, "lua_setfield");
	bGood = bGood && Lua51_LoadFunction(lua51_sethook, "lua_sethook");
	bGood = bGood && Lua51_LoadFunction(lua51_setlocal, "lua_setlocal");
	bGood = bGood && Lua51_LoadFunction(lua51_setmetatable, "lua_setmetatable");
	bGood = bGood && Lua51_LoadFunction(lua51_settable, "lua_settable");
	bGood = bGood && Lua51_LoadFunction(lua51_settop, "lua_settop");
	bGood = bGood && Lua51_LoadFunction(lua51_setupvalue, "lua_setupvalue");
	bGood = bGood && Lua51_LoadFunction(lua51_status, "lua_status");
	bGood = bGood && Lua51_LoadFunction(lua51_toboolean, "lua_toboolean");
	bGood = bGood && Lua51_LoadFunction(lua51_tocfunction, "lua_tocfunction");
	bGood = bGood && Lua51_LoadFunction(lua51_tointeger, "lua_tointeger");
	bGood = bGood && Lua51_LoadFunction(lua51_tolstring, "lua_tolstring");
	bGood = bGood && Lua51_LoadFunction(lua51_tonumber, "lua_tonumber");
	bGood = bGood && Lua51_LoadFunction(lua51_topointer, "lua_topointer");
	bGood = bGood && Lua51_LoadFunction(lua51_tothread, "lua_tothread");
	bGood = bGood && Lua51_LoadFunction(lua51_touserdata, "lua_touserdata");
	bGood = bGood && Lua51_LoadFunction(lua51_type, "lua_type");
	bGood = bGood && Lua51_LoadFunction(lua51_typename, "lua_typename");
	bGood = bGood && Lua51_LoadFunction(lua51_xmove, "lua_xmove");
	bGood = bGood && Lua51_LoadFunction(lua51_yield, "lua_yield");
	bGood = bGood && Lua51_LoadFunction(lua51L_addlstring, "luaL_addlstring");
	bGood = bGood && Lua51_LoadFunction(lua51L_addstring, "luaL_addstring");
	bGood = bGood && Lua51_LoadFunction(lua51L_addvalue, "luaL_addvalue");
	bGood = bGood && Lua51_LoadFunction(lua51L_argerror, "luaL_argerror");
	bGood = bGood && Lua51_LoadFunction(lua51L_buffinit, "luaL_buffinit");
	bGood = bGood && Lua51_LoadFunction(lua51L_callmeta, "luaL_callmeta");
	bGood = bGood && Lua51_LoadFunction(lua51L_checkany, "luaL_checkany");
	bGood = bGood && Lua51_LoadFunction(lua51L_checkinteger, "luaL_checkinteger");
	bGood = bGood && Lua51_LoadFunction(lua51L_checklstring, "luaL_checklstring");
	bGood = bGood && Lua51_LoadFunction(lua51L_checknumber, "luaL_checknumber");
	bGood = bGood && Lua51_LoadFunction(lua51L_checkoption, "luaL_checkoption");
	bGood = bGood && Lua51_LoadFunction(lua51L_checkstack, "luaL_checkstack");
	bGood = bGood && Lua51_LoadFunction(lua51L_checktype, "luaL_checktype");
	bGood = bGood && Lua51_LoadFunction(lua51L_checkudata, "luaL_checkudata");
	bGood = bGood && Lua51_LoadFunction(lua51L_error, "luaL_error");
	bGood = bGood && Lua51_LoadFunction(lua51L_findtable, "luaL_findtable");
	bGood = bGood && Lua51_LoadFunction(lua51L_getmetafield, "luaL_getmetafield");
	bGood = bGood && Lua51_LoadFunction(lua51L_gsub, "luaL_gsub");
	bGood = bGood && Lua51_LoadFunction(lua51L_loadbuffer, "luaL_loadbuffer");
	bGood = bGood && Lua51_LoadFunction(lua51L_loadfile, "luaL_loadfile");
	bGood = bGood && Lua51_LoadFunction(lua51L_loadstring, "luaL_loadstring");
	bGood = bGood && Lua51_LoadFunction(lua51L_newmetatable, "luaL_newmetatable");
	bGood = bGood && Lua51_LoadFunction(lua51L_newstate, "luaL_newstate");
	bGood = bGood && Lua51_LoadFunction(lua51L_openlib, "luaL_openlib");
	bGood = bGood && Lua51_LoadFunction(lua51L_openlibs, "luaL_openlibs");
	bGood = bGood && Lua51_LoadFunction(lua51L_optinteger, "luaL_optinteger");
	bGood = bGood && Lua51_LoadFunction(lua51L_optlstring, "luaL_optlstring");
	bGood = bGood && Lua51_LoadFunction(lua51L_optnumber, "luaL_optnumber");
	bGood = bGood && Lua51_LoadFunction(lua51L_prepbuffer, "luaL_prepbuffer");
	bGood = bGood && Lua51_LoadFunction(lua51L_pushresult, "luaL_pushresult");
	bGood = bGood && Lua51_LoadFunction(lua51L_ref, "luaL_ref");
	bGood = bGood && Lua51_LoadFunction(lua51L_register, "luaL_register");
	bGood = bGood && Lua51_LoadFunction(lua51L_typerror, "luaL_typerror");
	bGood = bGood && Lua51_LoadFunction(lua51L_unref, "luaL_unref");
	bGood = bGood && Lua51_LoadFunction(lua51L_where, "luaL_where");
	bGood = bGood && Lua51_LoadFunction(lua51open_base, "luaopen_base");
	bGood = bGood && Lua51_LoadFunction(lua51open_debug, "luaopen_debug");
	bGood = bGood && Lua51_LoadFunction(lua51open_io, "luaopen_io");
	bGood = bGood && Lua51_LoadFunction(lua51open_math, "luaopen_math");
	bGood = bGood && Lua51_LoadFunction(lua51open_os, "luaopen_os");
	bGood = bGood && Lua51_LoadFunction(lua51open_package, "luaopen_package");
	bGood = bGood && Lua51_LoadFunction(lua51open_string, "luaopen_string");
	bGood = bGood && Lua51_LoadFunction(lua51open_table, "luaopen_table");

	return bGood;
}