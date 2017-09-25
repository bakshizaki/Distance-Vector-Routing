//
// Created by zaki on 5/11/16.
//

#ifndef INC_589_P2_ERROR_CODES_H
#define INC_589_P2_ERROR_CODES_H

#define UPDATE_SUCCESS                  1
#define UPDATE_ID_NOT_INT               2
#define UPDATE_FIRST_ID_NOT_SELF        3
#define UPDATE_NEIGHBOUR_OUT_OF_BOUND   4
#define UPDATE_COST_INCORRECT           5
#define UPDATE_NEIGHBOUR_NOT_FOUND      6
#define UPDATE_NOT_ENOUGH_PARAMETERS    7


#define DISABLE_SUCCESS                 1
#define DISABLE_ID_NOT_INT              2
#define DISABLE_NEIGHBOUR_NOT_FOUND     3
#define DISABLE_NOT_ENOUGH_PARAMETERS   4

void parse_update_error(char *command, int error_code);
void parse_disable_error(char *command, int error_code);

#endif //INC_589_P2_ERROR_CODES_H
