#include "sql_create.h"
#include "catalog.h"

sql_create_data create_data;


void process_create_query() {
	catalog_insert_new_table(&catalog, &create_data);
}
