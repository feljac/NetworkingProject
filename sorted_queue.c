//
// Created by root on 17/10/18.
//

#include "utils.h"

// Utility function to initialize stack
void init_stack(struct stack **s)
{
    *s = NULL;
}

// Utility function to chcek if stack is empty
int is_empty(struct stack *s)
{
    if (s == NULL)
        return 1;
    return 0;
}

// Utility function to push an item to stack
void push(struct stack **s, pkt_t* pkt)
{
    struct stack *p = (struct stack *)malloc(sizeof(*p));

    if (p == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        return;
    }

    p->pkt = pkt;
    p->next = *s;
    *s = p;
}

// Utility function to look at an item from stack
pkt_t* peek(struct stack **s)
{
    if(is_empty(*s)){
        return NULL;
    }
    return (*s)->pkt;
}

// Utility function to remove an item from stack
pkt_t* pop(struct stack **s)
{
    pkt_t* x;
    struct stack *temp;

    x = (*s)->pkt;
    temp = *s;
    (*s) = (*s)->next;
    free(temp);

    return x;
}

// Function to find top item
pkt_t* top(struct stack *s)
{
    if(is_empty(s)){
        return NULL;
    }
    return (s->pkt);
}

// Recursive function to insert an item x in sorted way
void sorted_insert(struct stack **s, pkt_t* pkt)
{
    fprintf(stderr,"Recurcive call sorted_insert seq: %d\n", pkt_get_seqnum(pkt));
    // Base case: Either stack is empty or newly inserted
    // item is greater than top (more than all existing)
    // if item we push is lenght 0 (end of transmission) we push it at the end
    pkt_t* top_stack = top(*s);
    fprintf(stderr,"TEST\n");
    if (is_empty(*s) || sorted_queue_compare_seqnum(pkt_get_seqnum(pkt), pkt_get_seqnum(top_stack)) || (pkt_get_seqnum(pkt) == pkt_get_seqnum(top_stack) && pkt_get_length(pkt) != 0))
    {
        fprintf(stderr,"Recurcive call sorted_insert PUSH seq: %d\n", pkt_get_seqnum(pkt));
        push(s, pkt);
        return;
    }

    fprintf(stderr,"TEST2\n");
    // If top is smaller, remove the top item and recur
    pkt_t* temp = pop(s);
    sorted_insert(s, pkt);

    // Put back the top item removed earlier
    push(s, temp);
}

// Function to sort stack
void sort_stack(struct stack **s)
{
    // If stack is not empty
    if (!is_empty(*s))
    {
        // Remove the top item
        pkt_t* x = pop(s);

        // Sort remaining stack
        sort_stack(s);

        // Push the top item back in sorted stack
        sorted_insert(s, x);
    }
}