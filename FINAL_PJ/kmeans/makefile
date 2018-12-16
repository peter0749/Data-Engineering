all: kmeans
kmeans: main.c kmeans.o
	gcc main.c kmeans.o -lm -openmp -fopenmp -O3 --fast-math -march=native -o kmeans
kmeans.o: kmeans.c
	gcc kmeans.c -c -openmp -fopenmp -O3 --fast-math -march=native -o kmeans.o
clean:
	rm -f kmeans *.o
	rm -f *.bin *.csv

