#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "sql_insert_into.h"
#include "BPlusTree.h"
#include "catalog.h"

sql_insert_data insert_data;

static void sql_print_record (BPlusTree_t *schema_table, void *record);

void process_insert_into_query() {
	BPluskey_t key;
	BPluskey_t key2;
	BPluskey_t *key_ptr;
	catalog_rec *cat_rec;
	BPlusTree *schema_table;
	BPlusTree *record_table;
	void *rec;
	schema_rec *sr;
	int key_size;
	int rec_size;
	int i;
	sql_val *v;
	int offset;

	key.key = insert_data.table_name;
	key.key_size = TABLE_NAME_SIZE;
	cat_rec = (catalog_rec *) BPlusTree_Query_Key(&catalog, &key);
	if (!cat_rec) {
		printf("error: table %s not exist\n", insert_data.table_name);
		return;
	}
	schema_table = cat_rec->schema_table;
	record_table = cat_rec->record_table;

	key_size = 0;
	rec_size = 0;
	BPTREE_ITERATE_ALL_RECORDS_BEGIN(schema_table, key_ptr, rec) {
		sr = (schema_rec *) rec;
		if (sr->is_primary_key) {
			key_size += sr->size;
		}
		rec_size += sr->size;
	} BPTREE_ITERATE_ALL_RECORDS_END(schema_table, key_ptr, rec);

	key.key = calloc(1, key_size);
	key.key_size = key_size;
	offset = 0;
	for (i = 0; cat_rec->col_name[i][0] != '\0'; i++) {
		v = &insert_data.cols[i];
		key2.key = cat_rec->col_name[i];
		key2.key_size = COL_NAME_SIZE;
		sr = (schema_rec *) BPlusTree_Query_Key(schema_table, &key2);
		if (!sr->is_primary_key) continue;
		switch (sr->dtype) {
			case SQL_INT:
				memcpy((char *)key.key + offset, &v->val.i, sr->size);
				break;
			case SQL_DOUBLE:
				memcpy((char *)key.key + offset, &v->val.d, sr->size);
				break;
			case SQL_STRING:
				strncpy((char *)key.key + offset, v->val.str, sr->size);
				break;
			default:
				assert(0);
		}
		offset += sr->size;
	}

	if (BPlusTree_Query_Key(record_table, &key)) {
		printf("error: duplicate key\n");
		free(key.key);
		return;
	}

	rec = calloc(1, rec_size);
	for (i = 0; cat_rec->col_name[i][0] != '\0'; i++) {
		v = &insert_data.cols[i];
		key2.key = cat_rec->col_name[i];
		key2.key_size = COL_NAME_SIZE;
		sr = (schema_rec *) BPlusTree_Query_Key(schema_table, &key2);
		switch (sr->dtype) {
			case SQL_INT:
				memcpy((char *)rec + sr->offset, &v->val.i, sr->size);
				break;
			case SQL_DOUBLE:
				memcpy((char *)rec + sr->offset, &v->val.d, sr->size);
				break;
			case SQL_STRING:
				strncpy((char *)rec + sr->offset, v->val.str, sr->size);
				break;
			default:
				assert(0);
		}
	}
	assert(BPlusTree_Insert(record_table, &key, rec));
	printf("inserted\n");
	sql_print_record (schema_table, rec);
	printf("\n");
}

void sql_print_record (BPlusTree_t *schema_table, void *record) {
    /* The fn to print the record individual fields, not necessarily in the order
        in which they exist in a record */
    void *rec;
    BPluskey_t *bpkey_ptr;
    schema_rec *sr;

    BPTREE_ITERATE_ALL_RECORDS_BEGIN(schema_table, bpkey_ptr, rec) {
        sr = (schema_rec *)rec;
        switch (sr->dtype) {
            case SQL_STRING:
                printf ("%s ", (char *)record + sr->offset);
                break;
            case SQL_INT:
                printf ("%d ", *(int *)((char *)record + sr->offset));
                break;
            case SQL_DOUBLE:
                printf ("%lf ", *(double *)((char *)record + sr->offset));
                break;
            default:
                assert(0);
        }
    } BPTREE_ITERATE_ALL_RECORDS_END(schema_table, bpkey_ptr, rec);
}
