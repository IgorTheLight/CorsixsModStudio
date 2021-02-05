extern "C" {
/*
** $Id: lualib.h,v 1.36 2005/12/27 17:12:00 roberto Exp $
** Lua standard libraries
** See Copyright Notice in lua.h
*/


#ifndef lualib_h
#define lualib_h


/* Key to file-handle type */
#define LUA_FILEHANDLE		"FILE*"


#define LUA_COLIBNAME	"coroutine"
LUALIB_API int (* lua51open_base) (lua_State *L);

#define LUA_TABLIBNAME	"table"
LUALIB_API int (* lua51open_table) (lua_State *L);

#define LUA_IOLIBNAME	"io"
LUALIB_API int (* lua51open_io) (lua_State *L);

#define LUA_OSLIBNAME	"os"
LUALIB_API int (* lua51open_os) (lua_State *L);

#define LUA_STRLIBNAME	"string"
LUALIB_API int (* lua51open_string) (lua_State *L);

#define LUA_MATHLIBNAME	"math"
LUALIB_API int (* lua51open_math) (lua_State *L);

#define LUA_DBLIBNAME	"debug"
LUALIB_API int (* lua51open_debug) (lua_State *L);

#define LUA_LOADLIBNAME	"package"
LUALIB_API int (* lua51open_package) (lua_State *L);


/* open all previous libraries */
LUALIB_API void (* lua51L_openlibs) (lua_State *L); 



#ifndef lua51_assert
#define lua51_assert(x)	((void)0)
#endif


#endif
/*
** $Id: lauxlib.h,v 1.88 2006/04/12 20:31:15 roberto Exp $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#ifndef lauxlib_h
#define lauxlib_h


#include <stddef.h>
#include <stdio.h>

#if defined(LUA_COMPAT_GETN)
LUALIB_API int (* lua51L_getn) (lua_State *L, int t);
LUALIB_API void (* lua51L_setn) (lua_State *L, int t, int n);
#else
#define lua51L_getn(L,i)          ((int)lua51_objlen(L, i))
#define lua51L_setn(L,i,j)        ((void)0)  /* no op! */
#endif

#if defined(LUA_COMPAT_OPENLIB)
#define lua51I_openlib	lua51L_openlib
#endif


/* extra error code for `luaL_load' */
#define LUA_ERRFILE     (LUA_ERRERR+1)


typedef struct luaL_Reg {
  const char *name;
  lua_CFunction func;
} luaL_Reg;



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




/*
** ===============================================================
** some useful macros
** ===============================================================
*/

#define lua51L_argcheck(L, cond,numarg,extramsg)	\
		((void)((cond) || lua51L_argerror(L, (numarg), (extramsg))))
#define lua51L_checkstring(L,n)	(lua51L_checklstring(L, (n), NULL))
#define lua51L_optstring(L,n,d)	(lua51L_optlstring(L, (n), (d), NULL))
#define lua51L_checkint(L,n)	((int)lua51L_checkinteger(L, (n)))
#define lua51L_optint(L,n,d)	((int)lua51L_optinteger(L, (n), (d)))
#define lua51L_checklong(L,n)	((long)lua51L_checkinteger(L, (n)))
#define lua51L_optlong(L,n,d)	((long)lua51L_optinteger(L, (n), (d)))

#define lua51L_typename(L,i)	lua51_typename(L, lua51_type(L,(i)))

#define lua51L_dofile(L, fn) \
	(lua51L_loadfile(L, fn) || lua51_pcall(L, 0, LUA_MULTRET, 0))

#define lua51L_dostring(L, s) \
	(lua51L_loadstring(L, s) || lua51_pcall(L, 0, LUA_MULTRET, 0))

#define lua51L_getmetatable(L,n)	(lua51_getfield(L, LUA_REGISTRYINDEX, (n)))

#define lua51L_opt(L,f,n,d)	(lua51_isnoneornil(L,(n)) ? (d) : f(L,(n)))

/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/



typedef struct luaL_Buffer {
  char *p;			/* current position in buffer */
  int lvl;  /* number of strings in the stack (level) */
  lua_State *L;
  char buffer[LUAL_BUFFERSIZE];
} luaL_Buffer;

#define lua51L_addchar(B,c) \
  ((void)((B)->p < ((B)->buffer+LUAL_BUFFERSIZE) || lua51L_prepbuffer(B)), \
   (*(B)->p++ = (char)(c)))

/* compatibility only */
#define lua51L_putchar(B,c)	lua51L_addchar(B,c)

#define lua51L_addsize(B,n)	((B)->p += (n))

LUALIB_API void (* lua51L_buffinit) (lua_State *L, luaL_Buffer *B);
LUALIB_API char *(* lua51L_prepbuffer) (luaL_Buffer *B);
LUALIB_API void (* lua51L_addlstring) (luaL_Buffer *B, const char *s, size_t l);
LUALIB_API void (* lua51L_addstring) (luaL_Buffer *B, const char *s);
LUALIB_API void (* lua51L_addvalue) (luaL_Buffer *B);
LUALIB_API void (* lua51L_pushresult) (luaL_Buffer *B);


/* }====================================================== */


/* compatibility with ref system */

/* pre-defined references */
#define LUA_NOREF       (-2)
#define LUA_REFNIL      (-1)

#define lua51_ref(L,lock) ((lock) ? lua51L_ref(L, LUA_REGISTRYINDEX) : \
      (lua51_pushstring(L, "unlocked references are obsolete"), lua51_error(L), 0))

#define lua51_unref(L,ref)        lua51L_unref(L, LUA_REGISTRYINDEX, (ref))

#define lua51_getref(L,ref)       lua51_rawgeti(L, LUA_REGISTRYINDEX, (ref))


#define luaL51_reg	lua51L_Reg

#endif

};
