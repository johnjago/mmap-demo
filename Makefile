CC = gcc
ECHO = echo
RM = rm -f

CFLAGS = -Wall -Werror -ggdb -funroll-loops

all: demo

alloc: demo.c
	@$(ECHO) Compiling demo
	@$(CC) $(CFLAGS) -o demo.out demo.c

.PHONY: all clean

clean:
	@$(ECHO) Removing all generated files
	@$(RM) *.o *.out *.d TAGS core vgcore.* gmon.out
