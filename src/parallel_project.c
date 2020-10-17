/*
 ============================================================================
 Name        : parallel_project.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello MPI World in C 
 ============================================================================
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>
#include "mpi/mpi.h"
#include "sequential.h"
#include "cudaFuncsHeader.h"
#include "stringFuncs.h"

void receiveCoefficientsFromPrcessZero(double* co1,double* co2,double* co3,double* co4);
void sendResultToMasterProcess(int seqNum,int chosenN,int chosenK);
void receiveSeq1FromMasterProcess(char** seq1,int* numOfCharsInSeq1);
void receiveSeq2FromMasterProcess(char** seq2,int* numOfCharsInSeq2,int *seqNum);
int sendSeq2ToWorkerProcessNumJ(int i,int j,FILE* file);
int receiveFromWorkerProcessesResultsAndOutputToFile(int numOfAns,int done,int* procToSend,FILE* output_file);

int main(int argc, char* argv[]){
	int  my_rank; /* rank of process */
	int  num_proc;       /* number of processes */
	int tag=0;    /* tag for messages */
	MPI_Status status ;   /* return status for receive */
	
	/* start up MPI */
	
	MPI_Init(&argc, &argv);
	
	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 
	
	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
	
	if (num_proc==1)
	{
		sequential();
		return 1;
	}
	
	if (my_rank == 0)
	{
		FILE* file=fopen("input.txt","r");
		FILE* output_file=fopen("output.txt","w");
		int i,procToSend=0;
		int numOfAns=0;
		//int seq2NumTosend=1;
		double co1,co2,co3,co4;
		size_t buf=0;
		int numOfCharsInSeq1=0,numNs2=0;
		char** ns2Line=(char**)malloc(sizeof(char*));
		char** coLine=(char**)malloc(sizeof(char*));
		char** seq1=(char**)malloc(sizeof(char*));
		getline(coLine,&buf,file);
		sscanf(coLine[0],"%lf %lf %lf %lf",&co1,&co2,&co3,&co4);
		buf=0;
		numOfCharsInSeq1=getline(seq1,&buf,file);
		//send to every worker process the coefficients and sequence 1
		for (i=1;i<num_proc;i++)
		{
			MPI_Send(&co1,1,MPI_DOUBLE,i,tag,MPI_COMM_WORLD);
			MPI_Send(&co2,1,MPI_DOUBLE,i,tag,MPI_COMM_WORLD);
			MPI_Send(&co3,1,MPI_DOUBLE,i,tag,MPI_COMM_WORLD);
			MPI_Send(&co4,1,MPI_DOUBLE,i,tag,MPI_COMM_WORLD);
			MPI_Send(&numOfCharsInSeq1,1,MPI_INT,i,tag,MPI_COMM_WORLD);
			MPI_Send(seq1[0],numOfCharsInSeq1,MPI_CHAR,i,tag,MPI_COMM_WORLD);
		}
		buf=0;
		// get the number of sequence 2
		numNs2=getline(ns2Line,&buf,file);
		sscanf(ns2Line[0],"%d",&numNs2);
		i=0;
		int j;
		for (j=1;j<num_proc;j++)
		{
			if (i<numNs2)
			{
				int done=0;
				MPI_Send(&done,1,MPI_INT,j,tag,MPI_COMM_WORLD);
				i=sendSeq2ToWorkerProcessNumJ(i, j, file);
			}
			else
			{
				int done=1;
				MPI_Send(&done,1,MPI_INT,j,tag,MPI_COMM_WORLD);
			}
		}
		while (i<numNs2)
		{
			numOfAns=receiveFromWorkerProcessesResultsAndOutputToFile(numOfAns, 0,&procToSend,output_file);
			i=sendSeq2ToWorkerProcessNumJ(i, procToSend, file);
		}
		while (numOfAns<numNs2)
		{
			numOfAns=receiveFromWorkerProcessesResultsAndOutputToFile(numOfAns, 1,&procToSend,output_file);
		}
		fclose(file);
		fclose(output_file);

	}
	else
	{
		int done;
		int numOfCharsInSeq1,numOfCharsInSeq2;
		char** seq1;
		double co1,co2,co3,co4;
		int seqNum;
		MPI_Status status ;   /* return status for receive */
		seq1=(char**)malloc(sizeof(char*));
		receiveCoefficientsFromPrcessZero(&co1,&co2,&co3,&co4);
		receiveSeq1FromMasterProcess(seq1,&numOfCharsInSeq1);
		MPI_Recv(&done,1,MPI_INT,0,0,MPI_COMM_WORLD,&status);
		allocateSeq1AndCoefficients(*seq1, numOfCharsInSeq1, co1, co2, co3, co4);
		omp_lock_t writelock,writelock2;
		omp_init_lock(&writelock);
		omp_init_lock(&writelock2);
		while (done==0)
		{
			char** seq2 = (char**)malloc(sizeof(char*));
			receiveSeq2FromMasterProcess(seq2, &numOfCharsInSeq2, &seqNum);
			allocateSeq2(seq2[0],numOfCharsInSeq2);
			int difference=numOfCharsInSeq1-numOfCharsInSeq2-1;
			int chosenN=0,chosenK=0,num_threads=0;
			double maxScore=0;
#pragma omp parallel shared(maxScore,chosenN,chosenK,numOfCharsInSeq2)
			{
				int k,i;
				double currentScore;
				//allocate memory for the private variable sequence 2 and copy the sequence 2 to the private variable
				char* seq2Private=(char*)malloc(sizeof(char)*(numOfCharsInSeq2));
				for (i=0;i<numOfCharsInSeq2;i++)
				{
					seq2Private[i]=seq2[0][i];
				}
				//calculating between what indexes each threads is responsible for
				int tid=omp_get_thread_num();
				num_threads=omp_get_num_threads();
				int start=tid*(difference/num_threads);
				int end=(tid+1)*(difference/num_threads);
				if (tid==num_threads-1)
				{
					end+=(difference%num_threads);
				}
#pragma omp parallel for collapse(2)
				for (i=start;i<end;i++)
				{
					for (k=0;k<numOfCharsInSeq2;k++)
					{
						omp_set_lock(&writelock2);
						currentScore=startCuda(numOfCharsInSeq2, i, k,seqNum);
						omp_unset_lock(&writelock2);
						omp_set_lock(&writelock);
						if (currentScore>maxScore)
						{
							maxScore=currentScore;
							chosenK=k;
							chosenN=i;
						}
						omp_unset_lock(&writelock);
					}
				}
				currentScore=0;
				free(seq2Private);
			}
			sendResultToMasterProcess(seqNum,chosenN,chosenK);
			MPI_Recv(&done,1,MPI_INT,0,tag,MPI_COMM_WORLD,&status);
			free(seq2);
			freeSeq2();
		}
		free(*seq1);
		free(seq1);
	}
	/* shut down MPI */
	MPI_Finalize(); 
	
	
	return 0;
}






