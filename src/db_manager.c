#include <bson.h>
#include <bcon.h>
#include <mongoc.h>

static mongoc_client_t * client;
static mongoc_database_t * database;
static mongoc_collection_t * collection;


//TODO: all this operation should be thread-safe
static mongoc_client_t * _get_client(){
  if(client == NULL){
    client = mongoc_client_new ("mongodb://localhost:27017");
  }
  return client;
}

static mongoc_database_t * _get_database(mongoc_client_t * client, char * database_name){
  if(database == NULL){
    database = mongoc_client_get_database (client, database_name);
  }
  return database;
}

static mongoc_collection_t * _get_collection(mongoc_client_t * client, char * database_name, char * collection_name){
  if(collection == NULL){
    collection = mongoc_client_get_collection (client, database_name, collection_name);
  }
}

static int db_init(){
  // mongoc_client_t      *client;
  // mongoc_database_t    *database;
  // mongoc_collection_t  *collection;
  bson_t               *command,
                         reply,
                        *insert;
  bson_error_t          error;
  char                 *str;
  bool                  retval;

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

  /*
   * Release our handles and clean up libmongoc
   */
  mongoc_collection_destroy (collection);
  mongoc_database_destroy (database);
  mongoc_client_destroy (client);
  mongoc_cleanup ();

  return 0;
}

static void create_document(){

}

static void insert(){

}

static void delete(){

}

static void modify(){

}

static void search(){

}
