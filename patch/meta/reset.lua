-- ##############################################################
-- # This script is used to disassemble Complex Clash
-- #------------------------------------------------------------
-- # By Alex Reimann Cunha Lima, 2015
-- #
-- ##############################################################
dofile(MDIR .. "common.lua") -- IDA addressing stuff
dofile(MDIR .. "execfg.lua") -- sets ORIGINAL_EXE and BUILD_PATH
dofile(MDIR .. "filedb.lua") -- load database from file
dofile(MDIR .. "tables.lua")
print("[DONE]")
io.write("Loading original EXE ................................ ")
img_loadexe(ORIGINAL_EXE, true, true, true, false)
print("[DONE]")
io.write("Fixing labels over relocations ...................... ")
img_fixrel()
print("[DONE]")
io.write("Creating segments ................................... ")
dofile(MDIR .. "segments.lua")
print("[DONE]")
io.write("Setting labels '_end' and '_edata' .................. ")
dofile(MDIR .. "setedata.lua")
print("[DONE]")
io.write("Creating modules and slices ......................... ")
--dofile(MDIR .. "modules.lua")
dofile(MDIR .. "modmap.lua")
print("[DONE]")
io.write("Loading plans ....................................... ")
img_loadplan(1, MDIR .. "1plan.bin")
img_loadplan(3, MDIR .. "3plan.bin")
print("[DONE]")
io.write("Fixing labels over code and data sequences .......... ")
img_fixlabels(1)
img_fixlabels(3)
print("[DONE]")
io.write("Adding jump/call relocations and labels ............. ")
img_addrlabels(1)
print("[DONE]")
-- now that we have all the labels: give'em their proper names
io.write("Setting names for the labels ........................ ")
--dofile(MDIR .. "vars.lua") -- set var names
dofile(MDIR .. "varmap.lua") -- set var names
print("[DONE]")
io.write("Setting the segment for each label and relocation ... ")
img_setsegments()
print("[DONE]")
io.write("Setting the module for each label and relocation .... ")
img_setmodules()
print("[DONE]")
