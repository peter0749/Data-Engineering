FLAGS=-Ofast -march=native 
all: rsort 
rsort: rsort.c msort.o test
	gcc -std=c11 rsort.c msort.o -lm -pthread -o .rsort.undone ${FLAGS} -fprofile-generate
	./test 2000000 | ./.rsort.undone .test_pf_file -m 2 -j 3 > /dev/null
	./test 2000000 | ./.rsort.undone .test_pf_file -m 2 -j -n 3 > /dev/null
	gcc -std=c11 rsort.c msort.o -lm -pthread -o rsort ${FLAGS} -fwhole-program -fprofile-use
	rm -f .rsort.undone rsort.gcda
msort.o: msort.c msort.h
	gcc -std=c11 msort.c -c -o msort.o -pthread ${FLAGS}
test:
	gcc -std=c11 test.c -o test ${FLAGS} -fwhole-program
clean:
	rm -f rsort *.o test
