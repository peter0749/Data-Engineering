all: parser word_count feature_extracter find_news_by_id gen_corpus gen_wiki_corpus
parser: parser.c record_structure_io.c
	gcc parser.c record_structure_io.c -std=c11 -O0 -march=native -o parser
word_count: word_count.cpp cppjieba record_structure_io.c jieba_word_count.hpp wstringcvt.hpp
	g++ word_count.cpp record_structure_io.c -std=gnu++11 -openmp -fopenmp -O3 -march=native -o word_count
gen_corpus: gen_corpus.cpp record_structure_io.c cppjieba jieba_word_count.hpp 
	g++ gen_corpus.cpp record_structure_io.c -openmp -fopenmp -std=gnu++11 -o gen_corpus
gen_wiki_corpus: gen_wiki_corpus.cpp cppjieba jieba_word_count.hpp 
	g++ gen_wiki_corpus.cpp -openmp -fopenmp -std=gnu++11 -o gen_wiki_corpus
feature_extracter: feature_extracter.cpp read_histogram.hpp
	g++ feature_extracter.cpp -std=gnu++11 -O3 -march=native -o feature_extracter
find_news_by_id: find_news_by_id.cpp record_structure_io.c
	g++ find_news_by_id.cpp record_structure_io.c -std=gnu++11 -O2 -march=native -o find_news_by_id
cppjieba:
	git clone --depth=1 --single-branch --branch=master git://github.com/yanyiwu/cppjieba.git
	git -C cppjieba checkout 31eed035180582f45882d4587779754910be0951 -q
	cp -r cppjieba/deps/limonp cppjieba/include/cppjieba
	cp -r cppjieba/deps/gtest  cppjieba/include/cppjieba
	patch -p0 < Trie.hpp.patch
word2vec:
	git clone --depth=1 --single-branch --branch=master https://github.com/tmikolov/word2vec
	git -C word2vec checkout 20c129af10659f7c50e86e3be406df663beff438 -q
	$(MAKE) -C word2vec
compile_kmeans:
	$(MAKE) -C kmeans kmeans
data_cleaning: data data_cleaning.sh 
	./data_cleaning.sh
run_preprocessing: word2vec cppjieba parser gen_corpus gen_wiki_corpus wiki_data data
	./parser && ./gen_corpus && ./gen_wiki_corpus
	./word2vec/word2vec -train ./.db/word2vec/corpus.txt -output ./.db/word2vec/classes.txt -size 300 -window 5 -sample 1e-5 -hs 0 -threads 11 -iter 5 -min-count 3 -negative 10 -alpha 0.025 -binary 0 -cbow 0 -classes 30 
	./word2vec/word2vec -train ./.db/word2vec/corpus.txt -output ./.db/word2vec/vector.bin  -size 300 -window 5 -sample 1e-5 -hs 0 -threads 11 -iter 5 -min-count 3 -negative 10 -alpha 0.025 -binary 1 -cbow 0 
clean:
	$(MAKE) -C kmeans clean
	rm -rf cppjieba
	rm -rf .db
	rm -rf word2vec
	rm -f parser word_count feature_extracter find_news_by_id gen_corpus gen_wiki_corpus
	rm -f *.csv *.bin *.txt 

