COMPILER = gcc

CFLAGS:= -lm -std=c11 -march=native -Wall -Wextra -Wdouble-promotion -Wpedantic -Wstrict-prototypes -Wshadow -O2

test:
	$(COMPILER) *.c -o test_ $(CFLAGS)
clean:
	rm -rf test_
