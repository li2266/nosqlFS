#include "nosqlFS.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <bson.h>
#include <stdlib.h>
#include <fuse.h>

#include "util.h"
#include "cJSON.h"
#include "log.h"

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
// bug: head has no data
void list_append(struct head_node * head, void * data){
        struct node * p;
        p = (struct node *)malloc(sizeof(struct node));
        p->value = bson_copy(data);
        //p->value = data;
        p->next = NULL;
        if(head->head == NULL) {
                //head->head = p;
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
        p->value = bson_copy(data);
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
                if(sizeof(* tmp->value) == sizeof(bson_t)){
                    bson_destroy(tmp->value);
                }else{
                    free(p->value);
                }
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
        log_msg("GET TEST 1\n");
        json_str = bson_as_json(document, &len);
        // start parsing
        log_msg("GET TEST 2\n");
        cJSON * root_json = cJSON_Parse(json_str);
        log_msg("GET TEST 3\n");
        cJSON * command_parameter = cJSON_GetObjectItem(root_json, "command");
        log_msg("GET TEST 4\n");
        int command_length = cJSON_GetArraySize(command_parameter);
        log_msg("GET TEST 5\n");
        cJSON * item;
        for(int i = 1; i < command_length; ++i){
                item = cJSON_GetArrayItem(command_parameter, i);
                strcpy(parameters[i - 1], item->valuestring);
                log_msg("GET %s\n", parameters[i - 1]);
        }
        // -1 for removing the path of command
        return command_length - 1;
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
                        command[i][0] = '\0';
                        strcat(command[i], filename);
                        log_msg("PROCESS %s\n", command[i]);
                }else if(strcmp(command[i],"FILENAME_BACKUP") == 0){
                        char res[256] = "";
                        strcat(res, filename);
                        strcat(res, "_backup");
                        command[i][0] = '\0';
                        strcat(command[i], res);
                        log_msg("PROCESS %s\n", command[i]);
                }else if(strcmp(command[i], "NULL") == 0){
                        command[i] = NULL;
                        log_msg("PROCESS %s\n", command[i]);
                }else{
                        //strcpy(new_command[i], command[i]);
                        log_msg("PROCESS %s\n", command[i]);
                }
        }
        return 0;
}
/*
 * Some string process function
 * 1. remove quote
 */
char * remove_quote(char * str, char * new_str){
        strncpy(new_str, str + 1, strlen(str) - 2);
        new_str[strlen(str) - 2] = '\0';
        return new_str;
}

/*
 * Some string process function
 * 2. string split for special case '\0'
 */

void split(char * string, int length, char ** result){
    char * p = string;
    int index = -1;
    while(*p){
        strcpy(result[++index], p);
        p = strchr(p, '\0');
        p += strlen(p) + 1;
    }
}