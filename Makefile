CC = clang
CFLAGS = -Wall -Wextra -Wconversion -std=gnu99
LFLAGS = -lm -lncurses
DBGFLAGS = -ggdb -g3

default: debug

debug: CFLAGS += $(DBGFLAGS)
debug: clean squares

build: clean squares

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
