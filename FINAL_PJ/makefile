all: parser word_count
parser: parser.c record_structure_io.c
	gcc parser.c record_structure_io.c -std=c11 -O2 -o parser
word_count: word_count.cpp record_structure_io.c jieba_word_count.hpp wstringcvt.hpp
	g++ word_count.cpp record_structure_io.c -std=gnu++11 -openmp -fopenmp -O2 -o word_count
clean:
	rm -f parser word_count

