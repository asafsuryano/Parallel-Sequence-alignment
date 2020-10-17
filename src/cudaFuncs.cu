//this is a an experiment project
extern "C"{
#include "cudaFuncsHeader.h"
#include "stringFuncs.h"
}
#include <stdio.h>
#include <string.h>
#include <omp.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <helper_cuda.h>
#include <helper_functions.h>


__device__ int checkIfInConservativeGroup(char ch1,char ch2);
__device__ int checkIfInSemiConservativeGroup(char ch1,char ch2);
__device__ int compareTwoChars(char ch1,char ch2);
__global__ void compareAndReturnResult(char* seq1,char* seq2,int numOfCharsInSeq2,int n,int hyphen_index,double co1,double co2,double co3,double co4,int* pair_value);
__global__ void resetCounters();


//counters that will be in global memory so that all blocks and threads will access the same counters
__device__ int num_of_stars;
__device__ int num_of_colons;
__device__ int num_of_dots;
__device__ int num_of_nothing;

//global variable for sequence 1 on the gpu
char* seq1_on_gpu;
//global variable for sequence 2 on the gpu
char* seq2_On_Gpu;
//global variables for the coefficients (they are on the host side)
double co1_con,co2_con,co3_con,co4_con;
// pair value
int* pair_value_gpu;


//allocate sequence 1 to the gpu and it will stay there until the end of the program
extern "C"
void allocateSeq1AndCoefficients(char* seq1,int numOfChars,double co1,double co2,double co3,double co4)
{
	//int* pair;
	cudaMalloc((void**)&seq1_on_gpu,numOfChars);
	cudaMemcpy(seq1_on_gpu,seq1,numOfChars,cudaMemcpyHostToDevice);
	co1_con=co1;
	co2_con=co2;
	co3_con=co3;
	co4_con=co4;
	FILE* file=fopen("pair_values.txt","r");
	int* pair_value_arr=(int*)malloc(sizeof(int)*26*26);
	int i;
	for (i=0;i<26*26;i++)
	{
		fscanf(file,"%d",(pair_value_arr+i));
	}
	fclose(file);
	checkCudaErrors(cudaMalloc((void**)&pair_value_gpu,sizeof(int)*26*26));
	//checkCudaErrors(cudaMemcpyToSymbol(pair_value_gpu,pair_value_arr,sizeof(int)*26*26,0,cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemcpy(pair_value_gpu,pair_value_arr,sizeof(int)*26*26,cudaMemcpyHostToDevice));
}

extern "C"
void allocateSeq2(char* seq2Private,int numOfChars)
{
	//allocate space in gpu for sequence 2
	checkCudaErrors(cudaMalloc((void**)&seq2_On_Gpu,(numOfChars+1)*sizeof(char)));
	//copy the contents from sequence 2 to sequence 2 on the gpu
	checkCudaErrors(cudaMemcpy(seq2_On_Gpu,seq2Private,numOfChars+1,cudaMemcpyHostToDevice));
}
// this function will calculate the score with the help of cuda
extern "C"
double startCuda(int numOfChars,int i,int k,int seqNum)
{
	resetCounters<<<1,1>>>();
	int stars=0,colons=0,dots=0,nothing=0;
	int threads_per_block=256;
	int blocks_per_grid=(numOfChars+threads_per_block-1)/threads_per_block;
	//call the kernel function that will count the number of stars,colons,dots and nothing
	compareAndReturnResult<<<blocks_per_grid,threads_per_block>>>(seq1_on_gpu,seq2_On_Gpu,numOfChars,i,k,co1_con,co2_con,co3_con,co4_con,pair_value_gpu);
	//get the values from the counters in global memory
	copyCountersFromCudaMemToNormalMem(&stars,&colons,&dots,&nothing);
	//calculate and return the score
	double score=stars*co1_con-colons*co2_con-dots*co3_con-nothing*co4_con;
	return score;

}

extern "C"
void freeSeq2()
{
	checkCudaErrors(cudaFree(seq2_On_Gpu));
}

