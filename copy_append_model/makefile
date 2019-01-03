FLAGS=-march=native -O3 -std=c++11 -Wno-unused-result
all: copy_append_model delta_resolver compute_copy_range
delta_resolver: delta_resolver.cpp filereader.hpp
	g++ delta_resolver.cpp ${FLAGS} -o delta_resolver
copy_append_model: copy_append_model.cpp filereader.hpp
	g++ copy_append_model.cpp ${FLAGS} -o copy_append_model
compute_copy_range: compute_copy_range.cpp
	g++ compute_copy_range.cpp ${FLAGS} -o compute_copy_range
clean:
	rm -f *.o copy_append_model delta_resolver compute_copy_range

