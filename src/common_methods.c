/*
 * common_methods.c
 *
 *  Created on: 19-Oct-2016
 *      Author: zaki
 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<stdbool.h>
#include<errno.h>
#include<limits.h>
#include <stdint.h>
#include <netinet/in.h>

#include"common_methods.h"

int string_tokenizer(char *input_string,char *delimeter, char *tokens[], int token_array_size) { // split string into tokens are returns number of tokens
	char msg[BUFSIZ];
	int token_count = 0;
//	char *token_array[10];
	strcpy(msg, input_string);
	char *p = strtok(msg, delimeter);
	int i;

	while (p != NULL) {
		if (token_count == token_array_size)
				break;
		tokens[token_count]=(char *) calloc((strlen(p)+1),sizeof(char));
//		tokens[token_count]=(char *) malloc(1025*sizeof(char));
		strcpy(tokens[token_count],p);
		token_count++;
		p = strtok(NULL, delimeter);
	}
	if (p!=NULL) {
			if(DEBUG) printf("more than allowed tokens\n");
			return token_count;
		}
//		if(DEBUG) {
//		for(i=0;i<token_count;i++)
//			printf("%s\n",tokens[i]);
//		}
		return token_count;

}
int parseInt(const char *str, int *val) {
	char *temp;
	int rc = 1;
	errno = 0;
	*val = (int)strtol(str, &temp, 0);

	if (temp == str || *temp != '\0' || ((*val == LONG_MIN || *val == LONG_MAX) && errno == ERANGE)) {
		rc = 0;
	}

	return rc;
}

uint32_t parseIPStringToInt(char *ip) {
	uint32_t ret_int=0;
	int temp;
	char ip_addr[20];
	strcpy(ip_addr,ip);
	char *p=strtok(ip_addr,".");
	while(p!=NULL) {
		parseInt(p,&temp);
		ret_int=(ret_int)<<8|temp;
		p=strtok(NULL,".");
	}
	return ret_int;
}

void parseIPIntToString(int ip, char *ret_string)
{
	unsigned char bytes[4];
	char temp_string[INET_ADDRSTRLEN];
	bytes[0] = ip & 0xFF;
	bytes[1] = (ip >> 8) & 0xFF;
	bytes[2] = (ip >> 16) & 0xFF;
	bytes[3] = (ip >> 24) & 0xFF;
	sprintf(temp_string,"%d.%d.%d.%d", bytes[3], bytes[2], bytes[1], bytes[0]);
	strcpy(ret_string,temp_string);
}
