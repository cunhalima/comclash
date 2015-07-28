--[[

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

slice_create("vmouse",          ADDR(0x244FA0)) -- vmouse code
slice_create("dmiddle",         ADDR(0x245092)) -- after vmouse

slice_create("ail00",           ADDR(0x24EC40)) -- AIL stuff

-- code16
slice_create("key16",           0x20000000)

-- data
slice_create("cstart",          0x30000000)
slice_create("mingames",        ADDR(0x2B2910)) -- mingames data
slice_create("cmiddle",         ADDR(0x2B2B0C)) -- after mingames data

slice_create("vmouse",          ADDR(0x2B38A4)) -- vmouse data
slice_create("dmiddle",         ADDR(0x2B38AC)) -- after vmouse data

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
