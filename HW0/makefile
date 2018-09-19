all: parser
parser: parser.c postprocess.h postprocess
	gcc postprocess parser.c -std=c11 -march=native -O2 -o parser
postprocess:
	gcc -c postprocess.c -std=c11 -march=native -O2 -o postprocess
clean:
	rm -f parser postprocess

