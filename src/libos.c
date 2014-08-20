#include "lualib.h"
#include "lauxlib.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlobj.h>
#endif

static int
lwalkdir(lua_State *L) {
	const char* path = luaL_checkstring(L, -1);
	lua_newtable(L);
	int i = 1;

#ifdef WIN32
	char szPath[MAX_PATH];
	sprintf(szPath, "%s\\*", path);

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

static int
lmakedir(lua_State *L) {
	const char* path = luaL_checkstring(L, -1);

#ifdef WIN32
	char buf[MAX_PATH];
	GetFullPathName(path, MAX_PATH, buf, NULL);
	SHCreateDirectoryEx(NULL, buf, NULL);
#endif

	return 0;
}

int
register_libos(lua_State *L) {
	luaL_Reg l[] = {
		{ "walkdir", lwalkdir },
		{ "makedir", lmakedir },
		{ NULL, NULL }
	};

	luaL_newlib(L, l);
	return 1;
}