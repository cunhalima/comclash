MDIR = "patch/meta/"
print("Loading execfg.lua...")
dofile(MDIR .. "execfg.lua") -- sets ORIGINAL_EXE and BUILD_PATH
print("Opening database...")
img_open(":memory:")
print("Populating database tables...")
sql_file(MDIR .. "tables.sql")
print("Loading original EXE...")
img_loadexe(ORIGINAL_EXE, true, true, true, false)
print("Loading plan for section 1...")
img_loadplan(1, MDIR .. "1plan.bin")
print("Walking image plan...")
img_walkplan(1)
