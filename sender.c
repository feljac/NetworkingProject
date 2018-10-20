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
    fprintf(stderr,"delete buffer and close file\n");
    exit(EXIT_SUCCESS);
}

void read_write_loop(const int sfd,FILE* file, list_pkt * list_pkts ){

    int window[256]={0};
    uint8_t debutWindow = 0;
    int actual_size_window = MAX_WINDOW_SIZE;
    int lastPacketSend = 0;
    uint8_t lastSeqNumSend = 0;
    int lastCompteur = 0;
    int nbPacketSend  = 0;
    uint8_t seqNum = 0;
    size_t headerLength;
    int fileDescriptor;
    if((fileDescriptor = fileno(file)) == -1){
        fprintf(stderr, "error with transformation FILE* into file descriptor\n");
        return;
    }    int rto = INIT_RTO;
    struct pollfd fds[2];
    int compteurRTO = MAX_COMPTEUR_RTO;
    /* Open STREAMS device. */
    
    size_t toReturn;
    
    pkt_t* pkt_receive;
    int ret;
    int nbrFile = 1;
    fds[0].fd = sfd;
    fds[0].events = POLLIN;
    fprintf(stderr,"read/write\n");
    while(1){
        if(read_data_and_fill_list(list_pkts,fileDescriptor,sfd, actual_size_window, &lastPacketSend, &nbPacketSend, &lastSeqNumSend, &seqNum, window, &compteurRTO, &rto)){
            break;
        }
        ret = poll(fds, nbrFile, rto);
        if(ret > 0){
            if (fds[0].revents & POLLIN) {
                fprintf(stderr,"\nRead socket\n");
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
                    fprintf(stderr,"receive an ACK -> %d\n",pkt_get_seqnum(pkt_receive));
                    compteurRTO = MAX_COMPTEUR_RTO;
                    int ancienDebutWindow = debutWindow;
                    int  nbAckReceive;
                    fprintf(stderr,"actual windows size %d ACK pkt windows size : %d\n",actual_size_window,pkt_get_window(pkt_receive));
                    actual_size_window = pkt_get_window(pkt_receive);

                    if((nbAckReceive = check_window_sequence_and_delete_packet(window,&debutWindow,pkt_get_seqnum(pkt_receive),list_pkts)) > 0){
                        nbPacketSend -=  nbAckReceive;
                        fprintf(stderr,"nb paquet deleted %d\n",nbAckReceive);
                        fprintf(stderr,"move sliding window [%d -> %d[\n",ancienDebutWindow, debutWindow);
                        if(lastPacketSend && (nbPacketSend == 0)) break;
                    }
                    if( lastSeqNumSend && pkt_get_seqnum(pkt_receive) == lastSeqNumSend){
                        lastCompteur++;
                        if(lastCompteur == 2 && debutWindow == pkt_get_seqnum(pkt_receive) ){
                            fprintf(stderr,"the last packet is received close connection\n");
                            break;
                        }
                    }
                    if(read_data_and_fill_list(list_pkts,fileDescriptor,sfd, actual_size_window, &lastPacketSend, &nbPacketSend, &lastSeqNumSend,&seqNum, window, &compteurRTO, &rto)){
                        break;
                    }
                }else{
                    fprintf(stderr,"receive an NACK -> %d\n", pkt_get_seqnum(pkt_receive));
                    compteurRTO = MAX_COMPTEUR_RTO;
                    uint8_t pkt_to_resend = pkt_get_seqnum(pkt_receive);
                    fprintf(stderr,"packet to resend : %d \n",pkt_to_resend);
                    pkt_t* pkt = get_packet_to_index(pkt_to_resend,*list_pkts);
                    pkt_set_timestamp(pkt,time(NULL));
                    actual_size_window = pkt_get_window(pkt_receive);
                    size_t length_pkt_to_resend = sizeof(*pkt)+pkt_get_length(pkt);
                    char  to_resend[length_pkt_to_resend];
                    if (pkt_encode(pkt,to_resend,&length_pkt_to_resend) != PKT_OK){
                        fprintf(stderr,"error encode pkt recive\n");
                    }
                    if((int)write(sfd,to_resend,length_pkt_to_resend) == -1){
                        fprintf(stderr, "Error write\n");
                    }
                    fprintf(stderr,"receive NACK send packet \n");
                }
            }
        } else if(ret == 0 && actual_size_window != 0){ // retransmission timeout
            if(lastPacketSend == 1){
               
            } 
            if(check_retransmission_time_out(list_pkts,debutWindow,actual_size_window,sfd,rto, &compteurRTO)){
                fprintf(stderr," number of packet sent : %d but no ACK/NACK -> close connection \n", MAX_COMPTEUR_RTO);
                break;
            }
        }
    }
}
// return the number of ACK receive
int check_window_sequence_and_delete_packet(int* window,uint8_t* debutWindow,uint8_t index, list_pkt* list){
    if(compare_seqnum(*debutWindow,index) > MAX_WINDOW_SIZE || compare_seqnum(*debutWindow,index) <= 0 ) return 0;
    uint8_t i = *debutWindow;
    if(i == index) return 0;
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
// return 1 if the compteurRto is 0
int check_retransmission_time_out(list_pkt *list,uint8_t debutWindow,int actual_size_window,const int sfd, int rto,int* compteurRto){
    if(*compteurRto == 0)  return 1;
    uint8_t seqNum = debutWindow;
    pkt_t* pkt ;
    uint8_t compteur = 0;
    while(compteur < actual_size_window){
        if((pkt = get_packet_to_index(seqNum,*list)) != NULL){
            if(difftime(time(NULL),pkt_get_timestamp(pkt))*1000 >= rto){
            size_t length_pkt_to_resend = sizeof(*pkt)+pkt_get_length(pkt);
            char  to_resend[length_pkt_to_resend];
            pkt_set_timestamp(*(list->pkts+seqNum),time(NULL));
            pkt_encode(pkt,to_resend,&length_pkt_to_resend);
                if((int)write(sfd,to_resend,length_pkt_to_resend) == -1){
                    fprintf(stderr, "Error write");
                }
                fprintf(stderr, "resend packet %d \n",seqNum); 
            }
        }
        next_seqnum(&seqNum);
        compteur++;
    }
    (*compteurRto)--;
    return 0;
}
int read_data_and_fill_list(list_pkt* list_pkts, int fileDescriptor, int sfd, int actual_size_window,int* lastPacketSend, int* nbPacketSend, uint8_t* lastSeqNumSend, uint8_t* seqNum, int* window, int* compteurRTO, int* rto){
        while( *nbPacketSend <= actual_size_window  && !*lastPacketSend){
                fprintf(stderr,"\nRead stdin\n");
                pkt_t* pkt_send = (pkt_t *)pkt_new();
                pkt_set_type(pkt_send,PTYPE_DATA);
                pkt_set_tr(pkt_send,0);
                char bufOut[MAX_PAYLOAD_SIZE];
                size_t toReturn = read(fileDescriptor,bufOut,MAX_PAYLOAD_SIZE);
                fprintf(stderr,"read -> %ld  \n",toReturn);
                // last packet send
                if(toReturn == 0){
                    fprintf(stderr,"EOF stdin\n");
                    *lastPacketSend = 1;
                    *lastSeqNumSend = *seqNum;   
                     *rto = 500;
                     *compteurRTO = 20;               
                    fprintf(stderr, "lastSeq %d\n",*lastSeqNumSend);
                    if(seqNum == 0){
                        *seqNum = 255;
                    }else{
                        *seqNum -= 1;
                    }
                    pkt_set_seqnum(pkt_send,*seqNum);
                    pkt_set_length(pkt_send,0);
                    next_seqnum(seqNum);
                }else{
                    pkt_set_length(pkt_send,toReturn);
                    pkt_set_payload(pkt_send,bufOut,toReturn);
                    pkt_set_seqnum(pkt_send,*seqNum);
                }
                pkt_set_window(pkt_send,actual_size_window);
                pkt_set_timestamp(pkt_send,(unsigned)time(NULL));
                size_t length_pkt = sizeof(*pkt_send)+pkt_get_length(pkt_send);
                char  to_send[length_pkt];
                if(pkt_encode(pkt_send,to_send,&length_pkt) != PKT_OK){
                    fprintf(stderr,"error encode pkt send\n");
                }
                window[*seqNum]= WAIT_ACK;
                add_packet_to_index(*seqNum,pkt_send,list_pkts);
                next_seqnum(seqNum);
                int writed ;
                if((writed = write(sfd,to_send,length_pkt)) == -1){
                    fprintf(stderr, "Error write\n");
                    return 1;
                }
                (*nbPacketSend)++;
                fprintf(stderr,"sent to server  seq: %d  -> %ld -> %d bytes\n",pkt_get_seqnum(pkt_send),length_pkt, writed);
                
            }   
        return 0;
}