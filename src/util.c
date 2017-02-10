#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <bson.h>
#include <cJSON.h>

#include "util.h"
/*
 * this section is about the list 
 */
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

/*
 * this section is about json parsing
 */
char * get_file_state(bson_t * document){
        char * json_str;
        size_t len;
        json_str = bson_as_json(document, &len);
        // start parsing
        cJSON * root_json = cJSON_Parse(json_str);
        char * state = cJSON_Print(cJSON_GetObjectItem(root_json, "state"));
        bson_free(json_str);
        return state;
}

/*




*/