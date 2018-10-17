//
// Created by root on 16/10/18.
//

#ifndef NETWORKING_PROJECT_SENDER_H
#define NETWORKING_PROJECT_SENDER_H

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/poll.h>
#include <unistd.h>
#include <getopt.h>
#include "packet_interface.h"
#include "network.h"
#include <string.h>

//Value for window
#define WAIT_ACK 2 
#define ACK 1
#define NOT_DEFINE 0

char** get_file_by_name(int argc, char** argv);
void read_write_loop(const int sfd, FILE* file);

#endif //NETWORKING_PROJECT_SENDER_H
