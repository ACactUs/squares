debug:
	clang src/*.c -lm -lncurses -o out -Wall -Wextra -Wconversion -g -std=gnu99 -D_POSIX_C_SOURCE=199309L
build:
	clang src/*.c -lm -lncurses -o out -Wall -Wextra -Wconversion -std=gnu99 -D_POSIX_C_SOURCE=199309L
