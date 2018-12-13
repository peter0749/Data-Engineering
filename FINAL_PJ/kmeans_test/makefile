all: kmeans
kmeans: main.c kmeans.o
	gcc main.c kmeans.o -lm -openmp -fopenmp -Ofast --fast-math -o kmeans
kmeans.o: kmeans.c
	gcc kmeans.c -c -openmp -fopenmp -Ofast --fast-math -o kmeans.o
clean:
	rm -f kmeans *.o

