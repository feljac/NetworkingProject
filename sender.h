//
// Created by root on 16/10/18.
//
#include "list_packet.h"
#include "network.h"
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <getopt.h>

#ifndef NETWORKING_PROJECT_SENDER_H
#define NETWORKING_PROJECT_SENDER_H

//Value for window
#define WAIT_ACK 2 
#define ACK 1
#define NOT_DEFINE 0

// retransmition timeout
#define INIT_RTO 2000
#define MAX_COMPTEUR_RTO 50


/**
* read in the file and write in the socket
* @pre: sfd!= NULL; file != NULL, list != NULL
*/
void read_write_loop(const int sfd, FILE* file,list_pkt* list);

/**
* check if we must move the window and delete packets of my list if my window move
* @pre: window != NULL, debutWindow != NULL, index r!= NULL,list != NULL
* @post: if(debutwindow == index) 
* @return: number of sequence move else 0
*/
int check_window_sequence_and_delete_packet(int* window, uint8_t* debutWindow,uint8_t index,list_pkt* list);

/**
* delete all packets in my list
* @pre: list != NULL
*/
void delete_all_list(list_pkt* list);

/**
* Initialise un enregistrement
* @pre: r!= NULL
* @post: record_get_type(r) == 0 && record_get_length(r) == 0
*		 && record_has_footer(r) == 0
* @return: 1 en cas d'erreur, 0 sinon
*/
int check_retransmission_time_out(list_pkt*,uint8_t,int, const int,int,int*);

/**
* read my file and add packets in my list
* @pre:  list != NULL, fileDescriptor != NULL ,sfd != NULL, actual_size_window != NULL,
*        lastPacketSend != NULL, nbPacketSend != NULL,
*        lastSeqNumSend, seqNum != NULL, window, int* compteurRTO, int* rto
* @post: 
* @return: 1 en cas d'erreur, 0 sinon
*/
int read_data_and_fill_list(list_pkt* list, int fileDescriptor,int sfd, int actual_size_window,int* lastPacketSend, int* nbPacketSend, uint8_t* lastSeqNumSend, uint8_t* seqNum, int* window, int* compteurRTO, int* rto);

#endif //NETWORKING_PROJECT_SENDER_H
