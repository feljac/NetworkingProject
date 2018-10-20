
#include "list_packet.h"

 int init_list(int sizeListe,list_pkt* list){
    if((list->pkts = (pkt_t**) malloc(sizeListe*sizeof(pkt_t*))) == NULL){
        fprintf(stderr,"error with creation buffer \n");
        return 0;
    }
    for(int i = 0;i < sizeListe;i++){
        *(list->pkts+i) = NULL;
    }
    return 1;
}
pkt_t* get_packet_to_index(int index_pkt, list_pkt list){
    return *(list.pkts+index_pkt);
}
void delete_pkt_to_index(int index_pkt, list_pkt* list){
    fprintf(stderr,"seqNum to delete %d\n",(*(list->pkts+index_pkt))->header.seqnum);
    if((*(list->pkts+index_pkt)) == NULL){
        fprintf(stderr,"packet is already deleted\n");
        return;
    }
    free((*(list->pkts+index_pkt)));
    (*(list->pkts+index_pkt)) = NULL;
}
void  add_packet_to_index(int index,pkt_t* pkt, list_pkt* list){
    *(list->pkts+index) = pkt;
}
