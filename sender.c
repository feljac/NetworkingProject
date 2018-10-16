//
// Created by root on 16/10/18.
//
#include "sender.h"

int main(int argc, char** argv){

    int port;
	char *host;
    FILE* file;
    char** fileName;
   /* pkt_t ** windows;
    int * window_seq_num;
    int taillePhysiqueWindow;
*/
    if(argc == 1 || argc > 4){
        fprintf(stderr,"need or a lot of arguments\n");
        exit(EXIT_FAILURE);
    }
    if(argc < 4){
        fileName = (char**) get_file_by_name(argc,argv);
        if((file = fopen(*fileName,"r")) == NULL){
            fprintf(stderr,"error: open file \n");
            exit(EXIT_FAILURE);
        }
        port = atoi(argv[0]);
        host = argv[1];

    }else{
        port = atoi(argv[1]);
        host = argv[2];
        file = stdin;
    }
    struct sockaddr_in6 addr;
	const char *err = real_address(host, &addr);
	if (err) {
		fprintf(stderr, "Could not resolve hostname %s: %s\n", host, err);
		return EXIT_FAILURE;
	}
	/* Get a socket */
	int sfd = create_socket(NULL, -1, &addr, port); /* Connected */
    read_write_loop(sfd, file);
}
char** get_file_by_name(int argc, char** argv){
    int opt;
    if ((opt = getopt(argc, argv, "f:")) != -1) {
            return &optarg;
    }
    return NULL;
}
void read_write_loop(const int sfd,FILE* file){

    int fileDescriptor;
    if((fileDescriptor = fileno(file)) == -1){
        fprintf(stderr, "error with transformation FILE* into file descriptor\n");
        return;
    }
    struct pollfd fds[2];
    /* Open STREAMS device. */
    fds[0].fd = fileDescriptor;
    fds[1].fd =sfd;
    fds[0].events = POLLIN;
    fds[1].events = POLLIN;
    size_t toReturn;
    pkt_t* pkt_send;
    pkt_t* pkt_receive;
    size_t bufSize = MAX_PAYLOAD_SIZE;
    int compareBuf = (int)bufSize;
    char bufOut[bufSize+1];
    int ret;
    int i;
    fprintf(stderr,"read/write\n");
    while(1){
        ret = poll(fds, 2, -1);
        if(ret > 0){
            if (fds[1].revents & POLLIN) {
                fprintf(stderr,"Lecture socket\n");
                pkt_receive = pkt_new();
                size_t headerLength = sizeof(pkt_receive->header);
                char buffIn[headerLength];
                if((int)(toReturn = read(sfd,buffIn,headerLength)) == -1){
                    fprintf(stderr, "Error read");
                }
                if(pkt_get_type(pkt_receive) == PTYPE_ACK){
                    // move window and send to server if the window is move
                    
                }else{
                    // resend all paquets with that not receive ack
                    fprintf(stderr,"receive NACK send all data with no ACK \n ");
                }
                
            }
            if (fds[0].revents & POLLIN) {
                // Init tous et modification par apres seqNum et Timestamp
                pkt_send = (pkt_t *)pkt_new();
                pkt_set_type(pkt_send,PTYPE_DATA);
                pkt_set_tr(pkt_send,0);
                pkt_set_seqnum(pkt_send,0);
                pkt_set_window(pkt_send,0);
                pkt_set_timestamp(pkt_send,32);                
                toReturn = 0;
                fprintf(stderr,"Lecture stdin\n");
                for(i = 0; i < compareBuf; i++) {
                    if((int)fread(bufOut + i, 1, 1, file) == -1){
                        fprintf(stderr, "Error when I read in stdin");
                    }
                    if(feof(stdin)){
                        fprintf(stderr,"EOF stdin\n");
                        return;
                    }
                    toReturn++;
                    if(bufOut[i] == '\n'){
                        if(bufOut[i-1] == '\r'){
                        bufOut[i-1] = '\n';
                        bufOut[i] = '\0';
                        }
                        break;
                    }
                    if(bufOut[i] == '\r'){
                        bufOut[i] = '\n';
                        break;
                    }
                }
                pkt_set_length(pkt_send,toReturn);
                pkt_set_payload(pkt_send,bufOut,toReturn);
                size_t length_pkt = sizeof(*pkt_send);
                char  to_send[length_pkt];
                pkt_encode(pkt_send,to_send,&length_pkt);
                if((int)write(sfd,to_send,length_pkt) == -1){
                    fprintf(stderr, "Error write");
                }
                fprintf(stderr,"Lecture stdin -> sent to server %d bytes\n", (int)strlen(to_send));

            }
        }
    }
}
        
