all: parser word_count feature_extracter find_news_by_id
parser: parser.c record_structure_io.c
	gcc parser.c record_structure_io.c -std=c11 -O0 -o parser
word_count: word_count.cpp record_structure_io.c jieba_word_count.hpp ngram_word_count.hpp wstringcvt.hpp
	g++ word_count.cpp record_structure_io.c -std=gnu++11 -openmp -fopenmp -O3 -o word_count
feature_extracter: feature_extracter.cpp read_histogram.hpp
	g++ feature_extracter.cpp -std=gnu++11 -O3 -o feature_extracter
find_news_by_id: find_news_by_id.cpp record_structure_io.c
	g++ find_news_by_id.cpp record_structure_io.c -std=gnu++11 -O2 -o find_news_by_id
clean:
	rm -f parser word_count feature_extracter find_news_by_id

