CC = gcc
CFLAGS = -Wall -Wextra -Wconversion -std=gnu99
LFLAGS = -lm -lncurses -ltinfo
BUILDFLAGS = -Os
DBGFLAGS = -ggdb -g3 -O0

default: debug

debug: CFLAGS += $(DBGFLAGS)
debug: clean squares

build: CFLAGS += $(BUILDFLAGS)
build: clean squares

profile: CFLAGS += -pg
profile: clean build

squares: logic.o render.o main.o
	$(CC) $(CFLAGS) $(LFLAGS) -o out $^

logic.o: src/logic.c src/logic.h
	$(CC) $(CFLAGS) -c $<

render.o: src/render.c src/render.h src/logic.h
	$(CC) $(CFLAGS) -c $<

main.o: src/main.c src/render.h src/logic.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm *.o -f
