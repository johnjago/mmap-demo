CC = gcc
ECHO = echo
RM = rm -f

CFLAGS = -Wall -Werror -ggdb -funroll-loops

all: alloc prov-rep

alloc: alloc.c
	@$(ECHO) Compiling alloc
	@$(CC) $(CFLAGS) -o alloc.out alloc.c

prov-rep: prov-rep.c
	@$(ECHO) Compiling prov-rep
	@$(CC) $(CFLAGS) -o prov-rep.out prov-rep.c

.PHONY: all clean

clean:
	@$(ECHO) Removing all generated files
	@$(RM) *.o *.out *.d TAGS core vgcore.* gmon.out
