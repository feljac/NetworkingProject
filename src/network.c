//
// Created by root on 16/10/18.
//

#include "network.h"


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

const char * real_address(const char *address, struct sockaddr_in6 *rval){
    struct addrinfo* hints = (struct addrinfo*) calloc(1, sizeof(struct addrinfo));
    struct addrinfo* res;
    hints->ai_family = AF_INET6;
    hints->ai_socktype = SOCK_DGRAM;
    hints->ai_protocol = 0;


    if ((getaddrinfo(address, NULL, hints, &res)) != 0) {
        return ("Getaddrinfo Error\n");
    }

    while(res){
        if(res->ai_family == AF_INET6){
            //rval = (struct sockaddr_in6*) res[0].ai_addr;
            memcpy(rval, (struct sockaddr_in6*)res->ai_addr, sizeof(*rval));
            rval->sin6_family = AF_INET6;
        }
        res = res->ai_next;
        return NULL;
    }
    free(hints);
    return "No address found";
}

int create_socket(struct sockaddr_in6 *source_addr, int src_port, struct sockaddr_in6 *dest_addr, int dst_port){
    int fd;

    if(!source_addr && !dest_addr){
        return -1;
    }

    if((fd = socket(PF_INET6, SOCK_DGRAM, 0)) == -1) {
        fprintf(stderr, "Socket Error");
        return -1;
    }

    struct sockaddr_in6 address;

    if(source_addr && src_port > 0){
        memcpy(&address, source_addr, sizeof(address));
        address.sin6_port = htons(src_port);
        if(bind(fd, (struct sockaddr *) &address, sizeof(address)) == -1){
            fprintf(stderr, "Bind Error%s\n", strerror(errno));
            return -1;
        }
    }
    else if(dest_addr && dst_port > 0) {
        memcpy(&address, dest_addr, sizeof(address));
        address.sin6_port = htons(dst_port);
        if (connect(fd, (struct sockaddr*) &address, sizeof(address)) == -1){
            fprintf(stderr, "Connect Error %s\n", strerror(errno));
            return -1;
        }
    }
    else{
        return -1;
    }

    return fd;
}

int wait_for_client(int sfd){
    char* buf[1024];
    struct sockaddr_in6* src_addr =(struct sockaddr_in6*) calloc(1, sizeof(struct sockaddr_in6));
    socklen_t sendsize = sizeof(*src_addr);

    fprintf(stderr,"Server waiting\n");

    if((recvfrom(sfd, buf, sizeof(buf), MSG_PEEK, (struct sockaddr*) src_addr, &sendsize)) == -1){
        fprintf(stderr, "Error wait recvfrom\n");
        return -1;
    }

    fprintf(stderr,"Server connect...\n");

    if(connect(sfd, (struct sockaddr*) src_addr, sendsize) == -1){
        fprintf(stderr, "Error wait connect\n");
        return -1;
    }
    free(src_addr);

    fprintf(stderr,"Server connected\n");
    return 0;
}