//get all coefficients from master process
//the complexity of this function is o(1)
void receiveCoefficientsFromPrcessZero(double* co1,double* co2,double* co3,double* co4)
{
	int tag=0;    /* tag for messages */
	MPI_Status status ;   /* return status for receive */
	MPI_Recv(co1,1,MPI_DOUBLE,0,tag,MPI_COMM_WORLD,&status);
	MPI_Recv(co2,1,MPI_DOUBLE,0,tag,MPI_COMM_WORLD,&status);
	MPI_Recv(co3,1,MPI_DOUBLE,0,tag,MPI_COMM_WORLD,&status);
	MPI_Recv(co4,1,MPI_DOUBLE,0,tag,MPI_COMM_WORLD,&status);
}

//send the result offset and hyphen location to master process
//the complexity of this function is o(1)
void sendResultToMasterProcess(int seqNum,int chosenN,int chosenK)
{
	int tag=0;    /* tag for messages */
	MPI_Send(&seqNum,1,MPI_INT,0,tag,MPI_COMM_WORLD);
	MPI_Send(&chosenN,1,MPI_INT,0,tag,MPI_COMM_WORLD);
	MPI_Send(&chosenK,1,MPI_INT,0,tag,MPI_COMM_WORLD);
}


//receive seq1 from master process
//the complexity of this function is o(1)
void receiveSeq1FromMasterProcess(char** seq1,int* numOfCharsInSeq1)
{
	MPI_Status status ;   /* return status for receive */
	int tag=0;
	MPI_Recv(numOfCharsInSeq1,1,MPI_INT,0,tag,MPI_COMM_WORLD,&status);
	*seq1=(char*)malloc(sizeof(char)*(*(numOfCharsInSeq1)));
	MPI_Recv(*seq1,*(numOfCharsInSeq1),MPI_CHAR,0,tag,MPI_COMM_WORLD,&status);
}


