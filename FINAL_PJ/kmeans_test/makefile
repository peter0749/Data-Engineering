all: tester
tester: main.c kmeans.c
	gcc main.c kmeans.c -lm -openmp -fopenmp -Ofast --fast-math -o tester
clean:
	rm -f tester

