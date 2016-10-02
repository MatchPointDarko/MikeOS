#ifndef MIKE_OS_LIST_H
#define MIKE_OS_LIST_H

#include <common.h>

typedef struct node node_t;
typedef struct list_head
{
    node_t* head;
    node_t* iter_node;
} list_t;

bool_t initialize_list(list_t* list_head);
bool_t add_element(list_t* list_head, void* element);
bool_t remove_element(list_t* list_head, void* element);
uint32_t size(list_t* list_head);
void* iter_list(list_t* list_head);
void* pop_element(list_t* list_head, int32_t element_index);
bool_t is_empty(list_t* list_head);
bool_t free_list(list_t* list_head);

#endif //MIKE_OS_LIST_H
