all: parser word_count feature_extracter find_news_by_id gen_corpus
parser: parser.c record_structure_io.c
	gcc parser.c record_structure_io.c -std=c11 -O0 -march=native -o parser
word_count: word_count.cpp cppjieba record_structure_io.c jieba_word_count.hpp wstringcvt.hpp
	g++ word_count.cpp record_structure_io.c -std=gnu++11 -openmp -fopenmp -O3 -march=native -o word_count
gen_corpus: gen_corpus.cpp record_structure_io.c cppjieba jieba_word_count.hpp 
	g++ gen_corpus.cpp record_structure_io.c -openmp -fopenmp -std=gnu++11 -o gen_corpus
feature_extracter: feature_extracter.cpp read_histogram.hpp
	g++ feature_extracter.cpp -std=gnu++11 -O3 -march=native -o feature_extracter
find_news_by_id: find_news_by_id.cpp record_structure_io.c
	g++ find_news_by_id.cpp record_structure_io.c -std=gnu++11 -O2 -march=native -o find_news_by_id
cppjieba:
	git clone --depth=10 --branch=master git://github.com/yanyiwu/cppjieba.git
	cp -r cppjieba/deps/limonp cppjieba/include/cppjieba
	cp -r cppjieba/deps/gtest  cppjieba/include/cppjieba
word2vec:
	git clone https://github.com/tmikolov/word2vec
	$(MAKE) -C word2vec
compile_kmeans:
	$(MAKE) -C kmeans kmeans
run_preprocessing: word2vec cppjieba parser gen_corpus 
	./parser && ./gen_corpus && ./word2vec/word2vec -train ./.db/word2vec/corpus.txt -output ./.db/word2vec/classes.txt -size 100 -window 5 -sample 1e-3 -hs 1 -negative 5 -threads 12 -iter 20 -min-count 5 -alpha 0.025 -binary 0 -cbow 1 -classes 30
clean:
	$(MAKE) -C kmeans clean
	rm -rf cppjieba
	rm -rf .db
	rm -rf word2vec
	rm -f parser word_count feature_extracter find_news_by_id gen_corpus

