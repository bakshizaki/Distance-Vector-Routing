/*
 * mbakshi_proj2.c
 *
 *  Created on: 19-Oct-2016
 *      Author: zaki
 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <netinet/tcp.h>
#include <limits.h>
#include<stdint.h>


#define INT_BUF_SIZE 100

#include "server_node.h"
#include "common_methods.h"
#include "command_handling.h"
#include "error_codes.h"

void *get_in_addr(struct sockaddr *addr);

void readTopologyFile();

void sendUDP(int socket, void *buf, size_t len, char *ip, char *port);

void update_neighbour_dv(uint32_t rec_array[], int rec_int);

void input_validation(int argc, char *argv[]);

int update_neighbour_cost(int neighbour_id, int cost);

int update_command(char *arg1, char *arg2, char *arg3);

int disable_command(char *arg1);

void sendDV();

void calcDV();

void sendUpdateMessage(int neighbour_id, int cost);

int disable_neighbour(int neighbour_id);

server_node *neighbour_list;
server_node *distance_vector;
server_node *neighbours_dv_array[5];
int self_id = 1;
int sending_socket;
char *filename;
int interval;
char listening_port[6];
int packets_received=0;

int main(int argc, char *argv[]) {
    struct addrinfo hints, *ai_result, *p;
    int gai_result;
    int listener_socket;
    fd_set fds_master, fds_read;
    int yes = 1;
    int fdmax, i;
    struct sockaddr_storage remote_addr;
    socklen_t remote_addr_size;
    char remote_addr_string[INET6_ADDRSTRLEN];
    char rec_buf[BUFSIZ];
    int rec_bytes, sent_bytes;
    char send_buf[BUFSIZ];
    int cmd_err_code;
    struct timeval timeout;


    input_validation(argc, argv);
    timeout.tv_sec = interval;
    readTopologyFile();

    printf("---Starting---\n");
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((gai_result = getaddrinfo(NULL, listening_port, &hints, &ai_result)) != 0) {
        fprintf(stderr, "getaddrinfo:%s\n", gai_strerror(gai_result));
        exit(EXIT_FAILURE);
    }

    for (p = ai_result; p != NULL; p = p->ai_next) {
        listener_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener_socket == -1) {
            perror("socket");
            continue;
        }
        sending_socket = socket(p->ai_family, p->ai_socktype, 0);
        if (sending_socket == -1) {
            perror("socket");
            continue;
        }
        setsockopt(listener_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
        if (bind(listener_socket, p->ai_addr, p->ai_addrlen) == -1) {
            perror("bind");
            close(listener_socket);
            continue;
        }
        break;

    }
    if (p == NULL) {
        printf("could not get socket\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(ai_result);
    printf("listening to socket:%d\n", listener_socket);
    FD_ZERO(&fds_master);
    FD_ZERO(&fds_read);
    FD_SET(listener_socket, &fds_master);
    FD_SET(STDIN_FILENO, &fds_master);
    fdmax = listener_socket > STDIN_FILENO ? listener_socket : STDIN_FILENO;
    write(1, ">>", 2);
    while (1) {
        int select_rv;
        fds_read = fds_master;
        select_rv = select(fdmax + 1, &fds_read, NULL, NULL, &timeout);
        if (select_rv == -1) {
            perror("select");
        }
        if (select_rv == 0) {   //TIMEOUT
            server_node *temp_neighbour;
            if(DEBUG) printf("TIMEOUT\n");
            timeout.tv_sec = interval;
            temp_neighbour=neighbour_list;
            while(temp_neighbour!=NULL) {
                temp_neighbour->alive_counter++;
                if(temp_neighbour->alive_counter==4) {
                    temp_neighbour->cost = INF;
                    printf("Server timeout %d\n",temp_neighbour->id);
                }
                temp_neighbour=temp_neighbour->next;
            }
            calcDV();
            sendDV();
        }
        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &fds_read)) {
                if (i == listener_socket) {
                    uint32_t rec_array[INT_BUF_SIZE];
                    uint32_t received_array[INT_BUF_SIZE];
                    int iter, rec_int;
                    remote_addr_size = sizeof remote_addr;
                    if ((rec_bytes = recvfrom(listener_socket, received_array, (sizeof received_array), 0,
                                              (struct sockaddr *) &remote_addr, &remote_addr_size)) == -1) {
                        perror("ERROR recvfrom");
                        continue;
                    }
                    rec_int = rec_bytes / 4;
                    rec_buf[rec_bytes] = '\0';
                    inet_ntop(remote_addr.ss_family, get_in_addr((struct sockaddr *) &remote_addr), remote_addr_string,
                              sizeof remote_addr_string);
                    if(DEBUG) printf("Received msg from %s\n", remote_addr_string);
//                    printPacket(received_array, rec_int);
                    update_neighbour_dv(received_array, rec_int);
                    calcDV();
                } else if (i == STDIN_FILENO) {
                    int command = -1;
                    char *arg1 = NULL, *arg2 = NULL, *arg3 = NULL;

                    rec_bytes = read(i, rec_buf, sizeof rec_buf);
                    if (rec_bytes == -1) {
                        perror("read");
                        continue;
                    }
                    rec_buf[rec_bytes - 1] = '\0';
                    if (process_command(rec_buf, &command, &arg1, &arg2, &arg3) == -1) {
                        continue; //invalid command, skip the next part
                    }
                    switch (command) {
                        case CMD_UPDATE:
                            if (DEBUG)
                                printf("CMD_UPDATE\n");
                            cmd_err_code = update_command(arg1, arg2, arg3);
                            if (cmd_err_code == UPDATE_SUCCESS)
                                calcDV();
                            parse_update_error(rec_buf, cmd_err_code);

                            break;
                        case CMD_STEP:
                            if (DEBUG)
                                printf("CMD_STEP\n");
                            if(arg1!=NULL)
                                printf("%s FAILED: STEP DOES NOT NEED ARGUMETS\n",rec_buf);
                            else {
                                sendDV();
                                printf("%s SUCCESS\n", rec_buf);
                            }
                            break;
                        case CMD_PACKETS:
                            if (DEBUG)
                                printf("CMD_PACKETS\n");
                            if(arg1!=NULL)
                                printf("%s FAILED: PACKETS DOES NOT NEED ARGUMETS\n",rec_buf);
                            else {
                                printf("%s SUCCESS\n", rec_buf);
                                printf("Packets received:%d\n", packets_received);
                                packets_received = 0;
                            }
                            break;
                        case CMD_DISPLAY:
                            if (DEBUG)
                                printf("CMD_DISPLAY\n");
                            if(arg1!=NULL)
                                printf("%s FAILED: DISPLAY DOES NOT NEED ARGUMETS\n",rec_buf);
                            else {
                                printf("%s SUCCESS\n", rec_buf);
                                printServerNodeList(&distance_vector);
                            }

                            break;
                        case CMD_DISABLE:
                            if (DEBUG)
                                printf("CMD_DISABLE\n");
                            if(arg2!=NULL) {
                                printf("%s >> FAILED: INCORRECT ARGUMENTS\n",rec_buf);
                                break;
                            }
                            cmd_err_code=disable_command(arg1);
                            parse_disable_error(rec_buf,cmd_err_code);

                            break;
                        case CMD_CRASH:
                            if (DEBUG)
                                printf("CMD_CRASH\n");
                            exit(EXIT_SUCCESS);
                            break;
                        case CMD_TEST1:
                            if (DEBUG)
                                printf("CMD_TEST1\n");
                            {
                                int temp_id;
                                parseInt(arg1, &temp_id);
                                printServerNodeList(&neighbours_dv_array[temp_id - 1]);
                            }
                            break;
                        case CMD_TEST2:
                            if (DEBUG)
                                printf("CMD_TEST2\n");
                            printServerNodeList(&neighbour_list);
                            break;
                    }
                    write(1, ">>", 2);
                }

            }

        }
    }

}

void *get_in_addr(struct sockaddr *addr) {
    if (addr->sa_family == AF_INET)
        return &(((struct sockaddr_in *) addr)->sin_addr);
    else
        return &(((struct sockaddr_in6 *) addr)->sin6_addr);
}

void readTopologyFile() {
    FILE *file_ptr;
    char read_buf[1024];
    int total_servers;
    int total_neighbours;
    int i;
    char *ret_val;


    file_ptr = fopen(filename, "r");
    if (file_ptr == NULL) {
        printf("file not found/ permission denied\n");
        return;
    }

    ret_val = fgets(read_buf, sizeof read_buf, file_ptr);
    read_buf[strcspn(read_buf, "\r\n")] = 0;  //removes \n from end
    if (ret_val == NULL) {
        printf("Missing lines in file\n");
        return;
    }
    if (parseInt(read_buf, &total_servers) == 0) {
        printf("Error parsing integer\n");
        return;
    }

    ret_val = fgets(read_buf, sizeof read_buf, file_ptr);
    read_buf[strcspn(read_buf, "\r\n")] = 0;  //removes \n from end
    if (ret_val == NULL) {
        printf("Missing lines in file\n");
        return;
    }
    if (parseInt(read_buf, &total_neighbours) == 0) {
        printf("Error parsing integer\n");
        return;
    }
    //reading all servers ip and port
    for (i = 0; i < total_servers; i++) {
        char *tokens[3];
        int number_of_tokens;
        int temp_id;
        int j;

        ret_val = fgets(read_buf, sizeof read_buf, file_ptr);
        read_buf[strcspn(read_buf, "\r\n")] = 0;  //removes \n from end
        if (ret_val == NULL) {
            printf("Missing lines in file\n");
            return;
        }

        number_of_tokens = string_tokenizer(read_buf, " ", tokens, sizeof tokens);
        if (number_of_tokens != 3) {
            printf("Error in topology file\n");
            return;
        }
        parseInt(tokens[0], &temp_id);
//		if(temp_id==self_id)
//			addToServerNodeList(&distance_vector, temp_id, tokens[1], tokens[2], 0);
//		else
        addToServerNodeList(&distance_vector, temp_id, tokens[1], tokens[2], INF);
        for (j = 0; j < 5; j++) {
            if (temp_id == j + 1)
                addToServerNodeList(&neighbours_dv_array[j], temp_id, tokens[1], tokens[2], 0);
            else
                addToServerNodeList(&neighbours_dv_array[j], temp_id, tokens[1], tokens[2], INF);
        }
        for (j = 0; j < 3; j++) {
            free(tokens[j]);
        }
    }
    //reading neighbours cost
    for (i = 0; i < total_neighbours; i++) {
        char *tokens[3];
        int number_of_tokens;
        int temp_id, temp_cost;
        server_node *temp_node;
        int j;
        ret_val = fgets(read_buf, sizeof read_buf, file_ptr);
        read_buf[strcspn(read_buf, "\r\n")] = 0;  //removes \n from end
        if (ret_val == NULL) {
            printf("Missing lines in file\n");
            return;
        }

        number_of_tokens = string_tokenizer(read_buf, " ", tokens, sizeof tokens);
        if (number_of_tokens != 3) {
            printf("Error in topology file\n");
            return;
        }
        parseInt(tokens[0], &self_id);
        parseInt(tokens[1], &temp_id);
        parseInt(tokens[2], &temp_cost);
        temp_node = findById(&distance_vector, temp_id);
        addToServerNodeList(&neighbour_list, temp_node->id, temp_node->ip_address, temp_node->port, temp_cost);
        temp_node->cost = temp_cost;
        temp_node->next_hop_id = temp_id;
        for (j = 0; j < 3; j++) {
            free(tokens[j]);
        }

    }
    findById(&distance_vector, self_id)->cost = 0;  //setting cost to self as 0
    strcpy(listening_port, findById(&distance_vector, self_id)->port);
}




void sendUDP(int socket, void *buf, size_t len, char *ip, char *port) {
    struct addrinfo hints, *ai_result, *p;
    int gai_result, sent_bytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    if ((gai_result = getaddrinfo(ip, port, &hints, &ai_result)) != 0) {
        fprintf(stderr, "ERROR getaddrinfo:%s\n", gai_strerror(gai_result));
        return;
    }
    p = ai_result;
    if (p == NULL) {
        printf("ERROR exiting...");
        return;
    }
    if ((sent_bytes = sendto(socket, buf, len, 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("ERROR numbytes");
        return;
    }
    if (sent_bytes != len) {
        printf("Could not send whole data\n");
        return;
    }
    freeaddrinfo(ai_result);
    if (DEBUG) printf("%d bytes sent to %s\n", sent_bytes, ip);
}

int makeDVPacket(uint32_t send_buf[]) {
    int update_fields, temp;
    struct server_node *temp_node;
    int i, curr_int, total_int;

    update_fields = 4;
    temp_node = findById(&distance_vector, self_id);
    parseInt(temp_node->port, &temp);
    send_buf[0] = update_fields << 16 | temp;
    send_buf[1] = parseIPStringToInt(temp_node->ip_address);
    curr_int = 2;
    for (i = 0; i < 5; i++) {
        if (i + 1 == self_id)
            continue;
        temp_node = findById(&distance_vector, i + 1);
        send_buf[curr_int] = parseIPStringToInt(temp_node->ip_address);
        parseInt(temp_node->port, &temp);
        send_buf[curr_int + 1] = temp << 16 | 0;
        send_buf[curr_int + 2] = (temp_node->id) << 16 | temp_node->cost;
        curr_int += 3;
    }
    total_int = 2 + update_fields * 3;
    for (i = 0; i < total_int; i++) {
        send_buf[i] = htonl(send_buf[i]);
    }
    return total_int;

}

void sendDV() {
    struct server_node *p;
    uint32_t send_buf[INT_BUF_SIZE];
    int total_int;

    total_int = makeDVPacket(send_buf);
    if (total_int == 0)
        return;

    p = neighbour_list;
    while (p != NULL) {
        sendUDP(sending_socket, send_buf, total_int * 4, p->ip_address, p->port);
        p = p->next;
    }

}

void update_neighbour_dv(uint32_t rec_array[], int rec_int) {
    uint32_t converted_array[INT_BUF_SIZE];
    int i, curr_int;
    int update_fields;
    char port[6];
    char ip[INET_ADDRSTRLEN];
    int cost;
    int id, temp;
    char id_str[6], cost_str[6];
    server_node *temp_node, *neighbour_dv, *neighbour_temp;

    for (i = 0; i < rec_int; i++) {
        converted_array[i] = ntohl(rec_array[i]);
    }

    if (converted_array[0] == 123) {
        temp_node = findById(&neighbour_list, (int) converted_array[1]);
        temp_node->cost = (int) converted_array[2];
        printf("Changing cost to %d neighbour to %d\n", converted_array[1], converted_array[2]);
    } else {
        update_fields = converted_array[0] >> 16;
        sprintf(port, "%d", converted_array[0] & 0xFFFF);
        parseIPIntToString(converted_array[1], ip);
        temp_node = findByIPPort(&distance_vector, ip, port);
        neighbour_temp=findById(&neighbour_list,temp_node->id);
        if(neighbour_temp==NULL)
            return;
        neighbour_temp->alive_counter=0;
        neighbour_dv = neighbours_dv_array[(temp_node->id) - 1];
        printf("Received update from server %d\n",temp_node->id);
        for (curr_int = 2, i = 0; i < update_fields; i++, curr_int += 3) {
            sprintf(id_str, "%d", converted_array[curr_int + 2] >> 16);
            sprintf(cost_str, "%d", converted_array[curr_int + 2] & 0xFFFF);
            parseInt(id_str, &id);
            parseInt(cost_str, &cost);
            temp_node = findById(&neighbour_dv, id);
            temp_node->cost = cost;
        }
        packets_received++;
    }
}

void input_validation(int argc, char *argv[]) {
    if (argc != 5) {
        printf("incorrect number of commands, show usage\n");
        exit(EXIT_FAILURE);
    }
    if (!(((strcmp(argv[1], "-t") == 0) && (strcmp(argv[3], "-i") == 0)) ||
          ((strcmp(argv[1], "-i") == 0) && (strcmp(argv[3], "-t") == 0)))) {
        printf("input arguments incorrect\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(argv[1], "-t") == 0) {
        filename = strdup(argv[2]);
        if (parseInt(argv[4], &interval) == 0) {
            printf("Interval should be integer\n");
            exit(EXIT_FAILURE);
        }
    }
    if (strcmp(argv[1], "-i") == 0) {
        filename = strdup(argv[4]);
        if (parseInt(argv[2], &interval) == 0) {
            printf("Interval should be integer\n");
            exit(EXIT_FAILURE);
        }
    }

}

int update_neighbour_cost(int neighbour_id, int cost) {
    server_node *temp_node;
    temp_node = findById(&neighbour_list, neighbour_id);
    if (temp_node == NULL)
        return 0;
    temp_node->cost = cost;
    return 1;
}


int disable_command(char *arg1) {
    int neighbour_id;

    if(arg1==NULL)
        return DISABLE_NOT_ENOUGH_PARAMETERS;

    if(parseInt(arg1, &neighbour_id) == 0)
        return DISABLE_ID_NOT_INT;

    if(disable_neighbour(neighbour_id)==0)
        return DISABLE_NEIGHBOUR_NOT_FOUND;
    else
        return DISABLE_SUCCESS;

}

int update_command(char *arg1, char *arg2, char *arg3) {

    int read_self_id, neighbour_id, cost;
    if (arg1 == NULL || arg2 == NULL || arg3 == NULL) {
        return UPDATE_NOT_ENOUGH_PARAMETERS;
    }

    if (parseInt(arg1, &read_self_id) == 0 || parseInt(arg2, &neighbour_id) == 0) {
        return UPDATE_ID_NOT_INT;
    }
    if (read_self_id != self_id) {
        return UPDATE_FIRST_ID_NOT_SELF;
    }
    if (neighbour_id < 1 || neighbour_id > 5) {
        return UPDATE_NEIGHBOUR_OUT_OF_BOUND;
    }
    if (strcmp(arg3, "inf") != 0 && parseInt(arg3, &cost) == 0) {
        return UPDATE_COST_INCORRECT;
    }
    if (strcmp(arg3, "inf") == 0) {
        if (update_neighbour_cost(neighbour_id, INF) == 0)
            return UPDATE_NEIGHBOUR_NOT_FOUND;
        else
            sendUpdateMessage(neighbour_id, INF);
    } else {
        if (update_neighbour_cost(neighbour_id, cost) == 0)
            return UPDATE_NEIGHBOUR_NOT_FOUND;
        else
            sendUpdateMessage(neighbour_id, cost);
    }
    return UPDATE_SUCCESS;
}

void sendUpdateMessage(int neighbour_id, int cost) {

    uint32_t send_buf[3];
    server_node *temp_node;
    int i;

    temp_node = findById(&neighbour_list, neighbour_id);
    send_buf[0] = 123;
    send_buf[1] = self_id;
    send_buf[2] = cost;
    for (i = 0; i < 3; i++) {
        send_buf[i] = htonl(send_buf[i]);
    }

    sendUDP(sending_socket, send_buf, 3 * sizeof(uint32_t), temp_node->ip_address, temp_node->port);
}


void calcDV() {
    server_node *p, *q;
    p = distance_vector;
    int here_to_q, q_to_p_cost;
//    is_changed = 0;
    while (p != NULL) {        // for all nodes in network
        if (p->id == self_id) {
            p = p->next;
            continue;
        }
        int min_cost = INF;
        int min_next_hop_id = 0;
        q = neighbour_list;
        while (q != NULL) {    // for every neighbour
            here_to_q = q->cost;
            q_to_p_cost = findById(&neighbours_dv_array[(q->id) - 1], p->id)->cost;
            if (min_cost > (here_to_q + q_to_p_cost)) {
                min_cost = here_to_q + q_to_p_cost;
                min_next_hop_id = q->id;
            }
            q = q->next;
        }
//        if (p->cost != min_cost)
//            is_changed = 1;
        p->cost = min_cost;
        p->next_hop_id = min_next_hop_id;
        p = p->next;
    }
}



int disable_neighbour(int neighbour_id) {
    return removeFromServerNodeList(&neighbour_list,neighbour_id);
}