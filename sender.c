//
// Created by root on 16/10/18.
//
#include "sender.h"

int main(int argc, char** argv){

    int port;
	char *host;
    FILE* file;
    char** fileName;
    list_pkt list_pkts;
    

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
    // Initiation buffer
    if(!init_list(MAX_WINDOW_SIZE,&list_pkts)){
        fprintf(stderr,"erreur init buffer packets\n");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in6 addr;
	const char *err = real_address(host, &addr);
	if (err) {
		fprintf(stderr, "Could not resolve hostname %s: %s\n", host, err);
		return EXIT_FAILURE;
	}
	/* Get a socket */
	int sfd = create_socket(NULL, -1, &addr, port); /* Connected */
    read_write_loop(sfd, file,&list_pkts);

    // free liste and close file
    if(file != stdin) fclose(file);
    delete_all_list(&list_pkts);
}
char** get_file_by_name(int argc, char** argv){
    int opt;
    if ((opt = getopt(argc, argv, "f:")) != -1) {
            return &optarg;
    }
    return NULL;
}
void read_write_loop(const int sfd,FILE* file, list_pkt * list_pkts ){

    int window[255]={0};
    uint32_t debutWindow = 0;
    int actual_size_window = MAX_WINDOW_SIZE;
    int lastPacketSend = 0;
    int nbPacketSend  = 0;
    uint8_t seqNum = 0;
    size_t headerLength;
    int indexReceive = 0;
    int indexSend = 0;
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
    char bufOut[bufSize];
    int ret;
    fprintf(stderr,"read/write\n");
    while(1){
        ret = poll(fds, 2, -1);
        if(ret > 0){
            if (fds[1].revents & POLLIN) {
                fprintf(stderr,"Lecture socket\n");
                pkt_receive = pkt_new();
                headerLength = sizeof(pkt_receive->header);
                char buffIn[headerLength];
                if((int)(toReturn = read(sfd,buffIn,headerLength)) == -1){
                    fprintf(stderr, "Error read");
                }
                // decode header read
                pkt_decode(buffIn,headerLength,pkt_receive);
                if(pkt_get_type(pkt_receive) == PTYPE_ACK){
                    // TODO Calculer l'index
                    indexReceive = 0;
                    debutWindow = check_window_sequence(window,debutWindow,indexReceive,actual_size_window);
                    actual_size_window = pkt_get_window(pkt_receive);
                    fprintf(stderr,"sequence number to delete %d\n",pkt_receive->header.seqnum);
                    delete_pkt_to_index( indexReceive,list_pkts);
                    nbPacketSend--;
                    if(lastPacketSend && (nbPacketSend == 0)) break;
                }else{
                    int pkt_to_resend = pkt_receive->header.seqnum;
                    pkt_t* pkt = get_packet_to_index(pkt_to_resend,*list_pkts);
                    size_t length_pkt_to_resend = sizeof(*pkt);
                    char  to_resend[length_pkt_to_resend];
                    pkt_encode(pkt,to_resend,&length_pkt_to_resend);
                if((int)write(sfd,to_resend,length_pkt_to_resend) == -1){
                    fprintf(stderr, "Error write");
                }
                fprintf(stderr,"receive NACK send all data with no ACK \n ");
                }
                
            }
            if (fds[0].revents & POLLIN) {
                // TODO Add timer      
                if(actual_size_window != 0 || lastPacketSend){
                    toReturn = 0;
                    fprintf(stderr,"Lecture stdin\n");
                    pkt_send = (pkt_t *)pkt_new();
                    pkt_set_type(pkt_send,PTYPE_DATA);
                    pkt_set_tr(pkt_send,0);
                    toReturn = fread(bufOut, 1,MAX_PAYLOAD_SIZE, file);
                    // last packet send
                    if(feof(stdin)){
                        fprintf(stderr,"EOF stdin\n");
                        lastPacketSend = 1;
                        if(seqNum == 0) seqNum = 255;
                        pkt_set_length(pkt_send,0);
                    }else{
                        pkt_set_length(pkt_send,toReturn);
                        pkt_set_payload(pkt_send,bufOut,toReturn);
                    }
                    pkt_set_seqnum(pkt_send,seqNum);
                    next_seqnum(&seqNum);
                    pkt_set_window(pkt_send,actual_size_window);
                    pkt_set_timestamp(pkt_send,(unsigned)time(NULL));
                    size_t length_pkt = sizeof(*pkt_send);
                    char  to_send[length_pkt];
                    pkt_encode(pkt_send,to_send,&length_pkt);
                    if((int)write(sfd,to_send,length_pkt) == -1){
                        fprintf(stderr, "Error write");
                    }
                    nbPacketSend++;
                    // TODO ADD IN LIST 
                    indexSend = 0;
                    window[indexSend]= WAIT_ACK;
                    add_packet_to_index(indexSend,pkt_send,list_pkts);
                    fprintf(stderr,"Lecture stdin -> sent to server %d bytes\n", (int)strlen(to_send));
                }            
            }
        }
    }
}
uint8_t check_window_sequence(int* window,uint8_t debutWindow,uint8_t index, int actual_size_window){
    window[index] = ACK;   
    if(index == debutWindow){
        next_seqnum(&debutWindow);
    }
    uint8_t i = debutWindow;
    int compteur = 0;
    while(window[i] != WAIT_ACK){
        window[i] = NOT_DEFINE;
        next_seqnum(&i);
        if(compteur == actual_size_window) break; 
        compteur++;
    }
    return i;
}
void delete_all_list(list_pkt* list){
    for(pkt_t** pkts = list->pkts;pkts - list->pkts < MAX_WINDOW_SIZE;pkts++){
        if(*pkts != NULL) free(*pkts);
    }
    free(list->pkts);
}