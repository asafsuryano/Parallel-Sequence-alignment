CC    =   mpicc
FLAGS = -fopenmp

all: program

program: parallel_project.o stringFuncs.o cudaFuncs.o sequential.o
	$(CC) $(FLAGS) -o program parallel_project.o stringFuncs.o cudaFuncs.o sequential.o  /usr/local/cuda-9.1/lib64/libcudart_static.a -ldl -lrt -lstdc++

parallel_project.o: parallel_project.c stringFuncs.h sequential.h
	$(CC) $(FLAGS) -c parallel_project.c 

stringFuncs.o: stringFuncs.c stringFuncs.h
	gcc -c stringFuncs.c

sequential.o: sequential.c sequential.h
	gcc -c sequential.c 

cudaFuncs.o: cudaFuncs.cu cudaFuncsHeader.h
	nvcc  -I /usr/local/cuda-9.1/samples/common/inc  -c cudaFuncs.cu -o cudaFuncs.o 

clean:
	rm -f parallel_project.o stringFuncs.o cudaFuncs.o
