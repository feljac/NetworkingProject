//
// Created by root on 16/10/18.
//
#include "sender.h"

int main(int argc, char** argv){

    int port;
	char *host;
    FILE* file;
    char* filename = NULL;
    list_pkt list_pkts;
    int is_file = 0;
    int c;

     if (argc >= 4) {
        while ((c = getopt(argc, argv, "f:")) != -1) {
            switch (c) {
                case 'f':
                    if((filename = malloc(strlen(optarg)+1)) == NULL){
                        fprintf(stderr,"erreur malloc filename\n");
                        exit(EXIT_FAILURE);
                    }
                    memcpy(filename, optarg, strlen(optarg));
                    filename[strlen(optarg)] = '\0';
                    is_file = 1;
                    break;
                default:
                    fprintf(stderr, "Usage : receiver < -f filename > [ address ] [ port ]\n");
                    return EXIT_FAILURE;
            }
        }
    }

    argc -= optind;
    argv += optind;

    if (argc < 2) {
        fprintf(stderr, "Usage : receiver < -f filename > [ address ] [ port ]\n");
        return EXIT_FAILURE;
    }
    if(is_file && access(filename, F_OK ) != -1 ){
        if((file = fopen(filename,"r")) == NULL){
            fprintf(stderr,"error: open file \n");
            exit(EXIT_FAILURE);
        }
        free(filename);
    }else{
        file = stdin;
    }
    port = atoi(argv[1]);
    host = argv[0];

    // Initiation buffer
    if(!init_list(256,&list_pkts)){
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
    exit(EXIT_SUCCESS);
}

void read_write_loop(const int sfd,FILE* file, list_pkt * list_pkts ){

    int window[256]={0};
    uint8_t debutWindow = 0;
    int actual_size_window = MAX_WINDOW_SIZE;
    int lastPacketSend = 0;
    int nbPacketSend  = 0;
    uint8_t seqNum = 0;
    size_t headerLength;
    int fileDescriptor;
    if((fileDescriptor = fileno(file)) == -1){
        fprintf(stderr, "error with transformation FILE* into file descriptor\n");
        return;
    }
    fprintf(stderr, "file descriptor: %d\n",fileDescriptor);
    int rto = INIT_RTO;
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
        check_retransmission_time_out(*list_pkts,debutWindow,actual_size_window,sfd,rto);
        ret = poll(fds, 2, rto);
        if(ret > 0){
            if (fds[1].revents & POLLIN) {
                fprintf(stderr,"Lecture socket\n");
                pkt_receive = pkt_new();
                headerLength = sizeof(*pkt_receive);
                char buffIn[headerLength];
                if((int)(toReturn = read(sfd,buffIn,headerLength)) == -1){
                    fprintf(stderr, "Error read");
                }
                // decode header read
                if(pkt_decode(buffIn,headerLength,pkt_receive) == E_CRC){
                    continue;
                }
                if(pkt_get_type(pkt_receive) == PTYPE_ACK){
                    fprintf(stderr,"ACK\n");
                    int ancienDebutWindow = debutWindow;
                    int  nbAckRecive;
                    if((nbAckRecive = check_window_sequence_and_delete_packet(window,&debutWindow,pkt_receive->header.seqnum,list_pkts)) > 0){
                        nbPacketSend -=  nbAckRecive;
                        actual_size_window = pkt_get_window(pkt_receive);
                        fprintf(stderr,"sequence number to delete [%d -> %d[\n",ancienDebutWindow, debutWindow);
                        fprintf(stderr,"nbpacket send last: %d -> %d",lastPacketSend,nbPacketSend);
                        /* three way
                        if(lastPacketSend && (nbPacketSend == 0)) break;*/
                    }
                }else{
                    uint8_t pkt_to_resend = pkt_receive->header.seqnum;
                    pkt_t* pkt = get_packet_to_index(pkt_to_resend,*list_pkts);
                    size_t length_pkt_to_resend = sizeof(*pkt);
                    char  to_resend[length_pkt_to_resend];
                    if (pkt_encode(pkt,to_resend,&length_pkt_to_resend) != PKT_OK){
                        fprintf(stderr,"error encode pkt recive\n");
                    }
                    if((int)write(sfd,to_resend,length_pkt_to_resend) == -1){
                        fprintf(stderr, "Error write");
                    }
                    fprintf(stderr,"receive NACK send all data with no ACK \n");
                }
            }
            if (fds[0].revents & POLLIN) {
                if( nbPacketSend <= actual_size_window  && !lastPacketSend){
                    toReturn = 0;
                    fprintf(stderr,"Lecture stdin\n");
                    pkt_send = (pkt_t *)pkt_new();
                    pkt_set_type(pkt_send,PTYPE_DATA);
                    pkt_set_tr(pkt_send,0);
                    toReturn = read(fileDescriptor,bufOut,MAX_PAYLOAD_SIZE);
                    fprintf(stderr,"read -> %ld  \n",toReturn);
                    // last packet send
                    if(toReturn == 0){
                        fprintf(stderr,"EOF stdin\n");
                        lastPacketSend = 1;
                        if(seqNum == 0){
                            seqNum = 255;
                        }else{
                            seqNum -= 1;
                        }
                        pkt_set_seqnum(pkt_send,seqNum);
                        pkt_set_length(pkt_send,0);
                    }else{
                        pkt_set_length(pkt_send,toReturn);
                        pkt_set_payload(pkt_send,bufOut,toReturn);
                        pkt_set_seqnum(pkt_send,seqNum);
                    }
                    pkt_set_window(pkt_send,actual_size_window);
                    pkt_set_timestamp(pkt_send,(unsigned)time(NULL));
                    size_t length_pkt = sizeof(*pkt_send)+pkt_get_length(pkt_send);
                    char  to_send[length_pkt];
                    if(pkt_encode(pkt_send,to_send,&length_pkt) != PKT_OK){
                        fprintf(stderr,"error encode pkt send\n");
                    }
                    window[seqNum]= WAIT_ACK;
                    add_packet_to_index(seqNum,pkt_send,list_pkts);
                    next_seqnum(&seqNum);
                    int writed ;
                    if((writed = write(sfd,to_send,length_pkt)) == -1){
                        fprintf(stderr, "Error write");
                        break;
                    }
                    nbPacketSend++;
                    fprintf(stderr,"Lecture stdin -> sent to server  seq: %d  -> %ld -> %d bytes\n",pkt_get_seqnum(pkt_send),length_pkt, writed);
                }            
            }
        }
    }
}
// return the number of ACK receive
int check_window_sequence_and_delete_packet(int* window,uint8_t* debutWindow,uint8_t index, list_pkt* list){
    if(compare_seqnum(*debutWindow,index) > MAX_WINDOW_SIZE -1 || compare_seqnum(*debutWindow,index) <= 0 ) return 0;
    uint8_t i = *debutWindow;
    int compteur = 0;
    while(i != index){
        window[i] = NOT_DEFINE;
        delete_pkt_to_index(i,list);
        next_seqnum(&i);
        compteur++;
    }
    *debutWindow = i;
    return compteur;
}
void delete_all_list(list_pkt* list){
    for(pkt_t** pkts = list->pkts;pkts - list->pkts < MAX_WINDOW_SIZE;pkts++){
        if(*pkts != NULL) free(*pkts);
    }
    free(list->pkts);
}
void check_retransmission_time_out(list_pkt list,uint8_t debutWindow,int actual_size_window,const int sfd, int rto){
    uint8_t seqNum = debutWindow;
    pkt_t* pkt ;
    uint8_t compteur = 0;
    while(compteur < actual_size_window){
        if((pkt = get_packet_to_index(seqNum,list)) != NULL){
            if(difftime(time(NULL),pkt_get_timestamp(pkt)) > rto){
            size_t length_pkt_to_resend = sizeof(*pkt);
            char  to_resend[length_pkt_to_resend];
            pkt_encode(pkt,to_resend,&length_pkt_to_resend);
                if((int)write(sfd,to_resend,length_pkt_to_resend) == -1){
                    fprintf(stderr, "Error write");
                } 
            }
        }
        next_seqnum(&seqNum);
        compteur++;
    }
}