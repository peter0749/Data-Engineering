all: parser tcount rsort
rsort: rsort.c
	gcc rsort.c -O2 -o rsort
parser: parser.c
	gcc parser.c -O2 -o parser
tcount: tcount.c
	gcc tcount.c -O2 -o tcount
clean:
	rm -rf parser tcount rsort *.dSYM

