CC=gcc
CFLAGS=-O3 -fPIC #-DDEBUG
DEPS=my_malloc.h

all: lib

lib: my_malloc.o
	$(CC) $(CFLAGS) -shared -o libmymalloc.so my_malloc.o

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $< 

clean:
	rm -f *~ *.o *.so

clobber:
	rm -f *~ *.o
