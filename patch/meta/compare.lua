-- ##############################################################
-- # This script is used to compare a rebuilding of
-- # Complex Clash to its original EXE
-- #------------------------------------------------------------
-- # By Alex Reimann Cunha Lima, 2015
-- #
-- ##############################################################
MDIR = "patch/meta/"
io.output():setvbuf("no")
io.write("Initializing database ............................... ")
dofile(MDIR .. "common.lua") -- IDA addressing stuff
dofile(MDIR .. "execfg.lua") -- sets ORIGINAL_EXE and BUILD_PATH
dofile(MDIR .. "memdb.lua")    -- use a memory database
dofile(MDIR .. "tables.lua")
print("[DONE]")
io.write("Loading original EXE ................................ ")
img_loadexe(ORIGINAL_EXE, true, true, true, false)
print("[DONE]")
io.write("Loading plans ....................................... ")
img_loadplan(1, MDIR .. "1plan.bin")
print("[DONE]")
print("Comparing images:")
img_compare(BUILD_PATH .. "/ncc.exe")
