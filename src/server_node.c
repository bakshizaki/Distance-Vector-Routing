/*
 * server_node.c
 *
 *  Created on: 19-Oct-2016
 *      Author: zaki
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include "common_methods.h"

#include"server_node.h"

void addToServerNodeList(server_node **list, int id, char *ip_address, char *port, int cost) {
	server_node *p;
	server_node *temp_node = (server_node *) calloc(1,sizeof(server_node));

	temp_node->id = id;
	if (ip_address != NULL) {
		temp_node->ip_address = (char *) calloc((strlen(ip_address)+1) , sizeof(char));
		strcpy(temp_node->ip_address, ip_address);
	}
	if (port != NULL) {
		temp_node->port = (char *) calloc((strlen(port)+1) , sizeof(char));
		strcpy(temp_node->port, port);
	}
	temp_node->cost = cost;
	temp_node->next = NULL;
	temp_node->next_hop_id=0;
	temp_node->alive_counter=0;
	if (*list == NULL) {
		*list = temp_node;
		return;
	}
	p = *list;
	while (p->next != NULL) {
		p = p->next;
	}
	p->next = temp_node;


}

int removeFromServerNodeList(server_node **list, int id) {
	server_node *prev_node, *current_node, *temp_node;
	prev_node = NULL;
	current_node = *list;

	if (current_node->id == id) {
		temp_node = current_node;
		*list = current_node->next;
		free(temp_node);
		return 1;
	}

	while (current_node != NULL) {
		if (current_node->id == id)
			break;
		prev_node = current_node;
		current_node = current_node->next;
	}
	if (current_node==NULL) {
		if(DEBUG) printf("Could not find element\n");
		return 0;
	}
	temp_node = current_node;
	prev_node->next = current_node->next;
	free(temp_node);
	return 1;
}

server_node * findById(server_node **list, int id) {
	server_node *current_node;
		current_node = *list;
	while (current_node != NULL) {
		if (current_node->id == id)
			break;
		current_node = current_node->next;
	}
	if (current_node == NULL) {
		if(DEBUG) printf("findById: Could not find element by id\n");
		return NULL;
	}
	return current_node;
}

server_node * findByIPPort(server_node **list, char *ip_address, char *port){
	server_node *current_node;
	current_node = *list;
	while (current_node != NULL) {
		if (strcmp(current_node->ip_address, ip_address) == 0 && strcmp(current_node->port, port) == 0)
			break;
		current_node = current_node->next;
	}
	if (current_node == NULL) {
		if(DEBUG) printf("findByIP: Could not find element by IP\n");
		return NULL;
	}
	return current_node;
}

void printServerNodeList(server_node **list) {
	server_node *current_node;
	current_node = *list;
	printf("Server ID  Cost  Next Hop\n");
	while(current_node!=NULL) {
		if(current_node->cost==INF) {
			printf("%-11d%-6s%-8d\n",current_node->id,"INF",current_node->next_hop_id);
		}
		else
		printf("%-11d%-6d%-8d\n",current_node->id,current_node->cost,current_node->next_hop_id);
		current_node = current_node->next;
	}
}
