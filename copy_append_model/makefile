FLAGS=-march=native -O3
all: copy_append_model delta_resolver
delta_resolver: delta_resolver.cpp filereader.hpp
	g++ delta_resolver.cpp ${FLAGS} -o delta_resolver
copy_append_model: copy_append_model.cpp filereader.hpp
	g++ copy_append_model.cpp ${FLAGS} -o copy_append_model
clean:
	rm -f *.o copy_append_model delta_resolver

