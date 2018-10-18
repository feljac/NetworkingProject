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

char** get_file_by_name(int argc, char** argv);
void read_write_loop(const int sfd, FILE* file,list_pkt* list);
int check_window_sequence_and_delete_packet(int* window, uint32_t* debutWindow,uint32_t index,list_pkt* list);
void delete_all_list(list_pkt* list);

#endif //NETWORKING_PROJECT_SENDER_H
