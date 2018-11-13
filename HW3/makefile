all: parser tcount rsort test
rsort: rsort.c
	gcc rsort.c -O2 -o rsort
parser: parser.c
	gcc parser.c -O2 -o parser
tcount: tcount.c
	gcc tcount.c -O2 -o tcount
test: test.c
	gcc test.c -O2 -o test
clean:
	rm -rf parser tcount rsort test *.dSYM

