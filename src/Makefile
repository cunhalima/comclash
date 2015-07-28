WCC=../bin/wcc386
WLINK=../bin/wlink
NASM=nasm
CFLAGS=-j -orhtr -s -4r -bt=dos4g -q -we -i=/home/alex/ow/h
LDFLAGS=option nodefaultlibs option dosseg option map option quiet system dos4g
EXE=../links/ccpath/ncc.exe
#OBJ=main.obj
OBJ=
SSOBJ=aa.obj  \
    mingames.obj  \
    bb.obj  \
    canvas.obj  \
    cc.obj  \
    cursors.obj  \
    dd.obj  \
    vmouse.obj  \
    ee.obj  \
    event.obj  \
    ee1.obj  \
    bitmap.obj  \
    ff.obj  \
    ail_a.obj  \
    gg.obj  \
    ail_b.obj  \
    hh.obj  \
    ail_c.obj  \
    ii.obj  \
    ail_d.obj  \
    jj.obj  \
    ail_e.obj  \
    kk.obj  \
    key16.obj  \
    stack.obj
SSASM=$(patsubst %.obj,%.asm,$(SSOBJ))
.PHONY: all clean aclean

all: $(EXE)

clean:
	@rm -f $(SSOBJ) $(OBJ) $(EXE)

aclean: clean
	@rm -f $(SSASM) segs.inc

$(EXE): $(OBJ) $(SSOBJ)
	@$(WLINK) $(LDFLAGS) name $@ file { $(SSOBJ) $(OBJ) }

%.obj: %.asm
	@$(NASM) -O0 -f obj $<

%.obj: %.c
	@$(WCC) $(CFLAGS) $< -fo=$@
