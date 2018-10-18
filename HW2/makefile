FLAGS=-Ofast -march=native -flto
all: rsort test
rsort: rsort.c msort.o 
	gcc -std=c11 rsort.c msort.o -lm -pthread -o rsort ${FLAGS}
msort.o: msort.c msort.h
	gcc -std=c11 msort.c -c -o msort.o -pthread ${FLAGS}
test:
	gcc -std=c11 test.c -o test ${FLAGS}
clean:
	rm -f rsort *.o test
