#include "db_manager.h"

#include <bson.h>
#include <bcon.h>
#include <mongoc.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fuse.h>

#include "util.h"
#include "log.h"

static mongoc_client_t * client;
static mongoc_database_t * database;
static mongoc_collection_t * collection;


//TODO: all this operation should be thread-safe
mongoc_client_t * _get_client(){
        if(client == NULL) {
                client = mongoc_client_new ("mongodb://localhost:27017");
        }
        return client;
}

mongoc_database_t * _get_database(char * database_name){
        if(database == NULL) {
                if(client == NULL) {
                        _get_client();
                }
                database = mongoc_client_get_database (client, database_name);
        }
        return database;
}

mongoc_collection_t * _get_collection(char * database_name, char * collection_name){
        if(collection == NULL) {
                if(client == NULL) {
                        _get_client();
                }
                collection = mongoc_client_get_collection (client, database_name, collection_name);
        }
        return collection;
}

int db_init(){
        // mongoc_client_t      *client;
        // mongoc_database_t    *database;
        // mongoc_collection_t  *collection;
        bson_t               *command,
                             reply,
                             *insert;
        bson_error_t error;
        char                 *str;
        bool retval;

        /*
         * Required to initialize libmongoc's internals
         */
        mongoc_init();
        /*
         * Create a new client instance
         */
        client = _get_client();

        /*
         * Get a handle on the database "db_name" and collection "coll_name"
         */
        database = _get_database("nosqlFS");
        collection = _get_collection("nosqlFS", "file");

        return 0;
}

int disconnect(){
        /*
         * Release our handles and clean up libmongoc
         */
        mongoc_collection_destroy (collection);
        mongoc_database_destroy (database);
        mongoc_client_destroy (client);
        mongoc_cleanup ();

        return 0;
}

bson_t * create_document_file(struct stat *si, const char * path){
        // the data stored in mongodb should be string
        char str_st_dev[256];
        char str_st_ino[256];
        char str_st_mode[256];
        char str_st_nlink[256];
        char str_st_uid[128];
        char str_st_gid[128];
        char str_st_rdev[256];
        char str_st_size[256];
        char str_st_blksize[256];
        char str_st_blocks[256];
        char str_st_atime[256];
        char str_st_mtime[256];
        char str_st_ctime[256];
        /* store a long number to represent the time
           struct tm * last_access;
           struct tm * last_modif;
           struct tm * last_stat_change;
         */

        // get data in struct stat
        sprintf(str_st_dev, "%lu", si->st_dev);
        sprintf(str_st_ino, "%lu", si->st_ino);
        sprintf(str_st_mode, "%o", si->st_mode);
        sprintf(str_st_nlink, "%lu", si->st_nlink);
        sprintf(str_st_uid, "%d", si->st_uid);
        sprintf(str_st_gid, "%d", si->st_gid);
        sprintf(str_st_rdev, "%lu", si->st_rdev);
        sprintf(str_st_size, "%ld", si->st_size);
        sprintf(str_st_blksize, "%ld", si->st_blksize);
        sprintf(str_st_blocks, "%ld", si->st_blocks);
        sprintf(str_st_atime, "%ld", si->st_atime);
        sprintf(str_st_mtime, "%ld", si->st_mtime);
        sprintf(str_st_ctime, "%ld", si->st_ctime);

        bson_t * document = BCON_NEW(
                "path", BCON_UTF8(path),
                "st_dev", BCON_UTF8(str_st_dev),
                "st_ino", BCON_UTF8(str_st_ino),
                "st_mode", BCON_UTF8(str_st_mode),
                "st_nlink", BCON_UTF8(str_st_nlink),
                "st_uid", BCON_UTF8(str_st_uid),
                "st_gid", BCON_UTF8(str_st_gid),
                "st_rdev", BCON_UTF8(str_st_rdev),
                "st_size", BCON_UTF8(str_st_size),
                "st_blksize", BCON_UTF8(str_st_blksize),
                "st_blocks",BCON_UTF8(str_st_blocks),
                "st_atime", BCON_UTF8(str_st_atime),
                "st_mtime", BCON_UTF8(str_st_mtime),
                "st_ctime", BCON_UTF8(str_st_ctime)
                );
        return document;
}

