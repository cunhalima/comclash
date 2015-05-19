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
seg_create("TIB",         "DATA",     1,  32,  0x30001D6C,  0, 0)
seg_create("TI",          "DATA",     1,  32,  0x30001D6C,  1, 0)
seg_create("TIE",         "DATA",     1,  32,  0x30001D6C,  2, 0)
seg_create("XIB",         "DATA",     1,  32,  0x30001D6C,  3, 0)
seg_create("XI",          "DATA",     1,  32,  0x30001D6C,  4, 0)
seg_create("XIE",         "DATA",     1,  32,  0x30001D6C,  5, 0)
seg_create("YIB",         "DATA",     1,  32,  0x30001D6C,  6, 0)
seg_create("YI",          "DATA",     1,  32,  0x30001D6C,  7, 0)
seg_create("YIE",         "DATA",     1,  32,  0x30001D6C,  8, 0)
seg_create("_DATA",       "DATA",     4,  32,  0x30001D6C,  9, 0)
seg_create("DATA",        "DATA",     2,  32,  0x30001D6C, 10, 96772)
seg_create("_BSS",        "BSS",      2,  32,  0x40000000,  0, 1884704)
seg_create("STACK",       "STACK",    16, 32,  0x401CC220,  0, 0)
sql("UPDATE tab_label SET name='_end', address=0 WHERE address=1075626528")
sql("INSERT INTO tab_label(address,name) VALUES(1,'_edata')")
sql("UPDATE tab_reloc SET label=(SELECT labid FROM tab_label WHERE name='_edata') WHERE address=268961836")

sql("UPDATE tab_label SET segment=(SELECT segid FROM tab_segment WHERE name='_BSS') WHERE name='_edata'")
sql("UPDATE tab_label SET segment=(SELECT segid FROM tab_segment WHERE name='_BSS') WHERE name='_end'")

--mod_create("cstart")
--mod_create("key16")
--mod_create("key16")
slice_create("cstart",          0x10000000)
slice_create("key16",           0x20000000)
slice_create("data",            0x30000000)
slice_create("bss",             0x40000000)

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
