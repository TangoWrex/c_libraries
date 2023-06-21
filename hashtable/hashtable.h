/* @file hashtable.h
 * Void hash table function prototypes
 */

#ifndef HSH_TABLE_H
#define HSH_TABLE_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

/**
 * @brief hash function that will be used to hash the key
 * @param const char* to be stored
 * @param size_t size of the string
 */

typedef uint64_t hashfunction(const char *, size_t);

// function pointer for cleaning up object
typedef void cleanup_function(void * obj);

/**
 * @brief struct that will hold the hash table
 */
typedef struct _hash_table hash_table_t;

/**
 * @brief Creates the hash table of size amount by using hashfunction
 * @param uint32_t size of the hash table
 * @param hashfunction* pointer to the hash function
 * @param cleanup_function* pointer to the cleanup function
 * @return SUCCESS_CODE on success
 * @return FAILURE_CODE on failure
 */
hash_table_t * hash_table_create(uint32_t           size,
                                 hashfunction *     hf,
                                 cleanup_function * cleanup);

/**
 * @brief Deletes the hash table
 * @param hash_table* pointer to the hash table
 */
void hash_table_destroy(hash_table_t * ht);

/**
 * @brief Prints the hash table
 * @param hash_table* pointer to the hash table to print
 */
void hash_table_print(hash_table_t * ht);

/**
 * @brief inserts they key and object into the hash table
 * @param hash_table* pointer to the hash table
 * @param const char* key to be stored
 * @param void* object to be stored (void * allows for the function to accept any type of
 * object)
 */
int hash_table_insert(hash_table_t * ht, const char * key, void * obj);

/**
 * @brief Looks up the key in the hash table
 * @param hash_table* pointer to the hash table
 * @param const char key to searched for
 */
void * hash_table_lookup(hash_table_t * ht, const char * key);
// void *hash_table_lookup(hash_table *ht, const char *key, size_t keylen);

/**
 * @brief Finds an object and deletes it from the hash table
 * @param hash_table* pointer to the hash table
 * @param const char key to searched for
 */
void * hash_table_delete(hash_table_t * ht, const char * key);

/**
 * @brief hash function that will be used to hash the key
 * @param char * name of the key
 * @param size_t length of the key
 * @return uint64_t hash value
 */
uint64_t hash_function(const char * key, unsigned long length);

/**
 * @brief function that will be used to clean up the object
 *
 * @param hash_table_t hasthable
 * @param const char* p_key object to remove from the hashtable
 * @return void* to an object the caller must free
 */
void * hash_table_remove(hash_table_t * p_ht, const char * key);

/**
 * @brief Read the File data and if it's valid creates a hash table of objects
 *
 * @param fp to binary file
 * @param hash_table_t  hashtable to build
 * @return int SUCCESS_CODE on success
 * @return int FAILURE_CODE on failure
 */
int load_file_to_hash(FILE * fp, hash_table_t * p_ht);

/**
 * @brief Write the hash table to a binary file
 *
 * @param fp file to write to
 * @param hash_table_t hashtable to write
 * @return int SUCCESS_CODE on success
 * @return int FAILURE_CODE on failure
 */
int write_hash_to_file(FILE * fp, hash_table_t * p_ht);

/**
 * @brief Functions used by the 'keys' function. This creates a string to be sent as the
 * data
 *
 * @param hash_table_t hasthtable to get the items from
 * @param char *output string to be sent as the data
 * @return int
 */
int copy_keys_to_string(hash_table_t * p_ht, char * output);

/**
 * @brief search hashtable for all matching keys or keys that contain the string in them.
 *
 * @param hash_table_t
 * @param char *store_keys
 * @param char * key_to_find
 * @param int user_permission  to check against the key
 * @return int
 */
int return_all_matching_keys(hash_table_t * p_ht,
                             char *         store_keys,
                             char *         key_to_find,
                             int            user_permission);

/**
 * @brief dump all objects from the hashtable into a binary file
 *
 * @param char * dump_file_name
 * @param hash_table_t p_ht
 * @return int
 */
int dump_keys_to_file(char * dump_file_name, hash_table_t * p_ht);

#endif /* HSH_TABLE_H */
