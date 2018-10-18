//
// Created by root on 16/10/18.
//

#include "utils.h"

int compare_seqnum(uint8_t a, uint8_t b){
    if(a > b){
        return (MAX_SEQNUM - a) + b;
    }
    else{
        return b - a;
    }
}


void next_seqnum(uint8_t* seqnum){
    if(*seqnum + 1 == MAX_SEQNUM){
        *seqnum = 0;
    }
    else{
        (*seqnum)++;
    }
}

int sorted_queue_compare_seqnum(uint8_t a, uint8_t b){
    return (a + MAX_WINDOW_SIZE) % (MAX_SEQNUM + 1) < (b + MAX_WINDOW_SIZE) % (MAX_SEQNUM);
}

char** get_file_by_name(int argc, char** argv){
    if ((getopt(argc, argv, "f:")) != -1) {
        return &optarg;
    }
    return NULL;
}