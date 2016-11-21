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

bson_t * create_document(struct stat *si, const char * path){
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

void modify(bson_t * document, const char * path){
        log_msg("DB_manager function: Modify\n");
        bson_error_t error;
        bson_t * query = BCON_NEW("path", BCON_UTF8(path));
        if(!mongoc_collection_update(_get_collection("nosqlFS", "file"), MONGOC_UPDATE_NONE, query, document, NULL, &error)) {
                fprintf(stderr, "Modify failed: %s\n", error.message);
        }
        bson_destroy(document);
        bson_destroy(query);
}
/*
   original name of this function is search,
   I change that because find is used by mongodb
 */
// there are three find functions. this is because it's diffcult to implement a override in C

struct head_node * find_by_path(const char * path){
        log_msg("DB_manager function: find_by_path\n");
        bson_t * document = BCON_NEW("path", BCON_UTF8(path));
        const bson_t * result;
        mongoc_cursor_t * cursor = mongoc_collection_find(_get_collection("nosqlFS", "file"), MONGOC_QUERY_NONE, 0, 0, 0, document, NULL, NULL);
        struct head_node * head = list_init();
        log_msg("DB_manager function: find_by_path->list_init()\n");
        while(mongoc_cursor_next(cursor, &result)) {
                list_append(head, (void *)result);
                log_msg("DB_manager function: append\n");
        }
        return head;
}

struct head_node * find_by_document(bson_t * document){
        log_msg("DB_manager function: find_by_path\n");
        const bson_t * result;
        mongoc_cursor_t * cursor = mongoc_collection_find(_get_collection("nosqlFS", "file"), MONGOC_QUERY_NONE, 0, 0, 0, document, NULL, NULL);
        struct head_node * head = list_init();
        while(mongoc_cursor_next(cursor, &result)) {
                list_append(head, (void *)result);
        }
        return head;
}

struct head_node * find_by_key(int argc, char ** field, char ** value){
        log_msg("DB_manager function: find_by_path\n");
        bson_t * document = bson_new();
        const bson_t * result;
        for(int i = 0; i < argc; ++i) {
                BSON_APPEND_UTF8(document, field[i], value[i]);
                //bson_append_utf8(document, BCON_UTF8(field[i]), strlen(field[i]), BCON_UTF8(value[i]), strlen(value[i]));
        }
        mongoc_cursor_t * cursor = mongoc_collection_find(_get_collection("nosqlFS", "file"), MONGOC_QUERY_NONE, 0, 0, 0, document, NULL, NULL);
        struct head_node * head = list_init();
        while(mongoc_cursor_next(cursor, &result)) {
                list_append(head, (void *)result);
        }
        return head;
}

int insert(bson_t * document, const char * path){
        log_msg("DB_manager function: Insert\n");
        bson_error_t error;
        struct head_node * head = find_by_path(path);
        if(head->count != 0) {
                modify(document, path);
        }else{
                int result = mongoc_collection_insert(_get_collection("nosqlFS", "file"), MONGOC_INSERT_NONE, document, NULL, &error);
                if(!result) {
                        fprintf(stderr, "Insert failed: %s\n", error.message);
                }
                log_msg("DB_manager function: Insert returned: %d\n", result);
        }
        bson_destroy(document);
        list_destory(head);
        return 0;
}
