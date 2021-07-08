COMPILER = gcc

CFLAGS := -lm -std=c11 -march=native -Wall -Wextra -Wdouble-promotion -Wpedantic -Wstrict-prototypes -Wshadow -O3

test:
	$(COMPILER) *.c ../src/*.c -o test_ $(CFLAGS)
clean:
	rm -rf test_
