/*
 * command_handling.c
 *
 *  Created on: 23-Sep-2016
 *      Author: zaki
 */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include"command_handling.h"

char *command_array[NUMBEROFCOMMANDS] = { "update", "step", "packets", "display", "disable", "crash","test1","test2"};


void convertToLower(char *p) {
	for (; *p; ++p)
		*p = tolower(*p);
}

int process_command(char *cmd_msg, int *command, char **arg1, char **arg2, char **arg3) {
	char msg[500];
	int token_count = 0;
	char *token_array[MAXTOKENS];
	strcpy(msg,cmd_msg);

	char *p = strtok(msg, " ");
	char *command_string;
	int i;

	while (p != NULL) {
		if (token_count == MAXTOKENS)
			break;
		token_array[token_count++] = p;
		p = strtok(NULL, " ");
	}
//	for(i=0;i<token_count;i++)
//		printf("%s\n",token_array[i]);
	if (p!=NULL) {
		printf("more than allowed arguments in command\n");
		return 0;
	}
	command_string = token_array[0];
//	printf("cms:%s\n",command_string);
	convertToLower(command_string);
	for (i = 0; i < NUMBEROFCOMMANDS; i++) {
		if (strcmp(command_string, command_array[i]) == 0) {
//			printf("found\n");
			*command = i;
		}
	}
	if (*command < 0 || *command > (NUMBEROFCOMMANDS-1)) {
		printf("%s FAILED: not a valid command\n",cmd_msg);
		return 0;
	}
	if (token_count >= 2) {
		*arg1=(char *)calloc(strlen(token_array[1])+1,sizeof(char));
		strcpy(*arg1,token_array[1]);
	}
	if (token_count >= 3) {
		*arg2=(char *)calloc(strlen(token_array[2])+1,sizeof(char));
		strcpy(*arg2,token_array[2]);
	}
	if (token_count == 4) {
		*arg3=(char *)calloc(strlen(token_array[3])+1,sizeof(char));
		strcpy(*arg3,token_array[3]);
	}
	return 1;
}
