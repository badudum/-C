#ifndef DYNAMIC_LIST_H
#define DYNAMIC_LIST_H
#include <stdlib.h>
#include <string.h>

#define MAX(a,b) a>b? a:b
#define MIN(a,b) a<b? a:b

typedef struct DYNAMIC_LIST_S
{
    void ** items;
    size_t size;
    size_t item_size;
}dynamic_list_t;


dynamic_list_t * init_list(size_t item_size);

//adds an item to the end of the list
void list_enqueue(dynamic_list_t * list, void * item);

void list_add_at(dynamic_list_t * list, void * item, size_t index);

void list_push_safe(dynamic_list_t * list, void * item);

void list_push_safe_at(dynamic_list_t * list, void * item, size_t index);

void list_shift_left(dynamic_list_t * list, size_t index);

void list_shift_right(dynamic_list_t * list, size_t index);

void list_remove(dynamic_list_t * list, size_t index);

//adds an item to the beginning of the list
void list_push(dynamic_list_t * list, void * item);

//finds index of the item in the list
int list_index(dynamic_list_t * list, void * ptr);

//finds the index of the item in the list, but checks for the content
int list_index_deep(dynamic_list_t * list, void * ptr);

int list_contains(dynamic_list_t * list, void * ptr);

dynamic_list_t * list_merge(dynamic_list_t * list1, dynamic_list_t * list2);

dynamic_list_t * list_copy(dynamic_list_t * list);

void list_clear(dynamic_list_t * list);

void list_free(dynamic_list_t * list);


#endif