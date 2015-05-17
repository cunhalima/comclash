AR=ar
CC=gcc
CFLAGS=-O2 -Wall -Werror -pedantic -Wno-unused-but-set-variable -Wno-parentheses -std=c99
TARGET=libdisasm.a
OBJS = \
	ia32_implicit.o \
	ia32_insn.o \
	ia32_invariant.o \
	ia32_modrm.o \
	ia32_opcode_tables.o \
	ia32_operand.o \
	ia32_reg.o \
	ia32_settings.o \
	x86_disasm.o \
	x86_format.o \
	x86_imm.o \
	x86_insn.o \
	x86_misc.o \
	x86_operand_list.o

.PHONY: all clean

all: $(TARGET)

clean:
	@rm -f $(OBJS) $(TARGET)

.c.o:
	@$(CC) -c $(CFLAGS) $^ -o $@

$(TARGET): $(OBJS)
	@$(AR) rcs $@ $^
