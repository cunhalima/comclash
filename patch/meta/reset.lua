function uaddr_mk(secid, off)
    return secid * 0x10000000 + off
end

function ADDR(addr)
    if (addr >= 0x10000000) then
        return addr
    end
    if (addr >= 0x2C6770) then
        return uaddr_mk(4, addr - 0x2C6770)
    end
    if (addr >= 0x2AD000) then
        return uaddr_mk(3, addr - 0x2AD000)
    end
    if (addr >= 0x1DE000) then
        return uaddr_mk(1, addr - 0x1DE000)
    end
    if (addr >= 0x123000) then
        return uaddr_mk(2, addr - 0x123000)
    end
    return 0
end

MDIR = "patch/meta/"
print("Loading execfg.lua...")
dofile(MDIR .. "execfg.lua") -- sets ORIGINAL_EXE and BUILD_PATH
print("Opening database...")
--img_open(":memory:")
unlink(MDIR .. "db.db")
img_open(MDIR .. "db.db")
print("Populating database tables...")
sql_file(MDIR .. "tables.sql")
print("Loading original EXE...")
img_loadexe(ORIGINAL_EXE, true, true, true, false)
img_fixrel()
seg_create("BEGTEXT",     "CODE",    16,  32,  0x10000000,  0, 16)
seg_create("_TEXT",       "CODE",     1,  32,  0x10000010,  0, 847103)
seg_create("CODE16",      "CODE",     1,  16,  0x20000000,  0, 403)
seg_create("_NULL",       "BEGDATA",  4,  32,  0x30000000,  0, 4)
seg_create("_AFTERNULL",  "BEGDATA",  2,  32,  0x30000004,  0, 0)
seg_create("CONST",       "DATA",     2,  32,  0x30000004,  1, 7528)
seg_create("_DATA",       "DATA",     4,  32,  0x30001D6C,  9, 0)
seg_create("DATA",        "DATA",     2,  32,  0x30001D6C, 10, 96660)
seg_create("TIB",         "DATA",     1,  32,  ADDR(0x2C6700),  0, 0)
seg_create("TI",          "DATA",     1,  32,  ADDR(0x2C6700),  1, 0)
seg_create("TIE",         "DATA",     1,  32,  ADDR(0x2C6700),  2, 0)
seg_create("XIB",         "DATA",     1,  32,  ADDR(0x2C6700),  3, 0)
seg_create("XI",          "DATA",     1,  32,  ADDR(0x2C6700),  4, 102)
seg_create("XIE",         "DATA",     1,  32,  ADDR(0x2C6766),  5, 0)
seg_create("YIB",         "DATA",     1,  32,  ADDR(0x2C6766),  6, 0)
seg_create("YI",          "DATA",     1,  32,  ADDR(0x2C6766),  7, 6)
seg_create("YIE",         "DATA",     1,  32,  ADDR(0x2C676C),  8, 4)
seg_create("_BSS",        "BSS",      2,  32,  0x40000000,  0, 1884704)
seg_create("STACK",       "STACK",    16, 32,  0x401CC220,  0, 0)
sql("UPDATE tab_label SET name='_end', address=0 WHERE address=1075626528")
sql("INSERT INTO tab_label(address,name) VALUES(1,'_edata')")
sql("UPDATE tab_reloc SET label=(SELECT labid FROM tab_label WHERE name='_edata') WHERE address=268961836")

sql("UPDATE tab_label SET segment=(SELECT segid FROM tab_segment WHERE name='_BSS') WHERE name='_edata'")
sql("UPDATE tab_label SET segment=(SELECT segid FROM tab_segment WHERE name='_BSS') WHERE name='_end'")

-- put names
dofile(MDIR .. "vars.lua") -- set var names

--mod_create("cstart")
--mod_create("key16")
--mod_create("key16")
--[[

slice_create("cstart",          0x10000000)
slice_create("key16",           0x20000000)
slice_create("cstart",          0x30000000)
slice_create("bss",             0x40000000)
slice_create("mingames",        ADDR(0x2106F1))
slice_create("cmiddle",         ADDR(0x215362)) -- after mingames
slice_create("ail00",           ADDR(0x24EC40)) -- AIL stuff

slice_create("mingames",        ADDR(0x2B2910)) -- mingames data
slice_create("cmiddle",         ADDR(0x2B2B0C)) -- after mingames data
slice_create("ail00",           ADDR(0x2B3B28)) -- AIL stuff data
]]--

-- other
--xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

-- code
slice_create("cstart", ADDR(0x1DE000))
slice_create("poco",   ADDR(0x1E59D3))
slice_create("gfacio", ADDR(0x1E7559))
slice_create("galoba", ADDR(0x1E8B83))
slice_create("aaa",    ADDR(0x1EE3A3))
slice_create("bbb",    ADDR(0x1EEBEC))
slice_create("ccc",    ADDR(0x1EEF1B))
slice_create("ddd",    ADDR(0x1EF37E))
slice_create("fff",    ADDR(0x1F0AEF))
slice_create("ggg",    ADDR(0x1F0BB1))
slice_create("hhh",    ADDR(0x1F0C7F))
slice_create("iii",    ADDR(0x1F0ED5))
slice_create("jjj",    ADDR(0x1F139F))
slice_create("kkk",    ADDR(0x1F1C4C))
slice_create("lll",    ADDR(0x1F1E8F))
slice_create("mmm",    ADDR(0x1F2E07))
slice_create("nnn",    ADDR(0x1F7002))
slice_create("ooo",    ADDR(0x1F72C1))
slice_create("ppp",    ADDR(0x1FA061))
slice_create("qqq",    ADDR(0x1FB36F))
slice_create("mingames",        ADDR(0x2106F1))
slice_create("cmiddle",         ADDR(0x215362)) -- after mingames
slice_create("ail00",           ADDR(0x24EC40)) -- AIL stuff

-- code16
slice_create("key16",           0x20000000)

-- data
slice_create("cstart",          0x30000000)
slice_create("mingames",        ADDR(0x2B2910)) -- mingames data
slice_create("cmiddle",         ADDR(0x2B2B0C)) -- after mingames data
slice_create("ail00",           ADDR(0x2B3B28)) -- AIL stuff data

-- bss
slice_create("bss",             0x40000000)

--xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

--[[
--- OLD
slice_create("cstart",          0x10000000)
slice_create("key16",           0x20000000)
slice_create("data",            0x30000000)
slice_create("bss",             0x40000000)
slice_create("mingames",        ADDR(0x2106F1))
slice_create("c215362",         ADDR(0x215362)) -- after mingames
slice_create("mingames",        ADDR(0x2B2910)) -- mingames data
slice_create("d2B2B0C",         ADDR(0x2B2B0C)) -- after mingames data
--- OLD
]]--

--plan_convert(MDIR .. "1plan", MDIR .. "1plan.bin")
--plan_convert(MDIR .. "2plan", MDIR .. "2plan.bin")
--plan_convert(MDIR .. "3plan", MDIR .. "3plan.bin")
img_loadplan(1, MDIR .. "1plan.bin")
img_loadplan(3, MDIR .. "3plan.bin")
img_fixlabels(1)
img_fixlabels(3)
img_addrlabels(1)
img_setsegments()
img_setmodules()
img_disasm()
