//
// Created by zaki on 5/11/16.
//
#include <stdio.h>
#include <stdlib.h>

#include "error_codes.h"

void parse_update_error(char *command, int error_code) {
    if(error_code==UPDATE_SUCCESS) {
        printf("%s ",command);
        printf("SUCCESS\n");
    }
    if(error_code==UPDATE_ID_NOT_INT) {
        printf("%s ",command);
        printf(">>FAILED: ");
        printf("server id should be integer\n");
    }
    if(error_code==UPDATE_FIRST_ID_NOT_SELF) {
        printf("%s ",command);
        printf(">>FAILED: ");
        printf("First server id should be this server's id\n");
    }
    if(error_code==UPDATE_NEIGHBOUR_OUT_OF_BOUND) {
        printf("%s ",command);
        printf(">>FAILED: ");
        printf("Neighbour id out of bound\n");
    }
    if(error_code==UPDATE_COST_INCORRECT) {
        printf("%s ",command);
        printf(">>FAILED: ");
        printf("Cost should either be a number or inf\n");
    }
    if(error_code==UPDATE_NEIGHBOUR_NOT_FOUND) {
        printf("%s ",command);
        printf(">>FAILED: ");
        printf("Neighbour not found for the id\n");
    }
    if(error_code==UPDATE_NOT_ENOUGH_PARAMETERS) {
        printf("%s ",command);
        printf(">>FAILED: ");
        printf("Not enough parameters\n");
    }
}

void parse_disable_error(char *command, int error_code) {
    if(error_code==DISABLE_SUCCESS) {
        printf("%s ",command);
        printf("SUCCESS\n");
    }
    if(error_code==DISABLE_ID_NOT_INT) {
        printf("%s ",command);
        printf(">>FAILED: ");
        printf("server id should be integer\n");
    }
    if(error_code==DISABLE_NEIGHBOUR_NOT_FOUND) {
        printf("%s ",command);
        printf(">>FAILED: ");
        printf("Neighbour not found for the id\n");
    }
    if(error_code==DISABLE_NOT_ENOUGH_PARAMETERS) {
        printf("%s ",command);
        printf(">>FAILED: ");
        printf("Not enough parameters\n");
    }
}
