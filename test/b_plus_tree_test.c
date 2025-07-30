#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BPlusTree.h"


typedef enum dtype {
	DTYPE_STRING,
	DTYPE_INT,
	DTYPE_DOUBLE,
} dtype;

int bplus_tree_comp_fn(BPluskey_t *key1, BPluskey_t *key2, key_mdata_t* key_mdata, int key_mdata_size) {
    if (!key1 || !key1->key || !key1->key_size) return 1;
    if (!key2 || !key2->key || !key2->key_size) return -1;
	char *key1_ptr = (char *) key1->key;
	char *key2_ptr = (char *) key2->key;
	int offset = 0;

	for (int i = 0; i < key_mdata_size; i++) {
		int dtype = key_mdata[i].dtype;
		int size = key_mdata[i].size;
		switch (dtype) {
			case DTYPE_STRING:
			{
				int d = strncmp(key1_ptr + offset, key2_ptr + offset, size);
				if (d < 0) return 1;
				if (d > 0) return -1;
				break;
			}
			case DTYPE_INT:
			{
				int *n1 = (int *) (key1_ptr + offset);
				int *n2 = (int *) (key2_ptr + offset);
				if (*n1 < *n2) return 1;
				if (*n1 > *n2) return -1;
				break;
			}
			case DTYPE_DOUBLE:
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

int main(void) {
	int choice;
	char discard_buffer[2];
	BPlusTree_t tree;
	key_mdata_t key_mdata[] = {{DTYPE_STRING, 32}};

	memset(&tree, 0, sizeof(tree));
	BPlusTree_init(
		&tree,
		bplus_tree_comp_fn,
		NULL,
		NULL,
		4,
		NULL,
		key_mdata,
		sizeof(key_mdata)/sizeof(key_mdata[0])
	);
	while (1) {
		printf ("1. Insert\n");
		printf ("2. Delete\n");
		printf ("3. Update\n");
		printf ("4. Read\n");
		printf ("5. Destroy\n");
		printf ("6. Iterate over all Records\n");
		printf ("7. exit\n");
		scanf("%d", &choice);
		fgets(discard_buffer, sizeof(discard_buffer), stdin);
		fflush(stdin);
		switch (choice) {
			case 1:
			{
				BPluskey_t key;
				key.key_size = 32;
				key.key = calloc(1, key.key_size);
				printf("Insert key: ");
				fgets((char *)key.key, key.key_size, stdin);
				((char *) key.key)[strcspn((char *) key.key, "\n")] = '\0';
				char *value = (char *) calloc(1, 32);
				printf("Insert value: ");
				fgets(value, 32, stdin);
				BPlusTree_Insert(&tree, &key, (void *) value);
				break;
			}
			case 2:
			{
				BPluskey_t key;
				char key_buffer[32];
				key.key_size = 32;
				key.key = key_buffer;
				printf("Delete key: ");
				fgets((char *)key.key, key.key_size, stdin);
				((char *) key.key)[strcspn((char *) key.key, "\n")] = '\0';
				BPlusTree_Delete(&tree, &key);
				break;
			}
			case 3:
			{
				BPluskey_t key;
				char key_buffer[32];
				key.key_size = 32;
				key.key = key_buffer;
				printf("Update key: ");
				fgets((char *)key.key, key.key_size, stdin);
				((char *) key.key)[strcspn((char *) key.key, "\n")] = '\0';
				char *old_value = (char *) BPlusTree_Query_Key(&tree, &key);
				if (!old_value) {
					printf("Not found old value\n");
					break;
				}
				printf("Old value: %s\n", old_value);
				printf("Set new value: ");
				char *new_value = (char *) calloc(32, 1);
				fgets(new_value, 32, stdin);
				new_value[strcspn(new_value, "\n")] = '\0';
				BPlusTree_Modify(&tree, &key, new_value);
				break;
			}
			case 4:
			{
				BPluskey_t key;
				char key_buffer[32];
				key.key_size = 32;
				key.key = key_buffer;
				printf("Read key: ");
				fgets((char *)key.key, key.key_size, stdin);
				((char *) key.key)[strcspn((char *) key.key, "\n")] = '\0';
				char *value = (char *) BPlusTree_Query_Key(&tree, &key);
				if (!value) {
					printf("Not found\n");
					break;
				}
				printf("Value: %s\n", value);
				break;
			}
			case 5:
			{
				BPlusTree_Destroy(&tree);
				printf("Destroy B+ Tree successfully\n");
				break;
			}
			case 6:
			{
				BPluskey_t *key_ptr;
				void *rec_ptr;
				BPTREE_ITERATE_ALL_RECORDS_BEGIN((&tree), key_ptr, rec_ptr) {
					printf("key = %s, val = %s\n", (char *) key_ptr->key, (char *) rec_ptr);
				} BPTREE_ITERATE_ALL_RECORDS_END(&tree, key_ptr, rec_ptr);
				break;
			}
			case 7:
				return 0;
		}
	}
}
