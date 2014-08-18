#include "lualib.h"
#include "lauxlib.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

static int
lwalkdir(lua_State *L) {
	const char* folder = lua_tostring(L, -1);
	lua_newtable(L);
	int i = 1;

#ifdef WIN32
	char szPath[MAX_PATH];
	sprintf(szPath, "%s\\*", folder);

	WIN32_FIND_DATA findData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind = FindFirstFile(szPath, &findData);
	if (hFind != INVALID_HANDLE_VALUE) {
		while (1) {
			if (findData.cFileName[0] != '.') {
				lua_pushnumber(L, i);
				lua_pushstring(L, findData.cFileName);
				lua_settable(L, -3);
				i += 1;
			}

			if (!FindNextFile(hFind, &findData))
				break;
		}
		FindClose(hFind);
	}
#endif

	return 1;
}

int
register_libos(lua_State *L) {
	luaL_Reg l[] = {
		{ "walkdir", lwalkdir },
		{ NULL, NULL }
	};

	luaL_newlib(L, l);
	return 1;
}