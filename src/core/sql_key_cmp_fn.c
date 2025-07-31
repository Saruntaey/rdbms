#include <string.h>
#include "BPlusTree.h"
#include "sql_enum.h"

int sql_key_cmp_fn(BPluskey_t *key1, BPluskey_t *key2, key_mdata_t* key_mdata, int key_mdata_size) {
    if (!key1 || !key1->key || !key1->key_size) return 1;
    if (!key2 || !key2->key || !key2->key_size) return -1;
	char *key1_ptr = (char *) key1->key;
	char *key2_ptr = (char *) key2->key;
	int offset = 0;

	for (int i = 0; i < key_mdata_size; i++) {
		int dtype = key_mdata[i].dtype;
		int size = key_mdata[i].size;
		switch (dtype) {
			case SQL_STRING:
			{
				int d = strncmp(key1_ptr + offset, key2_ptr + offset, size);
				if (d < 0) return 1;
				if (d > 0) return -1;
				break;
			}
			case SQL_INT:
			{
				int *n1 = (int *) (key1_ptr + offset);
				int *n2 = (int *) (key2_ptr + offset);
				if (*n1 < *n2) return 1;
				if (*n1 > *n2) return -1;
				break;
			}
			case SQL_DOUBLE:
			{
				double *n1 = (double *) (key1_ptr + offset);
				double *n2 = (double *) (key2_ptr + offset);
				if (*n1 < *n2) return 1;
				if (*n1 > *n2) return -1;
				break;
			}
		}
		offset += size;
	}
	return 0;
}
