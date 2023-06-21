/* @file hashtable.c
 * @brief This hash table was inspired by Jacob Sorbers hash table video
 *
 * Code reference: https://www.youtube.com/watch?v=KI_V91UdL1I
 *
 */

#include "hashtable.h"
#include <pthread.h>
#include <string.h>

#define FAIL_CODE -1
#define SUCCESS_CODE 1
#define MAX_KEY_LENGTH 256
#define MAX_TABLE_SIZE 10000

/**
 * @brief entry struct
 * @param struct entry *next
 * @NOTE: *next pointer used because external chaining is used
 * which is used when there is a collision of two keys. When
 * there is a collision, the next pointer is used to point to
 * the next entry in the list.
 */
typedef struct entry {
  char *key;
  size_t keylength;
  void *object;
  struct entry *next; // *next pointer used because external chaining is used.
} entry;

typedef struct _hash_table {
  uint32_t size;             // size of the table
  hashfunction *hash;        // hash function to use
  cleanup_function *cleanup; // cleanup function to use
  entry **elements;          // an array of pointers to entries
  pthread_mutex_t hash_lock; // mutex lock for the hash table
} hashtable_t;

/**
 * @brief Calculates the index in the hash table for a given key.
 *
 * @param hash_table_t p_ht The hash table object.
 * @param const char * p_key The key to calculate the index for.
 * @return The index in the hash table for the given key.
 */
static size_t hash_table_index(hash_table_t *p_ht, const char *p_key) {
  size_t result = FAIL_CODE;
  if (NULL == p_ht) {
    fprintf(stderr, "hash_table_index: p_ht is NULL");
    goto EXIT;
  }
  if (NULL == p_key) {
    fprintf(stderr, "hash_table_index: p_key is NULL");
    goto EXIT;
  }
  if ((strlen(p_key) > MAX_KEY_LENGTH)) {
    fprintf(stderr, "hash_table_index: p_key is invalid");
    goto EXIT;
  }

  result = (p_ht->hash(p_key, strlen(p_key))) % p_ht->size;

EXIT:
  return result;
} /* hash_table_index() */

hash_table_t *hash_table_create(uint32_t size, hashfunction *p_hf,
                                cleanup_function *p_cf) {
  hash_table_t *p_ht = NULL;
  if (MAX_TABLE_SIZE < size) {
    fprintf(stderr, "hash_table_create: size is too large");
    goto EXIT;
  }
  if (NULL == p_hf) {
    fprintf(stderr, "hash_table_create: p_hf is NULL");
    goto EXIT;
  }
  if (NULL == p_cf) {
    fprintf(stderr, "hash_table_create: p_cf is NULL");
    goto EXIT;
  }

  p_ht = calloc(1, sizeof(hash_table_t));
  if (NULL == p_ht) {
    fprintf(stderr, "hash_table_create: calloc failed");
    goto EXIT;
  }
  // initialize pthread mutex hash lock
  if (pthread_mutex_init(&p_ht->hash_lock, NULL) != 0) {
    fprintf(stderr, "hash_table_create: pthread_mutex_init failed");
    goto ERR;
  }
  p_ht->size = size;
  p_ht->hash = p_hf;
  p_ht->cleanup = p_cf;
  p_ht->elements = calloc(sizeof(entry *), p_ht->size);
  if (NULL == p_ht->elements) {
    fprintf(stderr, "hash_table_create: calloc failed\n");
    goto ERR;
  }
  goto EXIT;

ERR:
  free(p_ht);
  p_ht = NULL;
EXIT:
  return p_ht;
} /* hash_table_create() */

void hash_table_destroy(hash_table_t *p_ht) {
  if (NULL == p_ht) {
    fprintf(stderr, "hash_table_destroy: hash table is NULL\n");
    goto EXIT;
  }
  for (uint32_t i = 0; i < p_ht->size; i++) {
    while (NULL != p_ht->elements[i]) {
      entry *temp = p_ht->elements[i];
      p_ht->elements[i] = temp->next;
      free(temp->key);
      temp->key = NULL;
      p_ht->cleanup(temp->object);
      temp->object = NULL;
      free(temp);
      temp = NULL;
    }
  }

  pthread_mutex_destroy(&p_ht->hash_lock);

  free(p_ht->elements);
  p_ht->elements = NULL;
  free(p_ht);
  p_ht = NULL;

EXIT:
  return;
} /* hash_table_destroy() */

