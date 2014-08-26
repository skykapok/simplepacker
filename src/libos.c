#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lualib.h"
#include "lauxlib.h"

#if defined(WIN32)
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	#include <Shlobj.h>
#elif defined (OSX)
	#include <dirent.h>
	#include <errno.h>
	#include <sys/stat.h>
	#include <sys/types.h>
#endif

static int
lwalkdir(lua_State *L) {
	const char* path = luaL_checkstring(L, -1);
	lua_newtable(L);
	int i = 1;

#if defined(WIN32)
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
#elif defined (OSX)
	DIR *dp = opendir(path);
	struct dirent* dir;
	while ((dir = readdir(dp)) != NULL) {
		lua_pushnumber(L, i);
		lua_pushstring(L, dir->d_name);
		lua_settable(L, -3);
		i += 1;
	}
	closedir(dp);
#endif

	return 1;
}

static int
lmakedir(lua_State *L) {
	const char* path = luaL_checkstring(L, -1);

#if defined(WIN32)
	char buf[MAX_PATH];
	GetFullPathName(path, MAX_PATH, buf, NULL);
	SHCreateDirectoryEx(NULL, buf, NULL);
#elif defined (OSX)
	char* buf = (char*)malloc(sizeof(char) * (strlen(path) + 1));
	strcpy(buf, path);
	char* p = buf;
	while (*p) {
		++p;
		while (*p && *p != '/') ++p;

		char tmp = *p;
		*p = '\0';
		if (mkdir(buf, 0755) == -1 && errno != EEXIST) break;
		*p = tmp;
	}
	free(buf);
#endif

	return 0;
}

static int
lwritefile(lua_State *L) {
	const char* path = luaL_checkstring(L, -2);
	const char* content = luaL_checkstring(L, -1);

	FILE* fp = fopen(path, "w");
	if (!fp) {
		luaL_error(L, "can't write to %s", path);
	}

	fputs(content, fp);
	fclose(fp);

	return 0;
}

int
register_libos(lua_State *L) {
	luaL_Reg l[] = {
		{ "walkdir", lwalkdir },
		{ "makedir", lmakedir },
		{ "writefile", lwritefile },
		{ NULL, NULL }
	};

	luaL_newlib(L, l);
	return 1;
}