#include <stdio.h>

#include "util.h"

struct head_node * list_init(){
        struct head_node * head;
        head = (struct head_node *)malloc(sizeof(struct head_node));
        head->head = NULL;
        head->tail = NULL;
        head->count = 0;
        return head;
}

void list_append(struct head_node * head, void * data){
        struct node * p;
        p = (struct node *)malloc(sizeof(struct node));
        p->value = data;
        p->next = NULL;
        if(head->tail == NULL) {
                head->tail = p;
        }else{
                head->tail->next = p;
                head->tail = p;
        }
        ++head->count;
}

void list_insert(struct head_node * head, void * data){
        struct node * p;
        p = (struct node *)malloc(sizeof(struct node));
        p->value = data;
        p->next = head->head;
        head->head = p;
        ++head->count;
}

void list_destory(struct head_node * head){
        struct node * p = head->head;
        struct node * tmp;
        while(p != NULL) {
                tmp = p;
                p = p->next;
                free(tmp);
                tmp = NULL;
        }
        free(head);
        head = NULL;
}
