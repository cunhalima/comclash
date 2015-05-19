CPP=g++
CPPFLAGS=-std=c++11 -O2 -Wall -Werror -pedantic -I./libdisasm -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function
LDFLAGS=-L./libdisasm -ldisasm -lsqlite3 -llua5.2 -lncurses
OBJS=main.o buffer.o image.o leload.o sqlq.o plan.o disasm.o compare.o walk.o
TARGET=builder
LIBDIS=libdisasm/libdisasm.a

.PHONY: all clean run val

all: $(TARGET)

clean:
	@rm -f $(OBJS) $(TARGET)

$(LIBDIS):
	@$(MAKE) --no-print-directory -C libdisasm

run: $(TARGET)
	@./$(TARGET)

val: $(TARGET)
	@valgrind --tool=memcheck ./$(TARGET)

sql:
	@sqlite3 data/cdshock.db

.cpp.o:
	@$(CPP) -c $(CPPFLAGS) $^ -o $@

$(TARGET): $(OBJS) $(LIBDIS)
	@$(CPP) $(CPPFLAGS) $(OBJS) $(LDFLAGS) -o $@