void hash_table_print(hash_table_t *p_ht) {
  if (NULL == p_ht) {
    fprintf(stderr, "hash_table_print: hash table is NULL\n");
    goto EXIT;
  }
  printf("start table\n");

  for (uint32_t i = 0; i < p_ht->size; i++) {
    if (p_ht->elements[i] != NULL) {
      printf("\t%i\t", i);
      entry *tmp = p_ht->elements[i];
      while (NULL != tmp) {
        printf("\"%s\"(%p) - ", tmp->key, tmp->object);
        { print_object(tmp->object); }
        tmp = tmp->next;
      }
      printf("\n");
    }
  }
  printf("end table\n");

EXIT:
  return;
} /* hash_table_print() */

int copy_keys_to_string(hash_table_t *p_ht, char *output) {
  int ret_code = FAIL_CODE;
  if (NULL == p_ht) {
    fprintf(stderr, "copy_keys_to_string: hash table is NULL\n");
    goto EXIT;
  }

  if (NULL == output) {
    fprintf(stderr, "copy_keys_to_string: output char* is NULL\n");
    goto EXIT;
  }

  // iterate the entire list and add a null byte between the strings
  size_t index = 0;
  for (uint32_t i = 0; i < p_ht->size; i++) {
    if (p_ht->elements[i] != NULL) {
      entry *tmp = p_ht->elements[i];
      while (NULL != tmp) {
        size_t key_length = strlen(tmp->key);
        for (size_t j = 0; j < key_length; j++) {
          output[index] = tmp->key[j];
          index++;
        }
        output[index] = '\0'; // Assign null byte directly
        index++;
        tmp = tmp->next;
      }
    }
  }

  ret_code = SUCCESS_CODE;
EXIT:
  return ret_code;
} /* copy_keys_to_string() */

int hash_table_insert(hash_table_t *p_ht, const char *p_key, void *obj) {
  int ret_code = FAIL_CODE;
  if (NULL == p_ht) {
    fprintf(stderr, "hash_table_insert: p_ht is NULL\n");
    goto EXIT;
  }
  if (NULL == p_key) {
    fprintf(stderr, "hash_table_insert: p_key is NULL\n");
    goto EXIT;
  }
  if (NULL == obj) {
    fprintf(stderr, "hash_table_insert: obj is NULL\n");
    goto EXIT;
  }

  size_t index = hash_table_index(p_ht, p_key);
  size_t check_index = -1;
  if (check_index == index) {
    fprintf(stderr, "hash_table_insert: hash_table_index failed\n");
    goto EXIT;
  }

  void *entry_check = (hash_table_lookup(p_ht, p_key));
  if (NULL != entry_check) {
    fprintf(stderr, "hash_table_insert: entry already exists\n");
    // We aren't failing here, a fail code will cause our program to shut down
    // we just can't have a duplicate key
    ret_code = SUCCESS_CODE;
    goto EXIT;
  }

  entry *p_entry = calloc(1, sizeof(entry));
  if (NULL == p_entry) {
    fprintf(stderr, "hash_table_insert: calloc failed\n");
    goto EXIT;
  }
  p_entry->object = obj;
  p_entry->next = NULL;
  p_entry->key = strdup(p_key);
  if (NULL == p_entry->key) {
    fprintf(stderr, "hash_table_insert: strdup failed\n");
    goto ERR;
  }

  p_entry->next = p_ht->elements[index];
  p_ht->elements[index] = p_entry;
  ret_code = SUCCESS_CODE;
  goto EXIT;
ERR:
  free(p_entry);
  p_entry = NULL;
EXIT:
  return ret_code;
} /* hash_table_insert() */

void *hash_table_lookup(hash_table_t *p_ht, const char *p_key) {
  void *object = NULL;
  if (NULL == p_ht) {
    fprintf(stderr, "hash_table_lookup: p_ht is NULL\n");
    goto EXIT;
  }
  if (NULL == p_key) {
    fprintf(stderr, "hash_table_lookup: p_key is NULL\n");
    goto EXIT;
  }

  size_t index = hash_table_index(p_ht, p_key);
  size_t check_index = -1;
  if (check_index == index) {
    fprintf(stderr, "hash_table_lookup: hash_table_index failed\n");
    goto EXIT;
  }

  entry *p_entry = p_ht->elements[index];
  while (NULL != p_entry) {
    if (0 == strcmp(p_entry->key, p_key)) {
      object = p_entry->object;
      goto EXIT;
    }
    p_entry = p_entry->next;
  }

EXIT:
  return object;
} /* hash_table_lookup() */

