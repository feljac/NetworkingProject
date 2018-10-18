//
// Created by root on 16/10/18.
//

#include "packet_interface.h"
#include "sorted_queue.h"
#include "network.h"
#include <unistd.h>
#include <stdint.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <stdio.h>
#include "zlib-1.2.11/zlib.h"
#include <netinet/in.h>

#ifndef NETWORKING_PROJECT_UTILS_H
#define NETWORKING_PROJECT_UTILS_H

#define MAX_SEQNUM 256
#define PKT_LENGTH sizeof(pkt_t)

int compare_seqnum(uint8_t a, uint8_t b);

void next_seqnum(uint8_t* seqnum);

int sorted_queue_compare_seqnum(uint8_t a, uint8_t b);

char** get_file_by_name(int argc, char** argv);

#endif //NETWORKING_PROJECT_UTILS_H
