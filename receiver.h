//
// Created by root on 16/10/18.
//

#ifndef NETWORKING_PROJECT_RECEIVER_H
#define NETWORKING_PROJECT_RECEIVER_H

#include "utils.h"

/* Loop to receive data on the specified socket and write it on the specified FILE following the TRTP protocol
 * @file: the file we write the received data on
 * @socket: the socket we read the data from
 */

void receive_data_from_socket(FILE * file, int socket);

void send_message(int socket, uint8_t tr,uint8_t seqnum, uint8_t window, uint32_t last_timestamp);

void write_to_file(int fileDescriptor, int socket, struct stack ** sorted_stack, uint8_t* tr, uint32_t* last_timestamp, uint8_t* min_seqnum_received, uint8_t* last_seqnum_written, uint8_t* window, int* is_receiving);

#endif //NETWORKING_PROJECT_RECEIVER_H
