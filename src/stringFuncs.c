/*
 * stringFuncs.c
 *
 *  Created on: Jul 14, 2020
 *      Author: asaf
 */
#include <stdio.h>
#include <stdlib.h>
#include "stringFuncs.h"



char* insert_hyphen_at_index(char* str,int len,int index,int seq2number)
{
	char* p=(char*)malloc(sizeof(char)*(len+1));
	int i;
	for (i=0;i<index;i++)
	{
		p[i]=str[i];
	}
	p[index]='-';
	for (i=index+1;i<len+1;i++)
	{
		p[i]=str[i-1];
	}
	return p;
}
char* remove_char_by_index(char* str,int len,int index,int seq2number)
{
	int i;
	char* p=(char*)malloc(sizeof(char)*(len));
	for (i=0;i<len;i++)
	{
		if (i>=index)
		{
			p[i]=str[i+1];
		}
		else
		{
			p[i]=str[i];
		}
	}
	//str=(char*)realloc(str,sizeof(char)*(len-1));
	return p;
}



