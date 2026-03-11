/* Encoding: UTF-8
@Authors: PdvSobral
@Date: 11/03/2026
@Links: Original project github repository -> https://github.com/PdvSobral/curricular_unit_P1
@Description:
    Header file for the linked_list module.
    The module was designed as a layer of abstraction on managing linked lists and double linked lists.
*/
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct _node {
    void* data;
    struct _node* next;
    struct _node* previous;
} NODE;

typedef struct _linkedlist {
    NODE* head;
    NODE* tail;
    int16_t size;
} LinkedList;

LinkedList*  create_linked_list		   ();
NODE*        create_node               (void *data);
NODE*        append_data_to_list       (LinkedList* list, void* data);
NODE*        insert_node_at_end        (LinkedList* list, NODE* new_node);
NODE*        insert_node_at_beggining  (LinkedList* list, NODE* new_node);
uint8_t      insert_node_at_index      (LinkedList* list, NODE* new_node, int32_t index);
uint8_t      insert_at_index           (LinkedList* list, void* data, int32_t index);
void         delete_linked_list        (LinkedList* list, void (*data_handler)(void*));
void         traverse_list             (LinkedList* list, void (*func)(void*));
NODE*        get_node_at_index         (LinkedList* list, int16_t index);
int32_t      find_node                 (LinkedList* list, int32_t (*check)(void*));
int32_t 	 find_node2				   (LinkedList* list, void* to_compare_to, int32_t (*check)(void*, void*));
int32_t      find_node_from            (LinkedList* list, int32_t (*check)(void*), int32_t from_index);
int32_t      count_occurences          (LinkedList* list, int32_t (*check)(void*));
uint8_t      remove_node_at_index      (LinkedList* list, int16_t index, void (*data_handler)(void*));
void         remove_nodes              (LinkedList* list, int32_t (*condition)(void*), void (*data_handler)(void*));
LinkedList*  concatenate               (LinkedList* less, LinkedList* equal, LinkedList* greater);
void         three_way_partition       (LinkedList* list, LinkedList* less, LinkedList* equal, LinkedList* greater, int32_t (*compare)(void*, void*));
void         sort_list                 (LinkedList* list, int32_t (*compare)(void*, void*));
void         three_way_quick_sort      (LinkedList* list, int32_t (*compare)(void*, void*));
