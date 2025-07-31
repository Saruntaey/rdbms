#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "catalog.h"

extern int sql_key_cmp_fn(BPluskey_t *key1, BPluskey_t *key2, key_mdata_t* key_mdata, int key_mdata_size);
static void catalog_rec_free_fn(void *rec); 
BPlusTree_t *new_schema_table(sql_create_data *create_data, bool *ok);
BPlusTree_t *new_record_table(sql_create_data *create_data, bool *ok);
key_mdata_t *construct_record_key_mdata(sql_create_data *create_data, int *key_mdata_size);

BPlusTree catalog;

bool catalog_insert_new_table (BPlusTree_t *catalog, sql_create_data *create_data) {
	catalog_rec *cat_rec;
	BPlusTree_t *schema_table;
	BPlusTree_t *record_table;
	static bool is_init_catalog = false;
	static key_mdata_t catalog_key_mdata[] = {{SQL_STRING, TABLE_NAME_SIZE}};
	BPluskey table_name_key;
	char *table_name;
	bool ok;

	if (!is_init_catalog) {
		memset(catalog, 0, sizeof(*catalog));
		BPlusTree_init(
			catalog,
			sql_key_cmp_fn,
			NULL,
			NULL,
			BTREE_CATALOG_MAX_CHILD,
			catalog_rec_free_fn,
			catalog_key_mdata,
			sizeof(catalog_key_mdata)/sizeof(catalog_key_mdata[0])
		);
		is_init_catalog = true;
	}
	table_name = (char *) calloc(sizeof(*table_name), TABLE_NAME_SIZE);
	table_name_key.key = (void *) table_name;
	table_name_key.key_size = TABLE_NAME_SIZE;
	strncpy(table_name, create_data->table_name, TABLE_NAME_SIZE - 1);
	table_name[TABLE_NAME_SIZE - 1] = '\0';
	if (BPlusTree_Query_Key(catalog, &table_name_key)) {
		printf("error: table %s already exist\n", table_name);
		free(table_name);
		return false;
	}

	schema_table = new_schema_table(create_data, &ok); 
	if (!ok) {
		printf("error: fail to create schema table\n");
		free(table_name);
		return false;
	}

	record_table = new_record_table(create_data, &ok);
	if (!ok) {
		printf("error: fail to create record table\n");
		free(table_name);
		return false;
	}

	cat_rec = (catalog_rec *) calloc(sizeof(*cat_rec), 1);
	strncpy(cat_rec->table_name, create_data->table_name, TABLE_NAME_SIZE - 1);
	cat_rec->table_name[TABLE_NAME_SIZE - 1] = '\0';
	cat_rec->schema_table = schema_table;
	cat_rec->record_table = record_table;
	int i;
	for (i = 0; i < create_data->n_cols; i++) {
		assert(i < MAX_COLS_PER_TABLE);
		strncpy(cat_rec->col_name[i], create_data->cols[i].name, COL_NAME_SIZE - 1);
		cat_rec->col_name[i][COL_NAME_SIZE - 1] = '\0';
	}
	cat_rec->col_name[i][0] = '\0'; // indicate end of columns
	BPlusTree_Insert(catalog, &table_name_key, (void *) cat_rec);
	printf("created table\n");
	catalog_table_print(catalog);
	return true;
}


BPlusTree_t *new_schema_table(sql_create_data *create_data, bool *ok) {
	BPlusTree_t *schema_table;
	schema_rec *rec;
	BPluskey key;
	char *key_buffer;
	int offset;
	static key_mdata_t key_mdata[] = {{SQL_STRING, COL_NAME_SIZE}};

	*ok = true;
	schema_table = (BPlusTree_t *) calloc(1, sizeof(*schema_table));
	BPlusTree_init(
		schema_table,
		sql_key_cmp_fn,
		NULL,
		NULL,
		BTREE_SCHEMA_MAX_CHILD,
		free,
		key_mdata,
		sizeof(key_mdata)/sizeof(key_mdata[0])
	);
	offset = 0;
	for(int i = 0; i < create_data->n_cols; i++) {
		assert(i < MAX_COLS_PER_TABLE);
		key_buffer = (char *) calloc(1, COL_NAME_SIZE);
		strncpy(key_buffer, create_data->cols[i].name, COL_NAME_SIZE - 1);
		key_buffer[COL_NAME_SIZE - 1] = '\0';
		key.key = (void *) key_buffer;
		key.key_size = COL_NAME_SIZE;
		rec = (schema_rec *) calloc(1, sizeof(*rec));
		strncpy(rec->col_name, create_data->cols[i].name, COL_NAME_SIZE - 1);
		rec->col_name[COL_NAME_SIZE - 1] = '\0';
		rec->dtype = create_data->cols[i].type;
		rec->size = create_data->cols[i].size;
		rec->is_primary_key = create_data->cols[i].is_primary_key;
		rec->offset = offset;
		offset += create_data->cols[i].size;
		assert(BPlusTree_Insert(schema_table, &key, (void *) rec));
	}
	return schema_table;
}

