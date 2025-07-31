#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "sql_drop.h"
#include "catalog.h"

void drop_table(char *table_name) {
	BPluskey_t key;
	char key_buffer[TABLE_NAME_SIZE];

	strncpy(key_buffer, table_name, TABLE_NAME_SIZE - 1);
	key_buffer[TABLE_NAME_SIZE - 1] = '\0';
	key.key = key_buffer;
	key.key_size = TABLE_NAME_SIZE;
	if (!BPlusTree_Query_Key(&catalog, &key)) {
		printf("error: table %s not exist\n", key_buffer);
		return;
	}
	assert(BPlusTree_Delete(&catalog, &key));
	printf("droped table %s\n", key_buffer);
}
