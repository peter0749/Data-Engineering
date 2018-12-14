all: parser word_count feature_extracter find_news_by_id
parser: parser.c record_structure_io.c
	gcc parser.c record_structure_io.c -std=c11 -O0 -march=native -o parser
word_count: word_count.cpp record_structure_io.c ngram_word_count.hpp wstringcvt.hpp
	g++ word_count.cpp record_structure_io.c -std=gnu++11 -openmp -fopenmp -O3 -march=native -o word_count
feature_extracter: feature_extracter.cpp read_histogram.hpp
	g++ feature_extracter.cpp -std=gnu++11 -O3 -march=native -o feature_extracter
find_news_by_id: find_news_by_id.cpp record_structure_io.c
	g++ find_news_by_id.cpp record_structure_io.c -std=gnu++11 -O2 -march=native -o find_news_by_id
cppjieba:
	git clone --depth=10 --branch=master git://github.com/yanyiwu/cppjieba.git
	cp -r cppjieba/deps/limonp cppjieba/include/cppjieba
	cp -r cppjieba/deps/gtest  cppjieba/include/cppjieba
clean:
	rm -rf cppjieba
	rm -rf .db
	rm -f parser word_count feature_extracter find_news_by_id

