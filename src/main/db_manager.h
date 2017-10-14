#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#define FUSE_USE_VERSION 31

#include <bson.h>
#include <bcon.h>
#include <mongoc.h>

// Init
mongoc_client_t * _get_client();
mongoc_database_t * _get_database(char * database_name);
mongoc_collection_t * _get_collection(char * database_name, char * collection_name);
int db_init();
int disconnect();

// CRUD
int insert(bson_t * document);
char * find(bson_t * query);
int update(bson_t * query, bson_t * update);

// Document create
bson_t * document_create_file(int str_last_modification, const char * path);
bson_t * document_create_query_file(const char * path);
bson_t * document_create_update(int str_last_modification);

#endif
