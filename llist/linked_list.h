/* @file linked_list.h
 * @brief This file handles the linked list that will hold all of our client
 * nodes this file also confirms the client account exists and logs them in
 *
 */

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "main_server.h"
#include "client.h"

/**
 * @brief Struct that defines pointer to head node and pthread lock
 */
typedef struct llist_t llist_t;

/**
 * @brief Allocates linked-list
 *
 * @return llist_t* On success
 * @return NULL on failure
 */
llist_t * llist_init();

/**
 * @brief Function to delete linked list and free memory
 *
 * @param p_llist Linked-list to delete
 * @return SUCCESS_CODE on success
 * @return FAILURE_CODE on failure
 */
int llist_delete(llist_t * p_llist);

/**
 * @brief Function to delete linked list and free memory
 *
 * @param p_llist Linked-list to delete
 * @return SUCCESS_CODE on success
 * @return FAILURE_CODE on failure
 */
int llist_add_client(llist_t * p_llist,
                     char *    p_username,
                     char *    p_password,
                     int       session_id,
                     int       req_privilege,
                     int       user_privilege,
                     int       sock);

/**
 * @brief Given a username, the llist will return a client if they exist
 *
 * @param llist Linked-list search
 * @param char * username to search for
 * @return client on success
 * @return NULL on failure
 */
client_t * llist_find_client(llist_t * p_llist, char * p_username);

/**
 * @brief given the session id return true or false if that id was found
 *
 * @param llist Linked-list to search
 * @param int  session_id to search for
 * @param int *privilege_level to store the privilege level of the user
 * @return client on success
 * @return NULL on failure
 */
int llist_search_session_id(llist_t * llist, int session_id, int * privilege_level);

/**
 * @brief when our timeout is hit for a socket then
 * we have to reset the session_id because their session is
 * no longer valid. the active clients will be given a socket.
 * when they disconnect this will be set to default value
 *
 * @param llist to search
 * @param int socket fd to search for
 * @return SUCCESS_CODE on success
 * @return FAILURE_CODE on failure
 */
int llist_reset_timeout(llist_t * llist, int sock);

/**
 * @brief Find a matching client with this socket and attempt to close it.
 * They can keep their session ID because timeout has not occured yet.
 *
 * @param llist to search
 * @param int socket fd to search for
 * @return SUCCESS_CODE on success
 * @return FAILURE_CODE on failure
 */
int llist_close_sock(llist_t * llist, int sock);

/**
 * @brief Adds void* to linked-list as a queue
 *
 * @param llist Linked-list to enqueue() to
 * @param client_t to add to linked list
 * @return SUCCESS_CODE on success
 * @return FAILURE_CODE on failure
 */
int llist_enqueue(llist_t * llist, client_t * client);

/**
 * @brief Removes client* from linked-list
 *
 * @param llist Linked-list to dequeue() from
 * @param data Void** to store data in
 * @return a void type item from the llist
 */
client_t * llist_dequeue(llist_t * llist);

/**
 * @brief iterates over each node in the linked list and prints the data
 *
 * @param llist Linked-list to iterate through
 * @return void
 */
void llist_print(llist_t * llist);

/**
 * @brief Adds client* to linked-list as a stack
 *
 * @param llist Linked-list to push() to
 * @param data Void* for data to be added
 * @return SUCCESS_CODE on success
 * @return FAILURE_CODE on failure
 */
int llist_push(llist_t * llist, client_t * client);

/**
 * @brief Removes client * from linked-list as a stack
 *
 * @param llist Linked-list to pop() from
 * @return a void type item from the llist
 */
client_t * llist_pop(llist_t * llist);

/**
 * @brief Checks if linked-list is empty
 *
 * @param list Linked-list to check
 * @return SUCCESS_CODE on not empty
 * @return FAILURE_CODE on empty
 */
int llist_is_empty(llist_t * llist);

/**
 * @brief Searches for a client and removes it from the list
 *
 * @param llist search for user
 * @param char *username to search for
 * @return SUCCESS_CODE on success
 * @return FAILURE_CODE on failure
 */
int llist_delete_client(llist_t * llist, char * p_username);

/**
 * @brief Searches for a client and returns the username
 *
 * @param llist_t p_llist llist to search through
 * @param int session_id to search for
 * @return char* username on Success
 * @return NULL on failure
 */
char * llist_get_client_name(llist_t * p_llist, int session_id);

#endif /* LINKED_LIST_H */
