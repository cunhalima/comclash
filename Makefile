DOSBOX=dosbox
BUILDER=builder/builder
MDIR=patch/meta/

.PHONY: all exe setup-original setup-mlook run gen aclean builder bclean compare ngen pgen val sql src compare walk test

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

aclean:
	@$(MAKE) --no-print-directory -C src aclean

bclean:
	@$(MAKE) --no-print-directory -C builder clean

ngen: builder
	@./$(BUILDER) $(MDIR)ngen.lua

pgen: builder
	@./$(BUILDER) $(MDIR)pgen.lua

test:
	@./$(BUILDER) $(MDIR)test.lua

val: builder
	@echo "Running valgrind and resetting db"
	@valgrind --tool=memcheck ./$(BUILDER) $(MDIR)ngen.lua

gen: builder
	@./$(BUILDER) $(MDIR)gen.lua

sql:
	sqlite3 $(MDIR)db.db
