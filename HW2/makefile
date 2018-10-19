FLAGS=-O3 -march=native 
all: rsort 
rsort: rsort.c msort.o test judge.sh self_test.sh
	gcc -std=c11 rsort.c msort.o -lm -pthread -o rsort ${FLAGS} -fwhole-program 
	./self_test.sh 5 300000
msort.o: msort.c msort.h
	gcc -std=c11 msort.c -c -o msort.o ${FLAGS}
test:
	gcc -std=c11 test.c -o test ${FLAGS} -fwhole-program
clean:
	rm -f rsort *.o test