//receive seq1 from master process
//the complexity of this function is o(1)
void receiveSeq2FromMasterProcess(char** seq2,int* numOfCharsInSeq2,int *seqNum)
{
	MPI_Status status ;   /* return status for receive */
	int tag=0;
	MPI_Recv(seqNum,1,MPI_INT,0,tag,MPI_COMM_WORLD,&status);
	MPI_Recv(numOfCharsInSeq2,1,MPI_INT,0,tag,MPI_COMM_WORLD,&status);
	*seq2=(char*)malloc(sizeof(char)*(*(numOfCharsInSeq2)));
	MPI_Recv(*seq2,*(numOfCharsInSeq2),MPI_CHAR,0,tag,MPI_COMM_WORLD,&status);
}

//send seq2 number i to process number j
//the complexity of this function is o(1)
int sendSeq2ToWorkerProcessNumJ(int i,int j,FILE* file)
{
	char** seq2;
	seq2=(char**)malloc(sizeof(char*));
	int numOfCharsInSeq2=0,tag=0;
	size_t buf=0;
	numOfCharsInSeq2=getline(seq2,&buf,file);
	MPI_Send(&i,1,MPI_INT,j,tag,MPI_COMM_WORLD);
	MPI_Send(&numOfCharsInSeq2,1,MPI_INT,j,tag,MPI_COMM_WORLD);
	MPI_Send(seq2[0],numOfCharsInSeq2,MPI_CHAR,j,tag,MPI_COMM_WORLD);
	free(seq2[0]);
	free(seq2);
	i++;
	return i;
}


//in this function we receive an answer from one of the processes after it finishes and outputing the result to a file
//the complexity of this function is o(1)
int receiveFromWorkerProcessesResultsAndOutputToFile(int numOfAns,int done,int* procToSend,FILE* output_file)
{
	MPI_Status status ;   /* return status for receive */
	int seq2num;
	int n,k;
	int tag=0;
	//receive the information of the output (offset n and hyphen location k)
	MPI_Recv(&seq2num,1,MPI_INT,MPI_ANY_SOURCE,tag,MPI_COMM_WORLD,&status);
	*procToSend=status.MPI_SOURCE;
	MPI_Recv(&n,1,MPI_INT,*procToSend,tag,MPI_COMM_WORLD,&status);
	MPI_Recv(&k,1,MPI_INT,*procToSend,tag,MPI_COMM_WORLD,&status);
	//telling the worker processes if there is more work or if it is done
	MPI_Send(&done,1,MPI_INT,*procToSend,tag,MPI_COMM_WORLD);
	//print to output.txt the result
	fprintf(output_file,"for seq2 number %d the highest alignment with offset %d and hyphen location %d\n",seq2num,n,k);
	//incrementing by one the number of answers given by the worker processes and returning it
	numOfAns++;
	return numOfAns;
}







