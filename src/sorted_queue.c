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
void sorted_insert(struct stack **s, pkt_t* pkt, uint8_t* window)
{
    // Base case: Either stack is empty or newly inserted
    // item is greater than top (more than all existing)
    // if item we push is lenght 0 (end of transmission) we push it at the end
    // only one item with the same seqnum that is not length 0 in queue
    pkt_t* top_stack = top(*s);
    if(!is_empty(*s) && pkt_get_seqnum(pkt) == pkt_get_seqnum(top_stack) && pkt_get_length(pkt) == pkt_get_length(top_stack)){
        return;
    }
    if (is_empty(*s) || sorted_queue_compare_seqnum(pkt_get_seqnum(pkt), pkt_get_seqnum(top_stack)))
    {
        push(s, pkt);
        (*window)--;
        return;
    }
    // If top is smaller, remove the top item and recur
    pkt_t* temp = pop(s);
    sorted_insert(s, pkt, window);

    // Put back the top item removed earlier
    push(s, temp);
}