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

int sorted_queue_compare_seqnum(uint8_t a, uint8_t b){
    if(a > 226 && b <= 29){
        return 1;
    }
    else if(b > 226 && a <= 29){
        return 0;
    }
    else{
        return a < b;
    }
}

char** get_file_by_name(int argc, char** argv){
    if ((getopt(argc, argv, "f:")) != -1) {
        return &optarg;
    }
    return NULL;
}