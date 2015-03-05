CC=gcc
AR=gccar
CFLAGS= -std=c99 -O2 -Wall -D DATE="\"`date`\"" -D GITCOMMIT="\"`git rev-parse HEAD`\"" -static
EXEC=doorsend
OBJS=doorsend.o common.o temp.o

INSPATH=../doorarc-sen-bin/

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS) -lm

install:	all
	mkdir -p $(INSPATH) && cp $(EXEC) $(INSPATH)

clean:
	rm -f $(EXEC) $(OBJS) *.elf *.o *.gdb *.gcda


