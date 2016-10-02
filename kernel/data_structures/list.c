/*
 * MikeOS: A list implemntation.
 */

#include <list.h>
#include <kmalloc.h>

typedef struct node
{
    void* element;
    struct node* next;
} node_t;

bool_t initialize_list(list_t* list_head)
{
    if(list_head == NULL)
    {
        return false;
    }

    list_head->head = NULL;

    list_head->iter_node = list_head->head;
}

bool_t add_element(list_t* list_head, void* element)
{
    if(list_head == NULL)
    {
        return false;
    }

    node_t** node = &list_head->head;

    while(*node != NULL)
        node = &((*node)->next);

    *node = kmalloc(sizeof(node_t));
    (*node)->next = NULL;
    (*node)->element = element;

    //If its the head..
    if(node == &list_head->head)
    {
        list_head->iter_node = *node;
    }

    return true;
}

void* pop_element(list_t* list_head, int32_t element_index)
{
    if(list_head == NULL || element_index < 0)
    {
        return NULL;
    }

    node_t** node_ptr = &list_head->head;

    //if index_element < 0 we return the first node
    for(node_t* node = list_head->head; node != NULL;
        node_ptr = &node->next, node = node->next, element_index--)
    {
        //Compare pointers for now.
        if(element_index == 0)
        {
            *node_ptr = node->next;
            void* element = node->element;
            kfree(node);

            //Just incase, update the iter node
            list_head->iter_node = list_head->head;

            return element;
        }
    }

    return NULL;
}

bool_t remove_element(list_t* list_head, void* element)
{
    node_t** node_ptr = &list_head->head;
    for(node_t* node = list_head->head; node != NULL; node_ptr = &node->next, node = node->next)
    {
        //Compare pointers for now.
        if(node->element == element)
        {
            *node_ptr = node->next;
            return true;
        }
    }

    return false;
}

uint32_t size(list_t* list_head)
{
    if(list_head == NULL)
    {
        return 0;
    }

    uint32_t counter = 0;
    for(node_t* node = list_head->head; node != NULL; node = node->next)
    {
        counter++;
    }

    return counter;
}

bool_t is_empty(list_t* list_head)
{
    if(list_head != NULL)
        return !list_head->head;
    else
        return false;
}

void* iter_list(list_t* list_head)
{
    if(list_head->iter_node == NULL)
    {
        list_head->iter_node = list_head->head;
        return NULL;
    }

    void* element = (list_head->iter_node)->element;
    list_head->iter_node = list_head->iter_node->next;

    return element;
}

bool_t free_list(list_t* list_head)
{
    //Assumes every pointer was allocated with kmalloc
    node_t* tmp = NULL;

    for(node_t* node = list_head->head; node != NULL; node = tmp)
    {
        tmp = node->next;
        kfree(node);
    }
}
