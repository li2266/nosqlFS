#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>

struct node {
        void * value;
        struct node * next;
};

struct head_node {
        struct node * head;
        struct node * tail;
        int count;
};

struct head_node * list_init();

void list_append(struct head_node * head, void * data);

void list_insert(struct head_node * head, void * data);

void list_destory(struct head_node * head);

char * get_file_state(bson_t * document);


#endif
