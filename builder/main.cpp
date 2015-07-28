#include <cstdio>
#include <cstring>
//#include <sqlite3.h>
#include <libdis.h>
#include <unistd.h>
#include <lua5.2/lua.hpp>
#include "defs.h"

uaddr_t ida_to_addr(int addr) {
    if (addr >= 0x2C6770) {
        return uaddr_mk(4, addr - 0x2C6770);
    }
    if (addr >= 0x2AD000) {
        return uaddr_mk(3, addr - 0x2AD000);
    }
    if (addr >= 0x1DE000) {
        return uaddr_mk(1, addr - 0x1DE000);
    }
    if (addr >= 0x123000) {
        return uaddr_mk(2, addr - 0x123000);
    }
    return 0;
}

int addr_to_ida(uaddr_t addr) {
    if (addr >= uaddr_mk(4, 0)) {
        return addr - uaddr_mk(4, 0) + 0x2C6770;
    }
    if (addr >= uaddr_mk(3, 0)) {
        return addr - uaddr_mk(3, 0) + 0x2AD000;
    }
    if (addr >= uaddr_mk(2, 0)) {
        return addr - uaddr_mk(2, 0) + 0x123000;
    }
    if (addr >= uaddr_mk(1, 0)) {
        return addr - uaddr_mk(1, 0) + 0x1DE000;
    }
    return 0;
}

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
    if (lua_gettop(L) != 7) {
        printf("img_create: wrong number of arguments\n");
        return 0;
    }
    const char *n = luaL_checkstring(L, 1);
    const char *c = luaL_checkstring(L, 2);
    int align = luaL_checkinteger(L, 3);
    int use = luaL_checkinteger(L, 4);
    uaddr_t addr = luaL_checkinteger(L, 5);
    int p = luaL_checkinteger(L, 6);
    int s = luaL_checkinteger(L, 7);
    IMG_createseg(g_img, n, c, align, use, addr, p, s);
    return 0;
}

static int l_begin_transaction(lua_State *L) {
    IMG_begintransaction(g_img);
    return 0;
}

static int l_end_transaction(lua_State *L) {
    IMG_endtransaction(g_img);
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

static int l_plan_convert(lua_State *L) {
    if (lua_gettop(L) != 2) {
        printf("plan_convert: wrong number of arguments\n");
        return 0;
    }
    const char *a = luaL_checkstring(L, 1);
    const char *b = luaL_checkstring(L, 2);
    plan_convert(a, b);
    return 0;
}

static int l_img_loadplan(lua_State *L) {
    if (lua_gettop(L) != 2) {
        printf("plan_convert: wrong number of arguments\n");
        return 0;
    }
    int a = luaL_checkinteger(L, 1);
    const char *b = luaL_checkstring(L, 2);
    IMG_loadplan(g_img, a, b);
    return 0;
}

static int l_img_fixlabels(lua_State *L) {
    if (lua_gettop(L) != 1) {
        printf("plan_convert: wrong number of arguments\n");
        return 0;
    }
    int a = luaL_checkinteger(L, 1);
    IMG_fixlabels(g_img, a);
    return 0;
}

static int l_img_addrlabels(lua_State *L) {
    if (lua_gettop(L) != 1) {
        printf("plan_convert: wrong number of arguments\n");
        return 0;
    }
    int a = luaL_checkinteger(L, 1);
    IMG_addrlabels(g_img, a);
    return 0;
}

static int l_img_setsegments(lua_State *L) {
    IMG_setsegments(g_img);
    return 0;
}

static int l_img_setmodules(lua_State *L) {
    IMG_setmodules(g_img);
    return 0;
}

static int l_img_disasm(lua_State *L) {
    IMG_disasm(g_img);
    return 0;
}

static int l_img_compare(lua_State *L) {
    const char *f = luaL_checkstring(L, 1);
    IMG_loadexe(g_img, f, false, false, false, true);
    return 0;
}

static int l_img_walkplan(lua_State *L) {
    int secid = luaL_checkinteger(L, 1);
    IMG_walkplan(g_img, secid);
    return 0;
}

static int l_NA(lua_State *L) {
    uaddr_t addr = luaL_checkinteger(L, 1);
    const char *name = luaL_checkstring(L, 2);
    IMG_setlabname(g_img, addr, name, LABEL_STATIC);
    return 0;
}

static int l_NP(lua_State *L) {
    uaddr_t addr = luaL_checkinteger(L, 1);
    const char *name = luaL_checkstring(L, 2);
    IMG_setlabname(g_img, addr, name, LABEL_PUBLIC);
    return 0;
}

static int l_img_linklabout(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);
    IMG_linklabout(g_img, name);
    return 0;
}

static int l_img_linkrelout(lua_State *L) {
    uaddr_t addr = luaL_checkinteger(L, 1);
    const char *name = luaL_checkstring(L, 2);
    const char *sname = luaL_checkstring(L, 3);
    IMG_linkrelout(g_img, addr, name, sname);
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <script-file>\n", argv[0]);
        return -1;
    }
    x86_init(opt_none, NULL, NULL);
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
    lua_regfun(L, plan_convert);
    lua_regfun(L, img_loadplan);
    lua_regfun(L, img_fixlabels);
    lua_regfun(L, img_addrlabels);
    lua_regfun(L, img_setmodules);
    lua_regfun(L, img_setsegments);
    lua_regfun(L, img_disasm);
    lua_regfun(L, img_compare);
    lua_regfun(L, img_walkplan);
    lua_regfun(L, begin_transaction);
    lua_regfun(L, end_transaction);
    lua_regfun(L, NA); // set label name (ASSEMBLY -- no underscore)
    lua_regfun(L, NP); // set label name (ASSEMBLY -- no underscore) PUBLIC
    lua_regfun(L, img_linklabout);
    lua_regfun(L, img_linkrelout);
    if (luaL_dofile(L, argv[1])) {
        printf("LUA ERROR: %s\n", lua_tostring(L, -1));
    }
    if (g_img != NULL) {
        IMG_free(g_img);
        g_img = NULL;
    }
    lua_close(L);
    x86_cleanup();
    return 0;
}
