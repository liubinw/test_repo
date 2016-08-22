

all: test1


LDFLAGS=-lSDL


test1:
	gcc -o test1 test1.c -lSDL


clean:
	rm -f *.o test1
