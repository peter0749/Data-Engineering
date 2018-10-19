FLAGS=-O3 -march=native 
all: rsort 
rsort: rsort.c test judge.sh self_test.sh
	gcc -std=c11 rsort.c -o rsort ${FLAGS} -fwhole-program 
	./self_test.sh 5 300000
test:
	gcc -std=c11 test.c -o test ${FLAGS} -fwhole-program
clean:
	rm -f rsort *.o test

