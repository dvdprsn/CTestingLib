all: A1SampleMain

utest: utest.c A1Header.h
	gcc -Wall -g -std=c11 -fpic -c utest.c
	gcc -Wall -g -std=c11 -shared -o libutest.so utest.o

A1SampleMain: A1SampleMain.c libutest.so A1Header.h
	gcc -Wall -g -std=c11 A1SampleMain.c -o main.o -c
	gcc -Wall -g -std=c11 main.o -lutest -L.

libutest.so: utest

clean: 
	rm -rf *.o *.so *.txt *.out