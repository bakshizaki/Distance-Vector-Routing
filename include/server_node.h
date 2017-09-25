/*
 * server_node.h
 *
 *  Created on: 19-Oct-2016
 *      Author: zaki
 */

#ifndef SERVER_NODE_H_
#define SERVER_NODE_H_
#define INF 32767


typedef struct server_node{
	int id;
	char *ip_address;
	char *port;
	int cost;
	int next_hop_id;
	struct server_node *next;
	int alive_counter;
}server_node;

void addToServerNodeList(server_node **list, int id, char *ip_address, char *port, int cost);
int removeFromServerNodeList(server_node **list, int id);
server_node * findById(server_node **list, int id);
void printServerNodeList(server_node **list);
server_node * findByIPPort(server_node **list, char *ip_address, char *port);
#endif /* SERVER_NODE_H_ */
