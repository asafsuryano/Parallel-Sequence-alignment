/*
 * cudaFuncs.h
 *
 *  Created on: Jul 15, 2020
 *      Author: asaf
 */

#ifndef CUDAFUNCS_H_
#define CUDAFUNCS_H_


void allocateSeq1AndCoefficients(char* seq1,int numOfChars,double co1,double co2,double co3,double co4);
double startCuda(int numOfChars,int i,int k,int seqNum);
void copyCountersFromCudaMemToNormalMem(int* stars,int* colons,int* dots,int* nothing);
void allocateSeq2(char* seq2Private,int numOfChars);
void freeSeq2();

#endif /* CUDAFUNCS_CUH_ */