/*
   function delete is not useful now.
 */
void delete(char * path){
        bson_error_t error;
        bson_t * document = BCON_NEW("path", BCON_UTF8(path));
        if(!mongoc_collection_remove(_get_collection("nosqlFS", "file"), MONGOC_REMOVE_SINGLE_REMOVE, document, NULL, &error)) {
                fprintf(stderr, "Delete failed: %s\n", error.message);
        }
}

void modify_file(bson_t * document, const char * path){
        log_msg("DB_manager function: Modify file path = %s\n", path);
        bson_error_t error;
        bson_t * query = BCON_NEW("path", BCON_UTF8(path));
        log_msg("DB_manager function: Modify file start\n");
        if(!mongoc_collection_update(_get_collection("nosqlFS", "file"), MONGOC_UPDATE_NONE, query, document, NULL, &error)) {
                log_msg("Modify file failed: %s\n", error.message);
        }
        log_msg("DB_manager function: Modify file finished\n");
        //TODO: maybe cause issue
        bson_destroy(document);
        bson_destroy(query);
}
/*
   original name of this function is search,
   I change that because find is used by mongodb
 */
// there are three find functions. this is because it's diffcult to implement a override in C

struct head_node * find_file_by_path(const char * path){
        log_msg("DB_manager function: find_file_by_path : %s\n", path);
        bson_t * document = BCON_NEW("path", BCON_UTF8(path));
        const bson_t * result;
        mongoc_cursor_t * cursor = mongoc_collection_find(_get_collection("nosqlFS", "file"), MONGOC_QUERY_NONE, 0, 0, 0, document, NULL, NULL);
        struct head_node * head = list_init();
        log_msg("DB_manager function: find_file_by_path->list_init()\n");
        while(mongoc_cursor_next(cursor, &result)) {
                list_append(head, (void *)result);
                log_msg("DB_manager function: append in find_file_by_path\n");
        }
        mongoc_cursor_destroy(cursor);
        bson_destroy(document);
        return head;
}

struct head_node * find_file_by_document(bson_t * document){
        log_msg("DB_manager function: find_file_by_document\n");
        const bson_t * result;
        mongoc_cursor_t * cursor = mongoc_collection_find(_get_collection("nosqlFS", "file"), MONGOC_QUERY_NONE, 0, 0, 0, document, NULL, NULL);
        struct head_node * head = list_init();
        while(mongoc_cursor_next(cursor, &result)) {
                list_append(head, (void *)result);
        }
        mongoc_cursor_destroy(cursor);
        bson_destroy(document);
        return head;
}

struct head_node * find_file_by_key(int argc, char ** field, char ** value){
        log_msg("DB_manager function: find_file_by_key\n");
        bson_t * document = bson_new();
        const bson_t * result;
        for(int i = 0; i < argc; ++i) {
                BSON_APPEND_UTF8(document, field[i], value[i]);
        }
        mongoc_cursor_t * cursor = mongoc_collection_find(_get_collection("nosqlFS", "file"), MONGOC_QUERY_NONE, 0, 0, 0, document, NULL, NULL);
        struct head_node * head = list_init();
        while(mongoc_cursor_next(cursor, &result)) {
                list_append(head, (void *)result);
        }
        mongoc_cursor_destroy(cursor);
        bson_destroy(document);
        return head;
}

int insert_file(bson_t * document, const char * path){
        log_msg("DB_manager function: Insert file\n");
        bson_error_t error;
        struct head_node * head = find_file_by_path(path);
        if(head->count != 0) {
                modify_file(document, path);
        }else{
                int result = mongoc_collection_insert(_get_collection("nosqlFS", "file"), MONGOC_INSERT_NONE, document, NULL, &error);
                if(!result) {
                        log_msg("Insert file failed: %s\n", error.message);
                }
                log_msg("DB_manager function: Insert file returned: %d, path = %s\n", result, path);
                // if this document is modified in this if/else struct, the document will be deleted in modify function
                // so we need to delete it in there, instead of delete it outside. It's not prefect but no more idea now
                bson_destroy(document);
        }
        list_destory(head);
        return 0;
}

