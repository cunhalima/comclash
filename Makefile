DOSBOX=dosbox
BUILDER=builder/builder

.PHONY: all exe setup-original setup-mlook run gen gclean builder bclean compare reset val sql

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

reset: builder
	@echo "Resetting db"
	@./$(BUILDER) patch/reset.lua

val: builder
	@echo "Running valgrind and resetting db"
	@valgrind --tool=memcheck ./$(BUILDER) patch/reset.lua

gen: builder
	@./$(BUILDER) patch/gen.lua

sql:
	sqlite3 patch/db.db
