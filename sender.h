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

void read_write_loop(const int sfd, FILE* file,list_pkt* list);
int check_window_sequence_and_delete_packet(int* window, uint8_t* debutWindow,uint8_t index,list_pkt* list);
void delete_all_list(list_pkt* list);
int check_retransmission_time_out(list_pkt*,uint8_t,int, const int,int,int*);
int read_data_and_fill_window(list_pkt*, int fileDescriptor,int sfd, int actual_size_window,int* lastPacketSend, int* nbPacketSend, uint8_t* lastSeqNumSend, uint8_t* seqNum, int* window);

#endif //NETWORKING_PROJECT_SENDER_H
