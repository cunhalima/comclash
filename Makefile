DOSBOX=dosbox
BUILDER=builder/builder

.PHONY: all exe setup-original setup-mlook run gen gclean builder bclean compare

all: $(EXE)

setup-original:
	@echo "%define TARGET_ORIGINAL" > src/target.inc

setup-mlook:
	@echo "%define TARGET_MLOOK" > src/target.inc

exe: src

builder:
	@$(MAKE) --no-print-directory -C $@

src:
	@$(MAKE) --no-print-directory -C $@

run: exe
	@$(DOSBOX) -conf dosbox.conf

gclean:
	@rm -rf src
	@mkdir src
	@echo "Don't edit files here: they will be overwritten" > src/README

bclean:
	@$(MAKE) --no-print-directory -C builder clean

restart-re: builder
	@./$(BUILDER) patch/restart-re.lua

gen: builder
	@./$(BUILDER) patch/gen.lua