// not useful now
bson_t * create_document_file_state(const char * path, char * state){
    bson_t * document = BCON_NEW(
        "path", BCON_UTF8(path),
        "state",BCON_UTF8(state)
        );
    return document;
}

// not useful now
// very easy version
bson_t * modify_document(char ** key, char ** value){
    return NULL;
}

// find for all document
struct head_node * find(char * name, const char * value, char * collection_name){
        log_msg("DB_manager function: find : name = %s, value = %s, collection = %s\n", name, value, collection_name);
        bson_t * document = bson_new();
        //BSON_APPEND_UTF8(document, "xattr", "user.backup");
        const bson_t * result;
        mongoc_cursor_t * cursor = mongoc_collection_find(_get_collection("nosqlFS", "xattr_list"), MONGOC_QUERY_NONE, 0, 0, 0, document, NULL, NULL);
        // list will be released when finished using such as after insertion
        struct head_node * head = list_init();
        log_msg("DB_manager function: find->list_init() %s\n", collection_name);
        while(mongoc_cursor_next(cursor, &result)) {
                // It's better to use the original type because maybe I will use it again

                char * str = bson_as_json (result, NULL);
                log_msg("TEST: %s\n", str);

                list_append(head, (void *)result);
                log_msg("DB_manager function: append in find %s\n", collection_name);
        }
        log_msg("Length of list %d\n", head->count);
        log_msg("DB_manager function: append in find finished %s\n", collection_name);
        mongoc_cursor_destroy(cursor);
        bson_destroy(document);
        return head;
}

// not useful now
struct head_node * find_by_path(const char * path, char * collection_name){
        log_msg("DB_manager function: find_by_path : path = %s, collection = %s\n", path, collection_name);
        bson_t * document = BCON_NEW("path", BCON_UTF8(path));
        const bson_t * result;
        mongoc_cursor_t * cursor = mongoc_collection_find(_get_collection("nosqlFS", collection_name), MONGOC_QUERY_NONE, 0, 0, 0, document, NULL, NULL);
        // list will be released when finished using such as after insertion
        struct head_node * head = list_init();
        log_msg("DB_manager function: find_by_path->list_init()\n");
        while(mongoc_cursor_next(cursor, &result)) {
                // It's better to use the original type because maybe I will use it again
                list_append(head, (void *)result);
                log_msg("DB_manager function: append\n");
        }
        mongoc_cursor_destroy(cursor);
        bson_destroy(document);
        return head;
}

//TODO:need modify
void update(bson_t * document, const char * path, char ** key, char ** value){
        log_msg("DB_manager function: update path = %s\n", path);
        bson_error_t error;
        bson_t * query = BCON_NEW("path", BCON_UTF8(path));
        log_msg("DB_manager function: Modify file start\n");
        if(!mongoc_collection_update(_get_collection("nosqlFS", "file"), MONGOC_UPDATE_NONE, query, document, NULL, &error)) {
                log_msg("Modify file failed: %s\n", error.message);
        }
        log_msg("DB_manager function: Modify finished\n");
        //TODO: maybe cause issue
        bson_destroy(document);
        bson_destroy(query);
}

//TODO: need modify
int insert(bson_t * document, const char * path, char * collection_name){
        log_msg("DB_manager function: Insert\n");
        bson_error_t error;
        int result = mongoc_collection_insert(_get_collection("nosqlFS", collection_name), MONGOC_INSERT_NONE, document, NULL, &error);
        if(!result) {
            log_msg("Insert file failed: %s\n", error.message);
        }
        log_msg("DB_manager function: Insert file returned: %d, path = %s\n", result, path);
        // if this document is modified in this if/else struct, the document will be deleted in modify function
        // so we need to delete it in there, instead of delete it outside. It's not prefect but no more idea now
        bson_destroy(document);
        return 0;
}
