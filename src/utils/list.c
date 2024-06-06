#include "../include/list.h"

dynamic_list_t * init_list(size_t item_size)
{
    dynamic_list_t * list = calloc(1, sizeof(struct DYNAMIC_LIST_S));
    list->size = 0;
    list->item_size = item_size;
    list->items = (void**)0;

    return list;
}

// Adds a new item to the end of the list
void list_enqueue(dynamic_list_t * list, void * item)
{
    list_add_at(list, item, list->size-1);
}


// Adds a new item to the index of the list
void list_add_at(dynamic_list_t * list, void * item, size_t index)
{
    if(!list || !item || index > list->size)
    {
        return;
    }

    list->size++;

    if(list->items == (void**)0)
    {
        list->items = calloc(1, list->item_size);
    }
    else
    {
        list->items = realloc(list->items, list->size * list->item_size);
    }

    list_shift_right(list, index);
    list->items[index] = item;

}

//you only call this when you have deleted an item
void list_shift_left(dynamic_list_t * list, size_t index)
{
    for (int i = index; i < list->size - 1; i++)
    {
        list->items[i] = list->items[i + 1];
    }
}

// You only call this when you have allocated memory for the item
void list_shift_right(dynamic_list_t * list, size_t index)
{
    for (int i = list->size - 1; i >=index; i--)
    {
        list->items[i] = list->items[i - 1];
        list->items[i] = 0; // this will make sure that the item at the index will be empty
    }

}

void list_remove(dynamic_list_t * list, size_t index)
{   
    
    if(!list || index > list->size)
    {
        return;
    }

    free(list->items[index]); //free the item at the index (remember that we have a list of pointers, so we need to free the memory at where the pointer is pointing to)
    list_shift_left(list, index); //shift the list to the left
    list->size--;
    list->items = realloc(list->items, list->size * list->item_size);

}

void list_push(dynamic_list_t * list, void * item)
{
    list_add_at(list, item, 0);
}

//finds the index of the pointer in the list
int list_index(dynamic_list_t * list, void * ptr)
{
    for(int i = 0; i < list->size; i++)
    {
        if(list->items[i] == ptr)
        {
            return i;
        }
    }
    return -1;
}

int list_index_deep(dynamic_list_t * list, void * ptr)
{
    for(int i = 0; i < list->size; i++)
    {
        if (!list->items[i])
        {
            continue;
        }
        if(strcmp(list->items[i], ptr) == 0)
        {
            return i;
        }
    }
    return -1;
}

//returns if the list contains the item
int list_contains(dynamic_list_t * list, void * ptr)
{
    return (list_index(list, ptr) != -1);
}

//merges two lists together like list1 += list2 in that order
dynamic_list_t * list_merge(dynamic_list_t * list1, dynamic_list_t * list2)
{
    dynamic_list_t * list = init_list(list1->item_size);

    for(int i = 0; i < list1->size; i++)
    {
        list_enqueue(list, list1->items[i]);
    }

    for(int i = 0; i < list2->size; i++)
    {
        list_enqueue(list, list2->items[i]);
    }

    return list;
}

//copies the list
dynamic_list_t * list_copy(dynamic_list_t * list)
{
    dynamic_list_t * new_list = init_list(list->item_size);

    for(int i = 0; i < list->size; i++)
    {
        list_enqueue(new_list, list->items[i]);
    }

    return new_list;
}

void list_clear(dynamic_list_t * list)
{
    for(int i = 0; i < list->size; i++)
    {
        free(list->items[i]);
    }
    free(list->items);
    list->size = 0;
    list->items = (void**)0;
}

void list_free(dynamic_list_t * list)
{
    list_clear(list);
    free(list);
}