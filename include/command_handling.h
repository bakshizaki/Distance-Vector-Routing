/*
 * command_handling.h
 *
 *  Created on: 23-Sep-2016
 *      Author: zaki
 */

#ifndef COMMAND_HANDLING_H_
#define COMMAND_HANDLING_H_

#define MAXTOKENS 4
#define NUMBEROFCOMMANDS 8

#define CMD_UPDATE		0
#define CMD_STEP		1
#define CMD_PACKETS		2
#define CMD_DISPLAY	    3
#define CMD_DISABLE		4
#define CMD_CRASH		5
#define CMD_TEST1       6
#define CMD_TEST2       7

void convertToLower(char *p);
int process_command(char *msg, int *command, char **arg1, char **arg2, char **arg3);




#endif /* COMMAND_HANDLING_H_ */
