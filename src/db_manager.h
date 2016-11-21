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
bson_t * create_document(struct stat *si, const char * path);
void delete(char * path);
void modify(bson_t * document, const char * path);
struct head_node * find_by_path(const char * path);
struct head_node * find_by_document(bson_t * document);
struct head_node * find_by_key(int argc, char ** field, char ** value);
int insert(bson_t * document, const char * path);

#endif