void *hash_table_remove(hash_table_t *p_ht, const char *key) {
  void *removed_object = NULL;

  if (NULL == p_ht) {
    fprintf(stderr, "hash_table_remove: hash table is NULL\n");
    goto EXIT;
  }

  uint32_t hash_value = hash_function(key, strlen(key)) % p_ht->size;
  entry *current_entry = p_ht->elements[hash_value];
  entry *previous_entry = NULL;

  while (NULL != current_entry) {
    if (strcmp(current_entry->key, key) == 0) {
      pthread_mutex_lock(&p_ht->hash_lock);
      if (previous_entry == NULL) {
        // Entry is the first element in the linked list
        p_ht->elements[hash_value] = current_entry->next;
      } else {
        // Entry is not the first element, update the previous entry's next
        // pointer
        previous_entry->next = current_entry->next;
      }

      removed_object = current_entry->object;
      free(current_entry->key);
      current_entry->key = NULL;
      free(current_entry);
      current_entry = NULL;
      pthread_mutex_unlock(&p_ht->hash_lock);
      break;
    }

    previous_entry = current_entry;
    current_entry = current_entry->next;
  }
EXIT:
  return removed_object;
} /* hash_table_remove() */

uint64_t hash_function(const char *p_key, unsigned long length) {
  if (NULL == p_key) {
    fprintf(stderr, "hash_function: name is NULL\n");
    return 0;
  }
  if (MAX_KEY_LENGTH < length) {
    fprintf(stderr, "hash_function: length is too long\n");
    return 0;
  }

  uint64_t hash_value = 0;
  for (size_t i = 0; i < length; i++) {
    hash_value += p_key[i];
    hash_value = hash_value * p_key[i];
  }
  return hash_value;
} /* hash_function() */

int return_all_matching_keys(hash_table_t *p_ht, char *store_keys,
                             char *key_to_find, int user_privilege) {
  int ret_code = FAIL_CODE;
  if (NULL == p_ht) {
    fprintf(stderr, "return_all_matching_keys: p_ht is NULL\n");
    goto EXIT;
  }
  if (NULL == store_keys) {
    fprintf(stderr, "return_all_matching_keys: store_keys is NULL\n");
    goto EXIT;
  }
  if (NULL == key_to_find) {
    fprintf(stderr, "return_all_matching_keys: key_to_find is NULL\n");
    goto EXIT;
  }

  int key_match = FAIL_CODE;
  size_t index = 0;
  for (uint32_t i = 0; i < p_ht->size; i++) {
    if (p_ht->elements[i] != NULL) {
      entry *tmp = p_ht->elements[i];
      while (NULL != tmp) {
        void *object = hash_table_lookup(p_ht, tmp->key);
        key_match = check_match(object, key_to_find, user_privilege);
        if (SUCCESS_CODE == key_match) {
          size_t key_length = strlen(tmp->key);
          for (size_t j = 0; j < key_length; j++) {
            store_keys[index] = tmp->key[j];
            index++;
          }
          // Assign null byte directly after each key
          store_keys[index] = '\0';
          index++;
        }
        tmp = tmp->next;
        object = NULL;
      }
    }
  }
  ret_code = SUCCESS_CODE;
EXIT:
  return ret_code;
} /* return_all_matching_keys() */

int load_file_to_hash(FILE *fp, hash_table_t *p_ht) {

  int ret_code = FAIL_CODE;
  if (NULL == fp) {
    fprintf(stderr, "copy_file_to_hash: fp is NULL\n");
    goto EXIT;
  }
  if (NULL == p_ht) {
    fprintf(stderr, "copy_file_to_hash: p_ht is NULL\n");
    goto EXIT;
  }
  char sep = '\x1E';
  char buffer[MAX_FILE_SIZE];

  char object_buffer[MAX_FILE_SIZE];
  size_t position = 0;

  // int file_valid = false;

  while (true) {
    size_t bytes_read = fread(buffer, sizeof(char), sizeof(buffer), fp);
    for (size_t i = 0; i < bytes_read; i++) {

      object_buffer[position] = buffer[i];
      position++;
      if (buffer[i] == sep) {

        object_buffer[position] = '\0';

        position = 0;

        object_t *object = calloc(1, sizeof(object_t));
        if (NULL == object) {
          fprintf(stderr, "copy_file_to_hash: calloc failed\n");
          goto EXIT;
        }

        memcpy(object, object_buffer, sizeof(object_t));

        object->key = calloc(1, object->key_len + 1);
        if (NULL == object->key) {
          fprintf(stderr, "copy_file_to_hash: calloc failed\n");
          goto KEY_ERR;
        }

        object->value = calloc(1, object->value_len + 1);
        if (NULL == object->value) {
          fprintf(stderr, "copy_file_to_hash: calloc failed\n");
          goto VALUE_ERR;
        }

        object->owner = calloc(1, object->owner_len + 1);
        if (NULL == object->owner) {
          fprintf(stderr, "copy_file_to_hash: calloc failed\n");
          goto OWNER_ERR;
        }

        position += sizeof(uint16_t) * 3;
        position += sizeof(uint8_t);

        memcpy(object->key, object_buffer + position, object->key_len);

        position += object->key_len;

        memcpy(object->value, object_buffer + position, object->value_len);

        position += object->value_len;

        memcpy(object->owner, object_buffer + position, object->owner_len);

        position += object->owner_len;

        void *p_object = hash_table_lookup(p_ht, object->key);
        if (NULL != p_object) {
          goto INSERT_ERR;
        }

        int insert_success = hash_table_insert(p_ht, object->key, object);
        if (FAIL_CODE == insert_success) {
          fprintf(stderr, "copy_file_to_hash: insert failed\n");
          goto INSERT_ERR;
        }
        goto ALL_PASS;

      INSERT_ERR:
        free(object->owner);
        object->owner = NULL;
      OWNER_ERR:
        free(object->value);
        object->value = NULL;
      VALUE_ERR:
        free(object->key);
        object->key = NULL;
      KEY_ERR:
        free(object);
        object = NULL;
      ALL_PASS:
        position = 0;
      }
    }
    break;
  }

  ret_code = SUCCESS_CODE;
EXIT:
  return ret_code;
} /* end load_file_to_hash */

