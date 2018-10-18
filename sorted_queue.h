/*
 * author : Félix Jacoby
 *
 * inspired by : https://www.geeksforgeeks.org/sort-a-stack-using-recursion/
 *
 */

#ifndef NETWORKING_PROJECT_SORTED_QUEUE_H
#define NETWORKING_PROJECT_SORTED_QUEUE_H


#include "packet_interface.h"

struct stack
{
    pkt_t* pkt;
    struct stack *next;
};

// Utility function to initialize stack
void init_stack(struct stack **s);

// Utility function to chcek if stack is empty
int is_empty(struct stack *s);

// Utility function to push an item to stack
void push(struct stack **s, pkt_t* pkt);

// Utility function to look at an item from stack
pkt_t* peek(struct stack **s);

// Utility function to remove an item from stack
pkt_t* pop(struct stack **s);

// Function to find top item
pkt_t* top(struct stack *s);

// Recursive function to insert an item x in sorted way
void sorted_insert(struct stack **s, pkt_t* pkt);

// Function to sort stack
void sort_stack(struct stack **s);

#endif //NETWORKING_PROJECT_SORTED_QUEUE_H
