//
// Created by root on 16/10/18.
//

#include "packet_interface.h"
#include "sorted_queue.h"
#include <unistd.h>
#include <stdint.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <stdio.h>
#include "zlib-1.2.11/zlib.h"
#include <netinet/in.h>

#ifndef NETWORKING_PROJECT_UTILS_H
#define NETWORKING_PROJECT_UTILS_H

#define MAX_SEQNUM 255
#define PKT_LENGTH sizeof(pkt_t)

int compare_seqnum(uint8_t a, uint8_t b);

void next_seqnum(uint8_t* seqnum);

uint32_t generate_crc1(pkt_t * pkt);

int sorted_queue_compare_seqnum(uint8_t a, uint8_t b);

void write_in_file(FILE* file, pkt_t* pkt);

#endif //NETWORKING_PROJECT_UTILS_H
