all: parser word_count
parser: parser.c record_structure_io.c
	gcc parser.c record_structure_io.c -std=c11 -O3 -o parser
word_count: word_count.cpp record_structure_io.c ngram.cpp
	g++ word_count.cpp ngram.cpp record_structure_io.c -std=gnu++11 -O3 -o word_count
clean:
	rm -f parser word_count

