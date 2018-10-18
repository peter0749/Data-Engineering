all: rsort test
rsort: rsort.c msort.o 
	gcc -std=c11 rsort.c msort.o -lm -pthread -O3 -o rsort
msort.o: msort.c msort.h
	gcc -std=c11 msort.c -c -o msort.o -pthread -O2
test:
	gcc -std=c11 test.c -O3 -o test
clean:
	rm -f rsort *.o test
