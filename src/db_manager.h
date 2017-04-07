#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#define FUSE_USE_VERSION 30

#include <bson.h>
#include <bcon.h>
#include <mongoc.h>

#include "util.h"

mongoc_client_t * _get_client();
mongoc_database_t * _get_database(char * database_name);
mongoc_collection_t * _get_collection(char * database_name, char * collection_name);
int db_init();
int disconnect();
bson_t * create_document_file(struct stat *si, const char * path);
void delete(char * path);
void modify_file(bson_t * document, const char * path);
struct head_node * find_file_by_path(const char * path);
struct head_node * find_file_by_document(bson_t * document);
struct head_node * find_file_by_key(int argc, char ** field, char ** value);
int insert_file(bson_t * document, const char * path);

bson_t * create_document_file_state(const char * path, char * state);
bson_t * modify_document(char ** key, char ** value);
struct head_node * find(char * name, const char * value, char * collection_name, char * sort_by, int sort);
struct head_node * find_by_path(const char * path, char * collection_name);
void update(bson_t * document, const char * path, char ** key, char ** value);
int insert(bson_t * document, const char * path, char * collection_name);

#endif