BPlusTree_t *new_record_table(sql_create_data *create_data, bool *ok) {
	BPlusTree_t *record_table;
	key_mdata_t *key_mdata;
	int key_mdata_size;
	BPluskey key;
	
	*ok = true;
	key_mdata = construct_record_key_mdata(create_data, &key_mdata_size);
	if (!key_mdata) {
		*ok = false;
		printf("error: no primary key\n");
		return NULL;
	}
	record_table = (BPlusTree_t *) calloc(1, sizeof(*record_table));
	BPlusTree_init(
		record_table,
		sql_key_cmp_fn,
		NULL,
		NULL,
		BTREE_RECORD_MAX_CHILD,
		free,
		key_mdata,
		key_mdata_size
	);
	
	return record_table;
}

key_mdata_t *construct_record_key_mdata(sql_create_data *create_data, int *key_mdata_size) {
	key_mdata_t *key_mdata;
	int primary_keys;
	int i;
	int j;

	primary_keys = 0;
	for (i = 0; i < create_data->n_cols; i++) {
		if (create_data->cols[i].is_primary_key) {
			primary_keys++;
		}
	}
	if (primary_keys == 0) {
		return NULL;
	}
	key_mdata = (key_mdata_t *) calloc(primary_keys, sizeof(*key_mdata));
	j = 0;
	for (i = 0; i < create_data->n_cols; i++) {
		if (create_data->cols[i].is_primary_key) {
			key_mdata[j++] = {create_data->cols[i].type, create_data->cols[i].size};
		}
	}
	*key_mdata_size = j;
	return key_mdata;
}

void catalog_rec_free_fn(void *rec) {
	catalog_rec * cat_rec = (catalog_rec *) rec;
	BPlusTree_Destroy(cat_rec->schema_table);
	free(cat_rec->schema_table);
	BPlusTree_Destroy(cat_rec->record_table);
	free(cat_rec->record_table);
	free(rec);
}

void catalog_table_print(BPlusTree_t *catalog) {
    int i = 0;
    void *rec;
    BPluskey_t *bpkey;
    catalog_rec *ctable_val;
    BPlusTree_t *schema_table;

    BPTREE_ITERATE_ALL_RECORDS_BEGIN(catalog, bpkey, rec) {
        printf ("Record Table Name = %s\n", (char *)bpkey->key);
        ctable_val = (catalog_rec *)rec;
        printf ("Schema Table : \n");
        schema_table_print(ctable_val->schema_table);
        printf ("Record Table Ptr : %p\n", ctable_val->record_table);
        printf ("Column List : \n");
        i = 0;
        while (ctable_val->col_name[i][0] != '\0') {
            printf ("%s ", ctable_val->col_name[i]);
            i++;
        }
        printf ("\n======\n");
       
    } BPTREE_ITERATE_ALL_RECORDS_END(catalog, bpkey, rec);
}

void schema_table_print(BPlusTree_t *schema_table) {
    void *rec;
    BPluskey_t *bpkey;
    schema_rec *schema;

    BPTREE_ITERATE_ALL_RECORDS_BEGIN(schema_table, bpkey, rec) {
        schema = (schema_rec *) rec;
        printf ("Column Name : %s  Dtype = %d  Dtype Len = %d  Is_Primary_key = %s  offset = %d\n",
            (char *)bpkey->key,
            schema->dtype,
            schema->size,
            schema->is_primary_key ? "Y" : "N" ,
            schema->offset);
    } BPTREE_ITERATE_ALL_RECORDS_END(schema_table, bpkey, rec);
}

