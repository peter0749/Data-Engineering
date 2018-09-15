all: parser
parser: parser.c postprocess.h postprocess
	gcc postprocess parser.c -march=native -O2 -o parser
postprocess:
	gcc -c postprocess.c -march=native -O2 -o postprocess
clean:
	rm -f parser postprocess

