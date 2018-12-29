FLAGS=-O3
all: copy_append_model delta_resolver
delta_resolver: delta_resolver.cpp filereader.hpp
	g++ delta_resolver.cpp ${FLAGS} -o delta_resolver
copy_append_model: copy_append_model.cpp sais.o SegmentTree.hpp filereader.hpp
	g++ copy_append_model.cpp sais.o ${FLAGS} -o copy_append_model
sais.o: sais.c sais.h
	gcc sais.c ${FLAGS} -c -o sais.o
clean:
	rm -f *.o copy_append_model delta_resolver

