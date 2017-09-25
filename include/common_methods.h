/*
 * common_methods.h
 *
 *  Created on: 19-Oct-2016
 *      Author: zaki
 */

#include <stdint.h>

#ifndef COMMON_METHODS_H_
#define COMMON_METHODS_H_

#define DEBUG 0
int string_tokenizer(char *input_string,char *delimeter, char *tokens[], int token_array_size);
int parseInt(const char *str, int *val);
uint32_t parseIPStringToInt(char *ip);
void parseIPIntToString(int ip, char *ret_string);

#endif /* COMMON_METHODS_H_ */
