#include <bson.h>
#include <bcon.h>
#include <mongoc.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

static mongoc_client_t * client;
static mongoc_database_t * database;
static mongoc_collection_t * collection;


//TODO: all this operation should be thread-safe
static mongoc_client_t * _get_client(){
        if(client == NULL) {
                client = mongoc_client_new ("mongodb://localhost:27017");
        }
        return client;
}

static mongoc_database_t * _get_database(char * database_name){
        if(database == NULL) {
                if(client == NULL) {
                        _get_client();
                }
                database = mongoc_client_get_database (client, database_name);
        }
        return database;
}

static mongoc_collection_t * _get_collection(char * database_name, char * collection_name){
        if(collection == NULL) {
                if(client == NULL) {
                        _get_client();
                }
                collection = mongoc_client_get_collection (client, database_name, collection_name);
        }
        return collection;
}

static int db_init(){
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
        database = _get_database(client, "nosqlFS");
        collection = _get_collection(client, "nosqlFS", "file");

        /*
         * Do work. This example pings the database, prints the result as JSON and
         * performs an insert
         */
        command = BCON_NEW ("ping", BCON_INT32 (1));

        printf(command);
        /*
           retval = mongoc_client_command_simple (client, "admin", command, NULL, &reply, &error);

           if (!retval) {
                fprintf (stderr, "%s\n", error.message);
                return EXIT_FAILURE;
           }

           str = bson_as_json (&reply, NULL);
           printf ("%s\n", str);

           insert = BCON_NEW ("hello", BCON_UTF8 ("world"));

           if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, insert, NULL, &error)) {
                fprintf (stderr, "%s\n", error.message);
           }

           bson_destroy (insert);
           bson_destroy (&reply);
           bson_destroy (command);
           bson_free (str);
         */

        /*
         * Release our handles and clean up libmongoc
         */
        /*
           mongoc_collection_destroy (collection);
           mongoc_database_destroy (database);
           mongoc_client_destroy (client);
           mongoc_cleanup ();
         */

        return 0;
}

static int disconnect(){
        /*
         * Release our handles and clean up libmongoc
         */
        mongoc_collection_destroy (collection);
        mongoc_database_destroy (database);
        mongoc_client_destroy (client);
        mongoc_cleanup ();

        return 0;
}

static bson_t create_document(struct stat *si, char * path){
        // the data stored in mongodb should be string
        char str_st_dev[256];
        char str_st_ino[256];
        char str_st_mode[256];
        char str_st_nlink[128];
        char str_st_uid[128];
        char str_st_gid[128];
        char str_st_rdev[256];
        char str_st_size[256];
        char str_st_blksize[256];
        char str_st_blocks[256];
        char str_st_atime[128];
        char str_st_mtime[128];
        char str_st_ctime[128];
        /* store a long number to represent the time
           struct tm * last_access;
           struct tm * last_modif;
           struct tm * last_stat_change;
         */

        // get data in struct stat
        sprintf(str_st_dev, "%lld", si->st_dev);
        sprintf(str_st_ino, "%lld", si->st_ino);
        sprintf(str_st_mode, "%o", si->st_mode);
        sprintf(str_st_nlink, "%d", si->st_nlink);
        sprintf(str_st_uid, "%d", si->st_uid);
        sprintf(str_st_gid, "%d", si->st_gid);
        sprintf(str_st_rdev, "%lld", si->st_rdev);
        sprintf(str_st_size, "%lld", si->st_size);
        sprintf(str_st_blksize, "%ld", si->st_blksize);
        sprintf(str_st_blocks, "%lld", si->st_blocks);
        sprintf(str_st_atime, "%d", si->st_atime);
        sprintf(str_st_mtime, "%d", si->st_mtime);
        sprintf(str_st_ctime, "%d", si->st_ctime);

        bson_t document = BCON_NEW(
                "path", BCON_UTF8(path),
                "st_dev", BCON_UTF8(str_st_dev),
                "st_ino", BCON_UTF8(str_st_ino),
                "st_mode", BCON_UTF8(str_st_mode),
                "st_nlink" BCON_UTF8(str_st_nlink),
                "st_uid", BCON_UTF8(str_st_uid),
                "st_gid", BCON_UTF8(str_st_gid),
                "st_rdev", BCON_UTF8(str_st_rdev);
                "st_size", BCON_UTF8(str_st_size),
                "st_blksize", BCON_UTF8(str_st_blksize),
                "st_blocks",BCON_UTF8(str_st_blocks),
                "st_atime", BCON_UTF8(str_st_atime),
                "st_mtime", BCON_UTF8(str_st_mtime),
                "st_ctime", BCON_UTF8(str_st_ctime)
                );
        return document;
}

static int insert(bson_t * document){
  bson_error_t error;
  // TODO: I am not sure what will be returned
  if(find_by_path(document)){
    modify();
  }else{
    if(!mongoc_collection_insert(_get_collection("nosqlFS", "file"), MONGOC_INSERT_NONE, document, NULL, &error)){
      fprintf(stderr, "%s\n", error.message);
    }
  }
  bson_destroy(document);
  return 0;
}

static void delete(){

}

static void modify(){

}
/*
  original name of this function is search,
  I change that because find is used by mongodb
*/
// there are two find function. this is because it's diffcult to implement a override in C

static void find_by_path(bson_t * document){
  bson_t * document = bson_new();
  bson_append_utf8(document, BCON_UTF8("path"), strlen("path"), BCON_UTF8(value[i]), strlen(value[i]));
  mongoc_cursor_t * cursor = mongoc_collextion_find(_get_collection("nosqlFS", "file"), MONGOC_QUERY_NONE, 0, 0, 0, document, NULL, NULL);
  // TODO: not sure what should be return. a list of char *, or, a list of bson_t. Both of them need a customed list
}

static void find_by_document(bson_t * document){
  mongoc_cursor_t * cursor = mongoc_collextion_find(_get_collection("nosqlFS", "file"), MONGOC_QUERY_NONE, 0, 0, 0, document, NULL, NULL);
  // TODO: not sure what should be return. a list of char *, or, a list of bson_t. Both of them need a customed list
}

static void find_by_key(int argc, char ** field, char ** value){
  bson_t * document = bson_new();
  for(int i = 0; i < argc; ++i){
    bson_append_utf8(document, BCON_UTF8(field[i]), strlen(field[i]), BCON_UTF8(value[i]), strlen(value[i]));
  }
  mongoc_cursor_t * cursor = mongoc_collextion_find(_get_collection("nosqlFS", "file"), MONGOC_QUERY_NONE, 0, 0, 0, document, NULL, NULL);
  // TODO: not sure what should be return. a list of char *, or, a list of bson_t. Both of them need a customed list
}
