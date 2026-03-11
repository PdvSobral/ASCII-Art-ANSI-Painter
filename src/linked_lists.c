/* Encoding: UTF-8
@Authors: PdvSobral
@Date: 29/05/2025
@Links: Project github repository -> https://github.com/PdvSobral/curricular_unit_P1

This file contains the functions developed for a simple implementation of linked lists.
For now it does not have much, but the idea is to have simple and double linked lists support.
UPDATE:
	Now only has double, queue and stack are implementable with these methods, but maybe add specific
	The docstrings for the functions are not updated to reflect new logic and/or updated signature

DEBUG FUNCTIONS! DO NOT USE WITHOUT CHECKING THE CODE:
		void         print_list                (LinkedList* list); ONLY FOR int32_t malloced DATA

*/

#include "linked_lists.h"
#include <stdlib.h>		// malloc, alloc (sem ela tmb tive algumas instabilidades com o uso de unsigneds)
#include <stdint.h>		// uint8_t
#include <stdio.h>		// printf

LinkedList* create_linked_list() {
	/*
	Function to create a new linked list
	Arguments:
		None
	Return:
		Pointer to the newly created LinkedList.
		If it returns NULL, the allocation failed.
	*/
    LinkedList* list = (LinkedList*) malloc(sizeof(LinkedList));
    if (list == NULL) return NULL;
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

NODE* create_node(void *data) {
	/*
	Function to create a new node.
	If it returns NULL, the allocation failed.
	Arguments:
		None
	Return:
		Pointer to the newly created node
	*/
    NODE* new_node = (NODE *) malloc(sizeof(NODE));
    if (new_node == NULL) return NULL;
    new_node->data = data;
    new_node->next = NULL;
    new_node->previous = NULL;
    return new_node;
}

NODE* append_data_to_list(LinkedList* list, void* data) {
	/*
	Function to create a new node at the end of a linked list
	Arguments:
		LinkedList* list	-> Linked List to add the node to
		void* data			-> Data to be contained in the node
	Return:
		Pointer to the newly created Node
	*/
    NODE* new_node = create_node(data);
    if (new_node == NULL) return NULL;

    if (list->head == NULL) {
        list->head = new_node;
        list->tail = new_node;
    } else {
        list->tail->next = new_node;  	  // Append to the end
        new_node->previous = list->tail;  // Previous of new node is the old tail
        list->tail = new_node;  		  // Move the tail pointer to the new node
    }
    list->size++;
    return new_node;
}

NODE* insert_node_at_end(LinkedList* list, NODE* new_node) {
	/*
	Function to create a new node at the end of a linked list
	Arguments:
		LinkedList* list	-> Linked List to add the node to
		void* data			-> Data to be contained in the node
	Return:
		Pointer to the newly created Node
	*/
    if (list->head == NULL) {
        list->head = new_node;
        list->tail = new_node;
    } else {
        list->tail->next = new_node;  	  // Append to the end
        new_node->previous = list->tail;  // Previous of new node is the old tail
        list->tail = new_node;  		  // Move the tail pointer to the new node
    }
    list->size++;
    return new_node;
}

NODE* insert_node_at_beggining(LinkedList* list, NODE* new_node) {
	/*
	Function to create a new node at the end of a linked list
	Arguments:
		LinkedList* list	-> Linked List to add the node to
		void* data			-> Data to be contained in the node
	Return:
		Pointer to the newly created Node
	*/
    if (list->head == NULL) {
        list->head = new_node;
        list->tail = new_node;
    } else {
    	new_node->next = list->head;	 // Point to current first term
    	list->head->previous = new_node; // Point current first term previous to the new_node
    	list->head = new_node;			 // Update head to the new_node
    }
    list->size++;
    return new_node;
}

uint8_t insert_node_at_index(LinkedList* list, NODE* new_node, int32_t index) {
    /*
	Function to insert a new node at a certain index.
	Arguments:
		LinkedList* list	-> Linked list to add the nodes to.
		void* data			-> Data for the node
		int16_t index		-> Index to insert on
	Return:
		uint8_t 			-> Status: 0 all right,
									   1: index out of bounds
	*/
    if (index > list->size) return 1;
    if (index == 0) insert_node_at_beggining(list, new_node);
    else if (index == -1 || index == list->size) insert_node_at_end(list, new_node);
    else {
        NODE* current = list->head;
        for (int32_t i = 0; i < index - 1; i++) current = current->next;
        new_node->next = current->next;
        if (current->next != NULL) current->next->previous = new_node;
        else list->tail = new_node;
        current->next = new_node;
        new_node->previous = current;
    }
    list->size++;
    return 0;
}

uint8_t insert_at_index(LinkedList* list, void* data, int32_t index) {
    /*
	Function to insert a new node at a certain index.
	Arguments:
		LinkedList* list	-> Linked list to add the nodes to.
		void* data			-> Data for the node
		int32_t index		-> Index to insert on
	Return:
		uint8_t 			-> Status: 0 all right, 1: index out of bounds, 2: memory allocation for node failed
	*/
    if (index > list->size) return 1;
    NODE* new_node = create_node(data);
    if (new_node == NULL) return 2;
    if (index == 0) {
        new_node->next = list->head;
        if (list->head != NULL) {
            list->head->previous = new_node;
        }
        list->head = new_node;
        if (list->size == 0) {
            list->tail = new_node;
        }
    } else {
        NODE* current = list->head;
        for (int32_t i = 0; i < index - 1; i++) current = current->next;
        new_node->next = current->next;
        if (current->next != NULL) current->next->previous = new_node;
        else list->tail = new_node;
        current->next = new_node;
        new_node->previous = current;
    }
    list->size++;
    return 0;
}

void delete_linked_list(LinkedList* list, void (*data_handler)(void*)) {
	/*
	Function to delete and free the space of a linked list
	Arguments:
		LinkedList* list	-> Linked List to delete
	Return:
		None
	*/
    NODE* current = list->head;
    NODE* next_node;
    while (current != NULL) {
    	data_handler(current->data);
        next_node = current->next;
        free(current);
        current = next_node;
    }
    free(list);
    return;
}

void traverse_list(LinkedList* list, void (*func)(void*)) {
	/*
	Function to aply a function to every node.
	It will probably never be used, but whatever.
	Arguments:
		LinkedList* list	-> Linked List to delete
	Return:
		None
	*/
    NODE* current_node = list->head;
    while (current_node != NULL) {
        func(current_node->data);
        current_node = current_node->next;
    }
}

NODE* get_node_at_index(LinkedList* list, int16_t index) {
	/*
	Function to retrieve a node at a certain index.
	Arguments:
		LinkedList* list	-> Linked list to get the node from.
		int16_t index		-> Index to get node from
	Return:
		NODE* 				-> Address for the node, NULL id index does not exist.
	*/
    if (index >= list->size) return NULL; // Index out of bounds
    NODE* current = list->head;
    for (int16_t i = 0; i < index; i++) current = current->next;
    return current;
}

int32_t find_node(LinkedList* list, int32_t (*check)(void*)) {
	/*
	Function to find the first node that meets a certain criteria.
	Arguments:
		LinkedList* list			-> Linked list to search on.
		uint8_t (*contition)(void*)	-> Function to be used to check if the node is the wanted one.
									   If the return is 1, then the index for the node in cause is returned.
	Return:
		int8_t						-> If positive, index for the node found with the lowest index.
									   Else, no index found
	*/
    NODE* current = list->head;
    int32_t index = 0;
    while (current != NULL) {
        if (check(current->data) == 1) return index;
        current = current->next;
        index++;
    }
    return -1;
}

int32_t find_node2(LinkedList* list, void* to_compare_to, int32_t (*check)(void*, void*)) {
	/*
	Function to find the first node that meets a certain criteria.
	Arguments:
		LinkedList* list			-> Linked list to search on.
		uint8_t (*contition)(void*)	-> Function to be used to check if the node is the wanted one.
									   If the return is 1, then the index for the node in cause is returned.
	Return:
		int8_t						-> If positive, index for the node found with the lowest index.
									   Else, no index found
	*/
    NODE* current = list->head;
    int32_t index = 0;
    while (current != NULL) {
        if (check(current->data, to_compare_to) == 1) return index;
        current = current->next;
        index++;
    }
    return -1;
}

int32_t find_node_from(LinkedList* list, int32_t (*check)(void*), int32_t from_index) {
	/*
	Function to find the first node that meets a certain criteria, but only from a certain address.
	Arguments:
		LinkedList* list			-> Linked list to search on.
		uint8_t (*contition)(void*)	-> Function to be used to check if the node is the wanted one.
									   If the return is 1, then the index for the node in cause is returned.
		uint8_t from_index			-> Only search from that index onwards, index included
	Return:
		int8_t						-> If positive, index for the node found with the lowest index.
									   Else, no index found
	*/
    NODE* current = list->head;
    int32_t index = 0;
    while (current != NULL) {
        if (index>=from_index && check(current->data) == 1) return index;
        current = current->next;
        index++;
    }
    return -1;
}

int32_t count_occurences(LinkedList* list, int32_t (*check)(void*)) {
	/*
	Function to count the numbers of nodes that meet a certain criteria.
	Arguments:
		LinkedList* list			-> Linked list to search on.
		uint8_t (*contition)(void*)	-> Function to be used to check if the node is the wanted one.
									   If the return is 1, then the node is considered found.
	Return:
		uint8_t						-> Number of ocurrences found in the list
	*/
    NODE* current = list->head;
    int32_t count = 0;
    while (current != NULL) {
        if (check(current->data) == 1) count++;
        current = current->next;
    }
    return count;
}

uint8_t remove_node_at_index(LinkedList* list, int16_t index, void (*data_handler)(void*)) {
	/*
	Function to insert a new node at a certain index.
	Arguments:
		LinkedList* list	-> Linked list to delete the node from.
		int16_t index		-> Index to delete node on
	Return:
		uint8_t 				-> Exit status. 0: All right, 1: Index out of bounds.
	*/
    if (index >= list->size) return 1;
    NODE* current = list->head;
    if (index == 0) {
        list->head = current->next;
        if (list->head != NULL) list->head->previous = NULL;
        if (list->size == 1) list->tail = NULL;
    } else {
        for (int16_t i = 0; i < index; i++) current = current->next;
        current->previous->next = current->next;
        if (current->next != NULL) current->next->previous = current->previous;
        else list->tail = current->previous;
    }
    data_handler(current->data);
    free(current);
    list->size--;
    return 0;
}

void remove_nodes(LinkedList* list, int32_t (*condition)(void*), void (*data_handler)(void*)) {
	/*
	Function to remove every node that meets a certain criteria.
	Arguments:
		LinkedList* list			-> Linked list to delete the nodes from.
		uint8_t (*contition)(void*)	-> Function to be used to check if the node should be deleted.
									   If the return is 1, then the node in cause is deleted.
	Return:
		None
	*/
    if (list->head == NULL) return;
    NODE* current = list->head;
    NODE* previous = NULL;
    while (current != NULL) {
    	// Compare data using the provided function
        if (condition(current->data) == 1) {
            // It's the first node, so simply reassign the head node
            if (previous == NULL) {
                list->head = current->next;
                if (list->head != NULL) {
                    list->head->previous = NULL;  // Set the previous of the new head to NULL
                }
            } else {
                previous->next = current->next;
                if (current->next != NULL) {
                    current->next->previous = previous;  // Set the previous of the next node
                } else {
                    list->tail = previous;  // If it's the last node, update the tail
                }
            }
            data_handler(current->data);
            free(current);
            if (previous) {
				current = previous->next; // Move to the next node
			} else {
				current = list->head; // Reset to the head of the list
			}
            list->size--;
        } else {
            previous = current;
            current = current->next;
        }
    }
}

LinkedList* concatenate(LinkedList* less, LinkedList* equal, LinkedList* greater) {
    /*
    Concatenates three linked lists into a single linked list.
    Arguments:
        LinkedList* less      -> Pointer to the 'less' linked list.
        LinkedList* equal     -> Pointer to the 'equal' linked list.
        LinkedList* greater   -> Pointer to the 'greater' linked list.
    Return:
        LinkedList*          -> Pointer to the head of the concatenated linked list.
                                If all input lists are NULL, it returns NULL.
    */
    LinkedList* result = create_linked_list();
    if (result == NULL) return NULL;
    NODE* current = less->head;
    while (current != NULL) {
        append_data_to_list(result, current->data);
        current = current->next;
    }
    current = equal->head;
    while (current != NULL) {
        append_data_to_list(result, current->data);
        current = current->next;
    }
    current = greater->head;
    while (current != NULL) {
        append_data_to_list(result, current->data);
        current = current->next;
    }
    return result;
}

void three_way_partition(LinkedList* list, LinkedList* less, LinkedList* equal, LinkedList* greater, int32_t (*compare)(void*, void*)) {
    /*
    Used in three-way quick sort. If compare is less than 0, left. If 0, middle. Else right.
    */
    NODE* current = list->head;
    void* pivot = current->data;
    while (current != NULL) {
        int32_t cmp = compare(current->data, pivot);
        if (cmp < 0) {
            append_data_to_list(less, current->data);
        } else if (cmp == 0) {
            append_data_to_list(equal, current->data);
        } else {
            append_data_to_list(greater, current->data);
        }
        current = current->next;
    }
}

void sort_list(LinkedList* list, int32_t (*compare)(void*, void*)) {
    if (list->head == NULL || list->head->next == NULL) {
        printf("No need to sort: List is empty or has only one element.\n");
        return;
    }
    printf("Starting bubble sort...\n");

	NODE* current;
	for (uint8_t i = 0; i < list->size - 1; i++){
		// Last i elements are already in place
		current = list->head;
		for (uint8_t j = 0; j < list->size - i - 1; j++) {
			if (compare(current->data, current->next->data) == 1) {
				/*previous | current | next | next->next*/
				/*para mudar o current e o next*/
				if (current->previous!=NULL && current->next!=NULL && current->next->next!=NULL){
					// both to swap are in the middle of the list
					current->next->previous = current->previous;
					current->previous->next = current->next;
					current->previous = current->next;
					current->next = current->next->next;
					current->next->previous->next = current;
					current->next->previous = current;
				} else if (current->previous!=NULL && current->next!=NULL){
					// both to swap are in the end of the list
					current->previous->next = current->next;
					current->next->next = current;
					current->next->previous = current->previous;
					current->previous = current->next;
					current->next = NULL;
					// update the tail element
					list->tail = current;
				} else if (current->next->next!=NULL){
					// the elements to swap are the first two of the list
					current->next->next->previous = current;
					current->next->previous = NULL;
					current->previous = current->next;
					current->next = current->next->next;
					current->previous->next = current;
					// since the head changed, update the head
					list->head = current->previous;
				} else {
					// since the head changed, update the head
					list->head = current->next;
					list->tail = current;
					// update internal
					list->head->previous = NULL;
					list->tail->next = NULL;
					list->head->next = list->tail;
					list->tail->previous = list->head;
				}
			}
			else current = current->next;
		}
	}
    printf("Bubble sort completed.\n");
    return;
}

void three_way_quick_sort(LinkedList* list, int32_t (*compare)(void*, void*)) {
	// FIXME: it is not freeing the ones that are lost and recreated... I will have to solve this.
 	if (list == NULL || list->head == NULL || list->head->next == NULL) return;

    LinkedList* less = create_linked_list();
    LinkedList* equal = create_linked_list();
    LinkedList* greater = create_linked_list();

    three_way_partition(list, less, equal, greater, compare);

    three_way_quick_sort(less, compare);
    three_way_quick_sort(greater, compare);

    LinkedList* sorted = concatenate(less, equal, greater);

    list->head = sorted->head;
    list->tail = sorted->tail;
    list->size = sorted->size;

    free(less);
    free(equal);
    free(greater);
    free(sorted);
}

void print_list(LinkedList* list) {
	NODE* current = list->head;
	printf("NULL -> ");
	while (current != NULL) {
		printf("%d -> ", *(int32_t*)current->data);
		current = current->next;
	}
	printf("NULL\n");
}


/*
int check_value = 3;
int32_t compare_ints(void* a, void* b) {
    uint8_t val_a = *(uint8_t*)a;
    uint8_t val_b = *(uint8_t*)b;
    return val_a > val_b ? 1 : 0;
}

int32_t compare_ints2(void* a, void* b) {
    return *(uint8_t*) a - *(uint8_t*)b;
}

int32_t comparison(void* data){
        return (*(int*)data == check_value) ? 1 : 0;
}

int32_t count_lol(void* data) {
    return (*(int*)data == 2) ? 1 : 0;
}


int main() {
    // Create a linked list
    LinkedList* list = create_linked_list();
    if (list == NULL) {
        printf("Failed to create linked list.\n");
        return 1;
    }

    // Add nodes to the list
    for (int i = 0; i < 5; i++) {
        int* data = malloc(sizeof(int));
        *data = i + 1; // Adding values 1 to 5
        append_data_to_list(list, data);
    }

    // Print the list
    printf("Initial list: ");
    print_list(list);

    // Insert at index
    int* new_data = malloc(sizeof(int));
    *new_data = 10;
    if (insert_at_index(list, new_data, 2) == 0) {
        printf("After inserting 10 at index 2: ");
        print_list(list);
    } else {
        printf("Failed to insert at index 2.\n");
    }

    // Remove node at index
    if (remove_node_at_index(list, 1, free) == 0) {
        printf("After removing node at index 1: ");
        print_list(list);
    } else {
        printf("Failed to remove node at index 1.\n");
    }

    // Find a node

    int8_t found_index = find_node(list, comparison);
    if (found_index != -1) {
        printf("Found value %d at index %d.\n", check_value, found_index);
    } else {
        printf("Value %d not found in the list.\n", check_value);
    }

    // Count occurrences
    int count = count_occurences(list, count_lol);
    printf("Count of occurrences of value 2: %d\n", count);

    printf("Before sorting: ");
    print_list(list);
    // Sort the list
    //printf("Bubble sorting...\n"); sort_list(list, compare_ints);
    printf("Three way quick sorting...\n"); three_way_quick_sort(list, compare_ints2);
    printf("After sorting: ");
    print_list(list);

    // Clean up
    delete_linked_list(list, free);
    printf("Linked list deleted.\n");

    return 0;
}
*/
