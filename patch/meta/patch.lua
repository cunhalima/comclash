-- tira o main de la e deixa externo
--sql("UPDATE tab_label SET address=NULL,module=NULL WHERE address=" .. ADDR(0x20C248))

--sql("INSERT INTO tab_label(name, type, module, segment) VALUES('gmain_', 0, NULL,(SELECT segid FROM tab_segment WHERE name='_TEXT'))")
--sql("UPDATE tab_reloc SET label=(SELECT labid FROM tab_label WHERE name='gmain_') WHERE address=" .. ADDR(0x2789C7))
img_linkrelout(ADDR(0x2789C7), "gmain_", '_TEXT')


img_linkrelout(ADDR(0x2B2AE5), "games_handle_cobra_", '_TEXT') -- handle
img_linkrelout(ADDR(0x2B2914), "games_expose_cobra_", '_TEXT') -- expose
img_linkrelout(ADDR(0x2B2938), "games_init_cobra_", '_TEXT') -- init

NP(ADDR(0x478BD8), "_grd_canvas")

--[[

sql("UPDATE tab_label SET name='_end', address=0 WHERE address=1075626528")
sql("INSERT INTO tab_label(address,name) VALUES(1,'_edata')")
sql("UPDATE tab_reloc SET label=(SELECT labid FROM tab_label WHERE name='_edata') WHERE address=268961836")
sql("UPDATE tab_label SET segment=(SELECT segid FROM tab_segment WHERE name='_BSS') WHERE name='_edata'")
sql("UPDATE tab_label SET segment=(SELECT segid FROM tab_segment WHERE name='_BSS') WHERE name='_end'")

-- Fix an AIL relocation to point to its own module
sql("UPDATE tab_reloc SET label=(SELECT labid FROM tab_label WHERE address=" .. ADDR(0x2B9BAC) ..  "),disp=4 WHERE address=" .. ADDR(0x27167F) .. "")
]]--