int write_hash_to_file(FILE *fp, hash_table_t *p_ht) {
  int ret_code = FAIL_CODE;
  if (NULL == fp) {
    fprintf(stderr, "write_hash_to_file: fp is NULL\n");
    goto EXIT;
  }
  if (NULL == p_ht) {
    fprintf(stderr, "write_hash_to_file: p_ht is NULL\n");
    goto EXIT;
  }

  char sep = '\x1E';

  for (uint32_t i = 0; i < p_ht->size; i++) {
    if (p_ht->elements[i] != NULL) {
      entry *tmp = p_ht->elements[i];
      while (NULL != tmp) {
        {
          object_t *tmp_obj = hash_table_lookup(p_ht, tmp->key);
          if (NULL != tmp_obj) {
            fwrite(&tmp_obj->key_len, sizeof(uint16_t), 1, fp);
            fwrite(&tmp_obj->value_len, sizeof(uint16_t), 1, fp);
            fwrite(&tmp_obj->owner_len, sizeof(uint16_t), 1, fp);
            fwrite(&tmp_obj->owner_permissions, sizeof(uint8_t), 1, fp);
            fwrite(tmp_obj->key, sizeof(char), tmp_obj->key_len, fp);
            fwrite(tmp_obj->value, sizeof(char), tmp_obj->value_len, fp);
            fwrite(tmp_obj->owner, sizeof(char), tmp_obj->owner_len, fp);
            fwrite(&sep, sizeof(uint8_t), 1, fp);
          }
        }
        tmp = tmp->next;
      }
    }
  }
  ret_code = SUCCESS_CODE;

EXIT:
  return ret_code;
} /* write_hash_to_file */

int dump_keys_to_file(char *dump_file_name, hash_table_t *p_ht) {
  int ret_code = FAIL_CODE;
  if (NULL == dump_file_name) {
    fprintf(stderr, "dump_keys_to_file: dump_file is NULL\n");
    goto EXIT;
  }
  if (NULL == p_ht) {
    fprintf(stderr, "dump_keys_to_file: p_ht is NULL\n");
    goto EXIT;
  }
  FILE *dump_file = fopen(dump_file_name, "wb+");

  char new_line = '\n';
  for (uint32_t i = 0; i < p_ht->size; i++) {
    if (p_ht->elements[i] != NULL) {
      entry *tmp = p_ht->elements[i];
      pthread_mutex_lock(&p_ht->hash_lock);
      while (NULL != tmp) {
        {
          object_t *tmp_obj = hash_table_lookup(p_ht, tmp->key);
          if (NULL != tmp_obj) {
            fwrite(tmp_obj->key, sizeof(char), tmp_obj->key_len - ONE_BYTE_SIZE,
                   dump_file);
            fwrite(&new_line, sizeof(uint8_t), 1, dump_file);
          }
        }
        tmp = tmp->next;
      }
      pthread_mutex_unlock(&p_ht->hash_lock);
    }
  }
  fclose(dump_file);
  ret_code = SUCCESS_CODE;
EXIT:
  return ret_code;
} /* dump_keys_to_file() */

/*** end of file ***/
