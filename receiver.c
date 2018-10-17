//
// Created by root on 16/10/18.
//

#include "receiver.h"

void send_message(int socket, uint8_t tr,uint8_t last_seqnum, uint8_t window, uint32_t last_timestamp){
    pkt_t* pkt = pkt_new();
    if(tr == 1){
        fprintf(stderr, "Sending NACK");
        pkt_set_seqnum(pkt, PTYPE_NACK);
    }
    else{
        fprintf(stderr, "Sending ACK");
        pkt_set_seqnum(pkt, PTYPE_ACK);
    }
    pkt_set_window(pkt, window);
    pkt_set_timestamp(pkt, last_timestamp);
    pkt_set_crc1(pkt, generate_crc1(pkt));
    size_t length = PKT_LENGTH;
    char* buffer = malloc(PKT_LENGTH);
    if(pkt_encode(pkt, buffer, &length) != PKT_OK){
        fprintf(stderr, "Error encode message");
    }
    if(write(socket, buffer, length) == -1){
        fprintf(stderr, "Error write message");
    }
    fprintf(stderr, "Message sent");
    free(buffer);
    pkt_del(pkt);
}

void write_to_file(FILE* file, struct stack ** sorted_stack, uint8_t* last_seqnum_written, uint8_t* window){
    pkt_t* pkt = peek(sorted_stack);
    while(pkt && compare_seqnum(*last_seqnum_written, pkt_get_seqnum(pkt)) == 1){
        size_t length = PKT_LENGTH + pkt_get_length(pkt);
        char* buffer = malloc(PKT_LENGTH + pkt_get_length(pkt));
        if(pkt_encode(pkt, buffer, &length) != PKT_OK){
            fprintf(stderr, "Error encode message");
        }
        fwrite(buffer, length, 1, file);
        *last_seqnum_written = pkt_get_seqnum(pkt);
        (*window)++;
        pop(sorted_stack);
        pkt_del(pkt);
        pkt = peek(sorted_stack);
    }
}

void receive_data_from_socket(FILE* file, int socket){
    int is_receiving = 1;
    int something_received = 0;
    ssize_t read_size = 0;
    uint8_t  tr = 0;
    uint8_t min_seqnum_received = 0;
    uint8_t last_seqnum_written = 0;
    uint8_t window = MAX_WINDOW_SIZE - 1;
    uint32_t last_timestamp = 0;
    char buffer[sizeof(pkt_t) + MAX_PAYLOAD_SIZE];
    pkt_t* pkt;
    struct stack* sorted_stack;
    init_stack(&sorted_stack);

    struct pollfd fds[1] = {{socket, POLLIN |POLLPRI | POLLOUT, 0}};

    while(is_receiving){
        if(poll(fds, 1, -1) == -1){
            fprintf(stderr, "Error poll");
            continue;
        }
        if(fds[0].revents & (POLLPRI | POLLIN)){

            if((read_size = read(socket, buffer, sizeof(pkt_t) + MAX_PAYLOAD_SIZE)) == -1){
                fprintf(stderr, "Error read on socket");
                continue;
            }

            pkt = pkt_new();

            /*receive data and put it in a pkt*/
            if(pkt_decode(buffer, (size_t)read_size, pkt) != PKT_OK){
                fprintf(stderr, "Packet corupted");
                pkt_del(pkt);
            }
            else if(pkt->header.type != PTYPE_DATA){
                fprintf(stderr, "Not a data pkt");
                pkt_del(pkt);
            }
            else if(window == 0){
                fprintf(stderr, "window is full");
                pkt_del(pkt);
            }
            else if(compare_seqnum(last_seqnum_written, pkt_get_seqnum(pkt)) > MAX_WINDOW_SIZE - 1 || compare_seqnum(last_seqnum_written, pkt_get_seqnum(pkt)) <= 0){
                fprintf(stderr, "Seqnum out of window");
                pkt_del(pkt);
            }
            /*if last packet stop receiving*/
            else if(pkt_get_length(pkt) == 0){
                fprintf(stderr, "End of data");
                if(last_seqnum_written == pkt_get_seqnum(pkt)){
                    is_receiving = 0;
                    send_message(socket, 0, pkt_get_seqnum(pkt), window, last_timestamp);
                    pkt_del(pkt);
                }
                else{
                    sorted_insert(&sorted_stack, *pkt);
                }
            }
            /* if tr == 1 and header is correct send nack */
            else if(pkt_get_tr(pkt) == 1){
                fprintf(stderr, "Truncated pkt received");
                tr = pkt_get_tr(pkt);
                last_timestamp = pkt_get_timestamp(pkt);
                send_message(socket, tr, pkt_get_seqnum(pkt), window, last_timestamp);
                pkt_del(pkt);
            }
            /* if tr == 0 and pkt is correct send ack*/
            else{
                fprintf(stderr, "Valid pkt received");
                if(pkt_get_seqnum(pkt) == min_seqnum_received){
                    fprintf(stderr, "Next packet received = %d", pkt_get_seqnum(pkt));
                    /*write*/
                    next_seqnum(&last_seqnum_written);
                    next_seqnum(&min_seqnum_received);
                }
                else{
                    sorted_insert(&sorted_stack, *pkt);
                    window--;
                    tr = pkt_get_tr(pkt);
                    last_timestamp = pkt_get_timestamp(pkt);
                    send_message(socket, tr, min_seqnum_received, window, last_timestamp);
                    write_to_file(file, &sorted_stack, &last_seqnum_written, &window);
                    if(compare_seqnum(min_seqnum_received, last_seqnum_written) > 0){
                        min_seqnum_received = last_seqnum_written;
                        next_seqnum(&min_seqnum_received);
                    }
                }
            }
        }
    }
}

int main(int argc, char** argv){

}