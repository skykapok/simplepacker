#include "lualib.h"
#include "lauxlib.h"

int register_libimage(lua_State *L);
int register_libos(lua_State *L);

int main(int argc, char** argv) {
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	luaL_requiref(L, "libimage", register_libimage, 0);
	luaL_requiref(L, "libos", register_libos, 0);

	if (luaL_dofile(L, "main.lua") != LUA_OK) {
		printf("====== load script failed ======\n%s\n", lua_tostring(L, -1));
		goto ERR;
	}

	lua_getglobal(L, "run");
	if (!lua_isfunction(L, -1)) {
		printf("====== cannot find script entry function ======\n");
		goto ERR;
	}

	lua_newtable(L);
	for (int i=0; i<argc; ++i) {
		lua_pushnumber(L, i+1);
		lua_pushstring(L, argv[i]);
		lua_settable(L, -3);
	}
	if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
		printf("====== script error ======\n%s\n", lua_tostring(L, -1));
		goto ERR;
	}

ERR:
	lua_close(L);
	return 0;
}