void copyCountersFromCudaMemToNormalMem(int* stars,int* colons,int* dots,int* nothing)
{
	checkCudaErrors(cudaMemcpyFromSymbol(stars,num_of_stars,sizeof(int),0,cudaMemcpyDeviceToHost));
	checkCudaErrors(cudaMemcpyFromSymbol(colons,num_of_colons,sizeof(int),0,cudaMemcpyDeviceToHost));
	checkCudaErrors(cudaMemcpyFromSymbol(dots,num_of_dots,sizeof(int),0,cudaMemcpyDeviceToHost));
	checkCudaErrors(cudaMemcpyFromSymbol(nothing,num_of_nothing,sizeof(int),0,cudaMemcpyDeviceToHost));
}

//returns 1 if two chars are identical
//returns 2 if two are in a conservative group
// returns 3 if two chars are in a semi conservative group
// returns 4 if two chars are none of the above
__device__ int compareTwoChars(char ch1,char ch2)
{
	
	if (ch1==ch2)
	{
		return 1;
	}
	else if (checkIfInConservativeGroup(ch1,ch2)==1)
	{
		return 2;
	}
	else if (checkIfInSemiConservativeGroup(ch1,ch2)==1)
	{
		return 3;
	}
	else
	{
		return 4;
	}
}

//this function will return 1 if they are in a conservative group or else 0
__device__ int checkIfInConservativeGroup(char ch1,char ch2)
{
	int numOfGroups=9;
	//define the conservative group
	const char *conservative[]={"NDEQ\0","NEQK\0","STA\0","MILV\0","QHRK\0","NHQK\0","FYW\0","HY\0","MILF\0"};
	//define the number of characters in each group
	const int conservative_num[]={4,4,3,4,4,4,3,2,4};
	int i,j;
	int ch1_is_in=0,ch2_is_in=0;
	for (i=0;i<numOfGroups;i++)
	{
		for (j=0;j<conservative_num[i];j++)
		{
			if (ch1==conservative[i][j])
			{
				ch1_is_in=1;
			}
			if (ch2==conservative[i][j])
			{
				ch2_is_in=1;
			}
		}
		if ((ch1_is_in==1)&&(ch2_is_in==1))
		{
			return 1;
		}
		ch1_is_in=0;
		ch2_is_in=0;
	}
	return 0;
}

//this function will return 1 if they are in a semi conservative group or else 0
__device__ int checkIfInSemiConservativeGroup(char ch1,char ch2)
{
	int numOfGroups=11;
	//define the semi conservative groups
	const char* semiConservative[]={"SAG\0","ATV\0","CSA\0","SGND\0","STPA\0","STNK\0","NEQHRK\0","NDEQHK\0","SNDEQK\0","HFY\0","FVLIM\0"};
	//define the number of characters in each group
	const int semi_conservative_num[]={3,3,3,4,4,4,6,6,6,3,5};
	int i,j;
	int ch1_is_in=0,ch2_is_in=0;
	for (i=0;i<numOfGroups;i++)
	{
		for (j=0;j<semi_conservative_num[i];j++)
		{
			if (ch1==semiConservative[i][j])
			{
				ch1_is_in=1;
			}
			if (ch2==semiConservative[i][j])
			{
				ch2_is_in=1;
			}
		}
		if ((ch1_is_in==1)&&(ch2_is_in==1))
		{
			return 1;
		}
		ch1_is_in=0;
		ch2_is_in=0;
	}
	return 0;
}


//this function compares seq1 to seq2 given offset n and returns the score
__global__ void compareAndReturnResult(char* seq1,char* seq2,int numOfCharsInSeq2,int n,int hyphen_index,double co1,double co2,double co3,double co4,int* pair_value)
{
	int index,ans=0;
	char ch1,ch2;
	index=blockDim.x * blockIdx.x + threadIdx.x;
	if (index>=hyphen_index)
		ch1=seq1[n+index+1];
	else
		ch1=seq1[n+index];
	ch2=seq2[index];
	if (index<numOfCharsInSeq2)
	{

		ans = pair_value[(ch1-65)*26+(ch2-65)];
		switch(ans)
		{
		case 1:
			//printf("got a star\n");
			atomicAdd(&num_of_stars,1);
			break;
		case 2:
			atomicAdd(&num_of_colons,1);
			break;
		case 3:
			atomicAdd(&num_of_dots,1);
			break;
		case 4:
			atomicAdd(&num_of_nothing,1);
			break;
		default:
			break;
		}
	}
}

//reset all counters
__global__ void resetCounters()
{
	 num_of_stars=0;
	 num_of_colons=0;
	 num_of_dots=0;
	 num_of_nothing=0;
}





