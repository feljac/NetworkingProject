//
// Created by root on 16/10/18.
//

#include "receiver.h"

int main(int argc, char** argv){
    int port;
    char *host;
    FILE* file;
    char* filename = NULL;
    int c;
    const char *err;
    struct sockaddr_in6 addr;
    int is_file = 0;

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

    if(argc == 1 || argc > 4){
        fprintf(stderr,"need or a lot of arguments\n");
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);
    host = argv[0];

    err = real_address(host, &addr);
    if (err) {
        fprintf(stderr, "Could not resolve hostname %s: %s\n", host, err);
        return EXIT_FAILURE;
    }

    int sfd = create_socket(&addr, port, NULL, -1); /* Connected */

    if(!sfd || wait_for_client(sfd) == -1){
        fprintf(stderr, "Error waiting for client\n");
        return EXIT_FAILURE;
    }

    if(is_file){
        file = fopen(filename, "w");
        free(filename);
    }
    else{
        file = stdout;
    }

    receive_data_from_socket(file, sfd);

    if(is_file){
        fclose(file);
    }
}

void send_message(int socket, uint8_t tr,uint8_t seqnum, uint8_t window, uint32_t last_timestamp){
    pkt_t* pkt = pkt_new();
    if(tr == 1){
        fprintf(stderr, "Sending NACK : %d\n", seqnum);
        pkt_set_type(pkt, PTYPE_NACK);
    }
    else{
        fprintf(stderr, "Sending ACK %d\n", seqnum);
        pkt_set_type(pkt, PTYPE_ACK);
    }
    pkt_set_seqnum(pkt, seqnum);
    pkt_set_window(pkt, window);
    pkt_set_timestamp(pkt, last_timestamp);
    size_t length = PKT_LENGTH;
    char* buffer = malloc(PKT_LENGTH);
    if(pkt_encode(pkt, buffer, &length) != PKT_OK){
        fprintf(stderr, "Error encode message\n");
    }
    if(write(socket, buffer, length) == -1){
        fprintf(stderr, "Error write message\n");
    }
    free(buffer);
    pkt_del(pkt);
}

void write_to_file(int fileDescriptor, int socket, struct stack ** sorted_stack, uint8_t* tr, uint32_t* last_timestamp, uint8_t* min_seqnum_received, uint8_t* last_seqnum_written, uint8_t* window, int* is_receiving){
    int writed;
    pkt_t* pkt = peek(sorted_stack);
    while(pkt && (compare_seqnum(*last_seqnum_written, pkt_get_seqnum(pkt)) == 1 || pkt_get_length(pkt) == 0)){
        uint16_t length = pkt_get_length(pkt);
        if((writed = write( fileDescriptor, pkt_get_payload(pkt), length)) == -1){
            fprintf(stderr, "Write file error\n");
        }
        *tr = pkt_get_tr(pkt);
        *last_timestamp = pkt_get_timestamp(pkt);
        (*window)++;
        pop(sorted_stack);
        fprintf(stderr, "Write to file from queue : %d\n", pkt_get_seqnum(pkt));
        next_seqnum(last_seqnum_written);
        next_seqnum(min_seqnum_received);
        if(pkt_get_length(pkt) == 0){
            fprintf(stderr, "End of file we need to stop !\n");
            send_message(socket, *tr, *min_seqnum_received, *window, *last_timestamp);
            send_message(socket, *tr, *min_seqnum_received, *window, *last_timestamp);
            send_message(socket, *tr, *min_seqnum_received, *window, *last_timestamp);
            send_message(socket, *tr, *min_seqnum_received, *window, *last_timestamp);
            send_message(socket, *tr, *min_seqnum_received, *window, *last_timestamp);
            send_message(socket, *tr, *min_seqnum_received, *window, *last_timestamp);
            *is_receiving = 0;
        }
        pkt_del(pkt);
        pkt = peek(sorted_stack);
    }
}

void receive_data_from_socket(FILE* file, int socket){
    int fileDescriptor;
    if((fileDescriptor = fileno(file)) == -1){
        fprintf(stderr, "error with transformation FILE* into file descriptor\n");
        return;
    }
    int is_receiving = 1;
    ssize_t read_size = 0;
    uint8_t  tr = 0;
    uint8_t min_seqnum_received = 0;
    uint8_t last_seqnum_written = 255;
    uint8_t window = MAX_WINDOW_SIZE;
    uint32_t last_timestamp = 0;
    char buffer[sizeof(pkt_t) + MAX_PAYLOAD_SIZE];
    pkt_t* pkt;
    struct stack* sorted_stack;
    init_stack(&sorted_stack);

    struct pollfd fds[1];
    fds[0].fd = socket;
    fds[0].events = POLLIN;

    while(is_receiving){
        if(poll(fds, 1, -1) == -1){
            fprintf(stderr, "Error poll\n");
            continue;
        }
        if(fds[0].revents & POLLIN){

            if((read_size = read(socket, buffer, sizeof(pkt_t) + MAX_PAYLOAD_SIZE)) == -1){
                fprintf(stderr, "Error read on socket\n");
                continue;
            }

            pkt = pkt_new();

            /*receive data and put it in a pkt*/
            if(pkt_decode(buffer, (size_t)read_size, pkt) != PKT_OK){
                fprintf(stderr, "Packet corupted\n");
                pkt_del(pkt);
            }
            else if(pkt->header.type != PTYPE_DATA){
                fprintf(stderr, "Not a data pkt\n");
                pkt_del(pkt);
            }
            else if(window == 0){
                fprintf(stderr, "window is full\n");
                pkt_del(pkt);
            }
            else if(compare_seqnum(min_seqnum_received, pkt_get_seqnum(pkt)) >= MAX_WINDOW_SIZE || compare_seqnum(pkt_get_seqnum(pkt), min_seqnum_received) < 0){
                fprintf(stderr, "Seqnum out of window, min expected : %d -  received : %d\n", min_seqnum_received, pkt_get_seqnum(pkt));
                send_message(socket, 0, min_seqnum_received, window, last_timestamp);
                pkt_del(pkt);
            }
            /* if tr == 1 and header is correct send nack */
            else if(pkt_get_tr(pkt) == 1){
                fprintf(stderr, "Truncated pkt received\n");
                last_timestamp = pkt_get_timestamp(pkt);
                send_message(socket, 1, pkt_get_seqnum(pkt), window, last_timestamp);
                pkt_del(pkt);
            }
            /* if tr == 0 and pkt is correct send ack*/
            else{
                tr = pkt_get_tr(pkt);
                last_timestamp = pkt_get_timestamp(pkt);
                fprintf(stderr, "Valid pkt received : %d\n", pkt_get_seqnum(pkt));
                sorted_insert(&sorted_stack, pkt, &window);
            }
            write_to_file(fileDescriptor, socket, &sorted_stack, &tr, &last_timestamp, &min_seqnum_received, &last_seqnum_written, &window, &is_receiving);
            send_message(socket, tr, min_seqnum_received, window, last_timestamp);
        }
    }
}
