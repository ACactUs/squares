debug:
	clang src/*.c -lm -lncurses -o out -Wall -Wextra -Wconversion -g -std=gnu99
build:
	clang src/*.c -lm -lncurses -o out -Wall -Wextra -Wconversion -std=gnu99 
