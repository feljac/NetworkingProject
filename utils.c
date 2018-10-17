//
// Created by root on 16/10/18.
//

#include "utils.h"

int compare_seqnum(uint8_t a, uint8_t b){
    if(a > b){
        return MAX_SEQNUM - a + b;
    }
    else{
        return b - a;
    }
}


void next_seqnum(uint8_t* seqnum){
    if(seqnum == MAX_SEQNUM){
        *seqnum = 0;
    }
    else{
        (*seqnum)++;
    }
}

uint32_t generate_crc1(pkt_t * pkt){
    uLong crc;
    uint8_t tr = pkt_get_tr(pkt);
    pkt_set_tr(pkt, 0);
    crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, (Bytef*) pkt, sizeof(pkt->header) - sizeof(uint32_t));
    pkt_set_tr(pkt, tr);
    return htonl((uint32_t)crc);
}

uint32_t generate_crc2(pkt_t * pkt){
    uLong crc;
    crc = crc32(crc, (Bytef*) pkt->payload, pkt_get_length(pkt));
    return htonl((uint32_t)crc);
}