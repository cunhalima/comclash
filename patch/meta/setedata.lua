sql("UPDATE tab_label SET name='_end', address=0 WHERE address=1075626528")
sql("INSERT INTO tab_label(address,name,type) VALUES(1,'_edata',0)")
sql("UPDATE tab_reloc SET label=(SELECT labid FROM tab_label WHERE name='_edata') WHERE address=268961836")
sql("UPDATE tab_label SET segment=(SELECT segid FROM tab_segment WHERE name='_BSS') WHERE name='_edata'")
sql("UPDATE tab_label SET segment=(SELECT segid FROM tab_segment WHERE name='_BSS') WHERE name='_end'")

-- Fix an AIL relocation to point to its own module
sql("UPDATE tab_reloc SET label=(SELECT labid FROM tab_label WHERE address=" .. ADDR(0x2B9BAC) ..  "),disp=4 WHERE address=" .. ADDR(0x27167F) .. "")
