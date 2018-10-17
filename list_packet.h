//
// Created by root on 16/10/18.
//

#ifndef LISTE_PACKET_H
#define LISTE_PACKET_H

#include "utils.h"

//Value for window
#define WAIT_ACK 2 
#define ACK 1
#define NOT_DEFINE 0

typedef struct list_pkt{
    pkt_t ** pkts;
} list_pkt;

int init_list(int sizeListe, list_pkt* liste);
pkt_t* get_packet_to_index(int index_pkt, list_pkt liste);
void  add_packet_to_index(int index,pkt_t* pkt, list_pkt* liste);
void delete_pkt_to_index(int index_pkt, list_pkt* liste);

#endif