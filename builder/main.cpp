#include <cstdio>
#include <cstring>
//#include <sqlite3.h>
#include <unistd.h>
#include <lua5.2/lua.hpp>
#include "defs.h"

static lua_State *L;
static Image_t *g_img;

#define lua_regfun(L, n) \
    lua_register(L, #n, l_ ## n)

static int l_img_open(lua_State *L) {
    const char *s = luaL_checkstring(L, 1);
    g_img = IMG_create();
    if (!IMG_dbopen(g_img, s)) {
        printf("error in database\n");
    }
    return 0;
}

static int l_sql(lua_State *L) {
    const char *s = luaL_checkstring(L, 1);
    IMG_sql(g_img, s);
    return 0;
}

static int l_sql_file(lua_State *L) {
    const char *s = luaL_checkstring(L, 1);
    IMG_sqlfile(g_img, s);
    return 0;
}

static int l_img_loadexe(lua_State *L) {
    if (lua_gettop(L) != 5) {
        printf("img_loadexe: wrong number of arguments\n");
        return 0;
    }
    const char *f = luaL_checkstring(L, 1);
    bool addrelocs = lua_toboolean(L, 2);
    bool createsections = lua_toboolean(L, 3);
    bool loaddata = lua_toboolean(L, 4);
    bool compare = lua_toboolean(L, 5);
    IMG_loadexe(g_img, f, addrelocs, createsections, loaddata, compare);
    return 0;
}

static int l_unlink(lua_State *L) {
    const char *f = luaL_checkstring(L, 1);
    unlink(f);
    return 0;
}

static int l_img_fixrel(lua_State *L) {
    IMG_fixrelocations(g_img);
    return 0;
}

static int l_seg_create(lua_State *L) {
    if (lua_gettop(L) != 6) {
        printf("img_create: wrong number of arguments\n");
        return 0;
    }
    const char *n = luaL_checkstring(L, 1);
    const char *c = luaL_checkstring(L, 2);
    int align = luaL_checkinteger(L, 3);
    int use = luaL_checkinteger(L, 4);
    uaddr_t addr = luaL_checkinteger(L, 5);
    int p = luaL_checkinteger(L, 6);
    IMG_createseg(g_img, n, c, align, use, addr, p);
    return 0;
}

static int l_slice_create(lua_State *L) {
    if (lua_gettop(L) != 2) {
        printf("slice_create: wrong number of arguments\n");
        return 0;
    }
    const char *n = luaL_checkstring(L, 1);
    uaddr_t addr = luaL_checkinteger(L, 2);
    IMG_createslice(g_img, n, addr);
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <script-file>\n", argv[0]);
        return -1;
    }
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    lua_regfun(L, unlink);
    lua_regfun(L, img_open);
    lua_regfun(L, img_loadexe);
    lua_regfun(L, sql);
    lua_regfun(L, sql_file);
    lua_regfun(L, img_fixrel);
    lua_regfun(L, seg_create);
    lua_regfun(L, slice_create);
    if (luaL_dofile(L, argv[1])) {
        printf("LUA ERROR: %s\n", lua_tostring(L, -1));
    }
    if (g_img != NULL) {
        IMG_free(g_img);
        g_img = NULL;
    }
    lua_close(L);
    return 0;
}
