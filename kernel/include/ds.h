#pragma once

#ifndef LIST_FREE_ELEMENT
#define __LIST_FREE_ELEMENT NULL
#else
#define __LIST_FREE_ELEMENT LIST_FREE_ELEMENT
#endif

#define __WRAP_IN_DO_WHILE(exp) do { exp } while(0)

// add element to double linked list
#define __LIST_ADD(list, value) \
    if ((list) == __LIST_FREE_ELEMENT) {  \
        (value)->prev = __LIST_FREE_ELEMENT;  \
    }  \
    else {  \
        (value)->prev = (list)->prev;  \
        if ((list)->prev != __LIST_FREE_ELEMENT) {  \
            (list)->prev->next = (value);  \
        }  \
        (list)->prev = (value);  \
    }  \
    (value)->next = (list);  \
    (list) = (value);

// add element to cyclic double linked list
#define __CLIST_ADD(list, value)  \
    LIST_ADD((list), (value));  \
    if ((value)->next == __LIST_FREE_ELEMENT && (value)->prev == __LIST_FREE_ELEMENT) {  \
        (value)->next = (value);  \
        (value)->prev = (value);  \
    }

// delete element from list
#define __LIST_REMOVE(value)  \
    if ((value)->next != __LIST_FREE_ELEMENT)  \
        (value)->next->prev = (value)->prev;  \
    if ((value)->prev != __LIST_FREE_ELEMENT)  \
        (value)->prev->next = (value)->next;

#define LIST_ADD(list, value) __WRAP_IN_DO_WHILE(__LIST_ADD((list), (value)))
#define CLIST_ADD(list, value) __WRAP_IN_DO_WHILE(__CLIST_ADD((list), (value)))
#define LIST_REMOVE(value) __WRAP_IN_DO_WHILE(__LIST_REMOVE(value))
