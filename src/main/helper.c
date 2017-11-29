#include "helper.h"

#include <bson.h>
#include <bcon.h>
#include <time.h>
#include <string.h>

#include "log.h"
#include "db_manager.h"

void record_file_basic_info(const char * path, struct stat * stbuf){
	// get time string
	//log_msg("Try to get time\n");
	char str_last_modification[32];
	int int_last_modification;
    	sprintf(str_last_modification, "%ld", stbuf->st_mtime);
    	int_last_modification = stbuf->st_mtime;
    	//log_msg("Try to create query\n");
	bson_t * query = document_create_query_file(path);
	//log_msg("Created query\n");
	char * result = find(query);
	//log_msg("Get result and compare: %s\n", path);
	
	//log_msg("JSON: \n %s \n", result);

	if(result == NULL){
		//log_msg("Ready to insert document\n");
		bson_t * doc = document_create_file(int_last_modification, path);
		insert(doc);
		//log_msg("Finish insert docuemnt\n");
	}else if(strstr(result, str_last_modification) == NULL){
		// TODO: update it!
		bson_t * update_doc = document_create_update(int_last_modification);
		//log_msg("Ready to update\n");
		if(update(query, update_doc) != 0){
			//log_msg("ERROR in update\n");
		}
		//log_msg("Finish update\n");
		bson_destroy(update_doc);
	}
	bson_destroy(query);
	bson_free(result);
}

/*
void record_file_basic_info(const char * path, struct stat * stbuf){
	// get time string
	char str_last_modification[32];
	int int_last_modification;
    	sprintf(str_last_modification, "%ld", stbuf->st_mtime);
    	int_last_modification = stbuf->st_mtime;
	
	bson_t * doc = document_create_file(int_last_modification, path);
	add2bulk(doc);
	//log_msg("add path: %s\n", path);
}

*/