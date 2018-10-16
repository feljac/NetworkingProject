//
// Created by root on 16/10/18.
//

#include "sender.h"
#include "packet_interface.h"
#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

int main(int argc, char** argv){

    int port;
	char *host;
    FILE* file;
    char** fileName;
 //   char buffer[MAX_PAYLOAD_SIZE];
  //  pkt_t * windows;

    if(argc == 1 || argc > 4){
        fprintf(stderr,"trop ou pas  arguments\n");
        exit(EXIT_FAILURE);
    }
    if(argc < 4){
        fileName = (char**) get_file_by_name(argc,argv);
        if((file = fopen(*fileName,"r")) == NULL){
            fprintf(stderr,"erreur lors de l'ouverture du fichier \n");
            exit(EXIT_FAILURE);
        }
        port = atoi(argv[2]);
        host = argv[3];

    }else{
        port = atoi(argv[1]);
        host = argv[2];
        file = 0;
    }
    fprintf(stderr,"port: %d", port);
    struct sockaddr_in6 addr;
	const char *err = real_address(host, &addr);
	if (err) {
		fprintf(stderr, "Could not resolve hostname %s: %s\n", host, err);
		return EXIT_FAILURE;
	}
	/* Get a socket */
//	int sfd = create_socket(NULL, -1, &addr, port); /* Connected */
}
char** get_file_by_name(int argc, char** argv){
    int opt;
    if ((opt = getopt(argc, argv, "f:")) != -1) {
            return &optarg;
    }
    return NULL;
}
        
