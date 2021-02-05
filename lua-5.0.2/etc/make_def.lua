dofile"compat.lua"

function read_file(filename)
  local fp = readfrom("../include/"..filename)
  local text = read("*a")
  closefile(fp)
  return text
end

function write_func(f)
  write("  ", f, "\n")
end

lua_h = read_file("lua.h")
--luadebug_h = read_file("luadebug.h")

writeto("_lua.def");
  write("LIBRARY LUA\n")
  write("VERSION ")
    -- #define LUA_VERSION	"Lua 4.0"
    gsub(lua_h, "LUA_VERSION.-(%d+%.%d+)", write)
  write("\n")
  write("EXPORTS\n")
    -- LUA_API lua_State *lua_open (int stacksize);
    gsub(lua_h, "LUA_API.-(lua_%w+)%s+%(", write_func)
--    gsub(luadebug_h, "LUA_API.-(lua_%w+)%s+%(", write_func)
writeto();

lualib_h = read_file("lualib.h")
lauxlib_h = read_file("lauxlib.h")

writeto("_lualib.def");
  write("LIBRARY LUALIB\n")
  write("VERSION ")
    gsub(lua_h, "LUA_VERSION.-(%d+%.%d+)", write)
  write("\n")
  write("EXPORTS\n")
    gsub(lualib_h, "LUALIB_API.-(luaopen_%w+)%s+%(", write_func)
    gsub(lauxlib_h, "LUALIB_API.-(luaL_%w+)%s+%(", write_func)
writeto();

