/*
 * sequential.c
 *
 *  Created on: Jul 19, 2020
 *      Author: asaf
 */


#include <stdio.h>
#include <stdlib.h>
#include "sequential.h"
#include "stringFuncs.h"



void sequential()
{
	FILE* file=fopen("input.txt","r");
	int i,procToSend=0;
	int numOfAns=0;
	//int seq2NumTosend=1;
	double co1,co2,co3,co4;
	size_t buf=0;
	int numOfCharsInSeq1=0,numNs2=0;
	char** ns2Line=(char**)malloc(sizeof(char*));
	char** coLine=(char**)malloc(sizeof(char*));
	char** seq1=(char**)malloc(sizeof(char*));
	char** seq2=(char**)malloc(sizeof(char*));
	getline(coLine,&buf,file);
	sscanf(coLine[0],"%lf %lf %lf %lf",&co1,&co2,&co3,&co4);
	buf=0;
	numOfCharsInSeq1=getline(seq1,&buf,file);
	buf=0;
	// get the number of sequence 2
	numNs2=getline(ns2Line,&buf,file);
	int numOfCharsInSeq2=0;
	sscanf(ns2Line[0],"%d",&numNs2);
	int j;
	for (j=0;j<numNs2;j++)
	{
		size_t buf=0;
		numOfCharsInSeq2=getline(seq2,&buf,file);
		int difference=numOfCharsInSeq1-numOfCharsInSeq2-1;
		int i,k,n;
		int chosenN=0,chosenK=0;
		double maxScore=0;
		double currentScore=0;
		int num_of_stars=0,num_of_colons=0,num_of_dots=0,num_of_nothing=0;
		for (i=0;i<difference;i++)
		{
			for (k=0;k<numOfCharsInSeq2;k++)
			{
				*seq2=insert_hyphen_at_index(*seq2, numOfCharsInSeq2, k,j);
				for (n=0;n<numOfCharsInSeq2;n++)
				{
					switch(compareTwoChars(seq1[0][i+n], seq2[0][n]))
					{
					case 1:
						num_of_stars++;
						break;
					case 2:
						num_of_colons++;
						break;
					case 3:
						num_of_dots++;
						break;
					case 4:
						num_of_nothing++;
						break;
					default:
						break;
					}
				}
				currentScore=co1*num_of_stars-co2*num_of_colons-co3*num_of_dots-co4*num_of_nothing;
				if (currentScore>maxScore)
				{
					maxScore=currentScore;
					chosenN=i;
					chosenK=k;
				}
				*seq2=remove_char_by_index(*seq2, numOfCharsInSeq2, k, j);
				num_of_stars=0;
				num_of_colons=0;
				num_of_dots=0;
				num_of_nothing=0;
			}
		}
		printf("for sequence number %d the chosen offset is %d and hyphen location is %d\n",j,chosenN,chosenK);
	}

}

//returns 1 if two chars are identical
//returns 2 if two are in a conservative group
// returns 3 if two chars are in a semi conservative group
// returns 4 if two chars are none of the above
int compareTwoChars(char ch1,char ch2)
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
int checkIfInConservativeGroup(char ch1,char ch2)
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
int checkIfInSemiConservativeGroup(char ch1,char ch2)
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
