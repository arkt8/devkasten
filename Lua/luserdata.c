#include <stdio.h>
#include <lua5.2/lua.h>
#include <lua5.2/lauxlib.h>

/*
** THISFILE.h
*/
int udata(lua_State *L);
int luaopen_luserdata(lua_State *L);


/*
** THISFILE.c
*/

int udata(lua_State *L) {
  return 0;
}

static const luaL_Reg export[] = {
  {"udata", udata},
  {NULL,    NULL }
};





int luaopen_luserdata(lua_State *L) {
  luaL_newlib(L, export);
  return 1;
}

