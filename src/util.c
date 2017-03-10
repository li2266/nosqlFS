#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <bson.h>
#incldue <stdlib.h>

#include "util.h"
#include "cJSON.h"

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
 * this section is about json parsing for command in mongodb
 * add more function for getting parameter later
 */
char * get_command_location(bson_t * document){
        char * json_str;
        size_t len;
        json_str = bson_as_json(document, &len);
        // start parsing
        cJSON * root_json = cJSON_Parse(json_str);
        cJSON * command_location = cJSON_GetObjectItem(root_json, "command");
        char * res = cJSON_Print(cJSON_GetArrayItem(command_location, 0));
        return res;
}

int get_command_parameter(bson_t * document, char ** parameters){
        char * json_str;
        size_t len;
        json_str = bson_as_json(document, &len);
        // start parsing
        cJSON * root_json = cJSON_Parse(json_str);
        cJSON * command_parameter = cJSON_GetObjectItem(root_json, "command");
        int command_length = cJSON_GetArraySize(command_parameter) - 1;
        parameters = (char**)malloc(sizeof(char *) * command_length);
        cJSON * item;
        for(int i = 0; i < command_length; ++i){
                item = cJSON_GetArrayItem(command_parameter, i + 1);
                parameters[i] = (char*)malloc(sizeof(char) * 256);
                strcpy(parameters[i], item->valuestring);
        }
        return command_length;
}

char * get_value(bson_t * document, char * name){
        char * json_str;
        size_t len;
        json_str = bson_as_json(document, &len);
        // start parsing
        cJSON * root_json = cJSON_Parse(json_str);
        char * res = cJSON_Print(cJSON_GetObjectItem(root_json, name));
        return res;
}

/*
 * this section process command
 */

int command_process(char ** command, char * filename, int parameter_length){
        for(int i = 0; i < parameter_length; ++i){
                if(strcmp(command[i], "FILENAME") == 0){
                        strcpy(command[i], filename);
                }else if(strcmp(command[i],"FILENAME_BACKUP") == 0){
                        char res[256];
                        strcat(res, filename);
                        strcat(res, "_backup");
                        strcpy(command[i], res);
                }else if(strcmp(command[i], "NULL") == 0){
                        command[i] = NULL;
                }
        }
        return 0;
}

// remove quote

char * remove_quote(char * str, char * new_str){
        strncpy(new_str, str + 1, strlen(str) - 2);
        new_str[strlen(str) - 2] = '\0';
        return new_str;
}
