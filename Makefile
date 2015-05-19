DOSBOX=dosbox
BUILDER=builder/builder
MDIR=patch/meta/

.PHONY: all exe setup-original setup-mlook run gen gclean builder bclean compare reset val sql src compare walk

all: exe

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
	@$(DOSBOX) -conf $(MDIR)dosbox.conf

compare: builder exe
	@./$(BUILDER) $(MDIR)compare.lua

walk: builder
	@./$(BUILDER) $(MDIR)walk.lua

gclean:
	@rm -rf src
	@mkdir src
	@echo "Don't edit files here: they will be overwritten" > src/README

bclean:
	@$(MAKE) --no-print-directory -C builder clean

reset: builder
	@echo "Resetting db"
	@./$(BUILDER) $(MDIR)reset.lua

val: builder
	@echo "Running valgrind and resetting db"
	@valgrind --tool=memcheck ./$(BUILDER) $(MDIR)reset.lua

gen: builder
	@./$(BUILDER) $(MDIR)gen.lua

sql:
	sqlite3 $(MDIR)db.db
