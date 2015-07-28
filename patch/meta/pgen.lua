-- ##############################################################
-- # This script is used to disassemble Complex Clash
-- #------------------------------------------------------------
-- # By Alex Reimann Cunha Lima, 2015
-- #
-- ##############################################################
MDIR = "patch/meta/"
io.output():setvbuf("no")
dofile(MDIR .. "reset.lua")
io.write("Applying patch ...................................... ")
dofile(MDIR .. "patch.lua")
print("[DONE]")
io.write("Disassembling ....................................... ")
img_disasm()
print("[DONE]")
