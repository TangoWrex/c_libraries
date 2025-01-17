/** @file linked_list.c
 *
 * @brief This linked list is used to hold nodes which contain my clients.
 * client_node.c handles the client functions
 *
 *
 */
#include "linked_list.h"
#include "client.h"

/*
 * @brief Node in the linked list to the hold the client data
 */
struct node_t
{
    struct node_t * next;
    client_t *      client;
};

/*
 * @brief Linked list which also holds a lock to protect the list
 */
struct llist_t
{
    int             size;
    struct node_t * head;
    struct node_t * tail;
    pthread_mutex_t llist_lock;
};

llist_t * llist_init()
{

    llist_t * llist = calloc(1, sizeof(*llist));
    if (NULL == llist)
    {
        fprintf(stderr, "bad data *llist_create*");
        goto ERROR;
    }
    llist->size = 0;
    llist->head = NULL;
    llist->tail = NULL;
    pthread_mutex_init(&llist->llist_lock, NULL);
ERROR:
    return llist;
} /* llist_init() */

int llist_delete(llist_t * p_llist)
{
    int llist_delete_success = FAIL_CODE;
    if (NULL == p_llist)
    {
        fprintf(stderr, "bad data *llist_delete*");
        // llist_delete_success = false;
        goto EXIT;
    }
    pthread_mutex_lock(&p_llist->llist_lock);
    {
        struct node_t * node = p_llist->head;
        struct node_t * temp = NULL;

        while (node)
        {
            temp = node;
            node = node->next;
            client_delete(temp->client);
            free(temp->client);
            temp->client = NULL;
            temp->next   = NULL;
            free(temp);
            temp = NULL;
        }
    }
    pthread_mutex_unlock(&p_llist->llist_lock);
    pthread_mutex_destroy(&p_llist->llist_lock);
    p_llist->head = NULL;
    p_llist->tail = NULL;
    free(p_llist);
    p_llist              = NULL;
    llist_delete_success = SUCCESS_CODE;
EXIT:
    return llist_delete_success;
} /* llist_delete() */

int llist_enqueue(llist_t * p_llist, client_t * p_client)
{
    int enqueue_success = FAIL_CODE;
    if ((NULL == p_llist) || (NULL == p_client))
    {
        fprintf(stderr, "bad data *llist_enqueue*");
        // enqueue_success = false;
        goto EXIT;
    }
    struct node_t * node = calloc(1, sizeof(*node));
    if (NULL == node)
    {
        fprintf(stderr, "bad data *llist_enqueue*");
        goto EXIT;
    }

    node->client = p_client;
    node->next   = NULL;
    pthread_mutex_lock(&p_llist->llist_lock);
    {
        if (p_llist->tail)
        {

            p_llist->tail->next = node;
        }
        else
        {
            p_llist->head = node;
        }
        p_llist->tail = node;
    }
    pthread_mutex_unlock(&p_llist->llist_lock);
    enqueue_success = SUCCESS_CODE;
EXIT:
    return enqueue_success;
} /* llist_enqueue() */

client_t * llist_find_client(llist_t * p_llist, char * p_username)
{
    client_t * client = NULL;
    if (NULL == p_llist)
    {
        fprintf(stderr, "bad data *llist_find_client*");
        goto EXIT;
    }
    if (NULL == p_username)
    {
        fprintf(stderr, "bad data *llist_find_client*");
        goto EXIT;
    }

    pthread_mutex_lock(&p_llist->llist_lock);
    {
        struct node_t * node = p_llist->head;
        while (node)
        {
            // After I get the username from our client
            // I want to compare it to the username we are looking for
            char * username = client_get_username(node->client);
            if (NULL == username)
            {
                fprintf(stderr, "client_get_username returned NULL");
                goto ERR;
            }
            int match = strcmp(p_username, username);
            if (0 == match)
            {
                client = node->client;
                break;
            }
            node = node->next;
        }
    }
ERR:
    pthread_mutex_unlock(&p_llist->llist_lock);

EXIT:
    return client;
} /* llist_find_client() */

int llist_search_session_id(llist_t * p_llist, int session_id, int * p_privilege_level)
{
    int ret_code = FAIL_CODE;
    if (NULL == p_llist)
    {
        fprintf(stderr, "bad data *llist_search_session_id*");
        goto EXIT;
    }
    if ((DEFAULT_SESSION_ID >= session_id) || (MAX_SESSION_ID < session_id))
    {
        fprintf(stderr, "session_id is invalid\n");
        goto EXIT;
    }
    if (NULL == p_privilege_level)
    {
        fprintf(stderr, "privilege_level is NULL\n");
        goto EXIT;
    }
    pthread_mutex_lock(&p_llist->llist_lock);
    {
        // iterate throught the llist and if our int pointer matches
        struct node_t * node = p_llist->head;
        while (node)
        {
            // After I get the username from our client
            // I want to compare it to the username we are looking for
            // set return code to success and break
            if (session_id == node->client->session_id)
            {
                *p_privilege_level = node->client->privilege;
                ret_code           = SUCCESS_CODE;
                break;
            }
            node = node->next;
        }
    }
    pthread_mutex_unlock(&p_llist->llist_lock);
EXIT:
    return ret_code;
} /*llist_search_session_id()*/

int llist_push(llist_t * p_llist, client_t * p_client)
{
    int push_success = FAIL_CODE;
    if (NULL == p_llist)
    {
        fprintf(stderr, "bad data *llist_push*");
        goto EXIT;
    }
    if (NULL == p_client)
    {
        fprintf(stderr, "bad data *llist_push*");
        goto EXIT;
    }
    struct node_t * node = calloc(1, sizeof(*node));
    if (NULL == node)
    {
        fprintf(stderr, "bad data *llist_push*");
        goto EXIT;
    }
    node->client = p_client;
    pthread_mutex_lock(&p_llist->llist_lock);
    {
        node->next    = p_llist->head;
        p_llist->head = node;
    }
    pthread_mutex_unlock(&p_llist->llist_lock);
    push_success = SUCCESS_CODE;
EXIT:
    return push_success;
} /* llist_push() */

client_t * llist_dequeue(llist_t * p_llist)
{
    client_t * client = NULL;
    if (NULL == p_llist)
    {
        fprintf(stderr, "bad data *llist_dequeue*");
        goto EXIT;
    }
    if (NULL != p_llist->head)
    {
        struct node_t * temp = p_llist->head;
        p_llist->head        = temp->next;
        client               = temp->client;

        if (NULL == p_llist->head)
        {
            p_llist->tail = NULL;
        }
        temp->next   = NULL;
        temp->client = NULL;
        free(temp);
        temp = NULL;
        p_llist->size--;
    }
EXIT:
    return client;
} /* llist_dequeue() */

void llist_print(llist_t * p_llist)
{
    if (NULL == p_llist)
    {
        fprintf(stderr, "bad data *llist_print*");
        goto EXIT;
    }
    struct node_t * node = p_llist->head;
    while (node)
    {
        print_client_node(node->client);
        node = node->next;
    }
EXIT:
    return;
} /* llist_print() */

client_t * llist_pop(llist_t * p_llist)
{
    return llist_dequeue(p_llist);
} /* llist_pop() */

int llist_add_client(llist_t * p_llist,
                     char *    p_username,
                     char *    p_password,
                     int       session_id,
                     int       req_privilege,
                     int       user_privilege,
                     int       sock)
{
    int add_client_success = FAIL_CODE;
    if (NULL == p_llist)
    {
        fprintf(stderr, "bad data *llist_add_client*");
        goto EXIT;
    }

    int valid_args =
        validate_client_args(p_username, p_password, session_id, req_privilege, sock);
    if (FAIL_CODE == valid_args)
    {
        goto EXIT;
    }

    client_t * user =
        client_init(p_username, p_password, DEFAULT_SESSION_ID, user_privilege, SOCK_MIN);
    if (NULL == user)
    {
        fprintf(stderr, "client_init failed\n");
        goto EXIT;
    }

    int enq_success = llist_enqueue(p_llist, user);
    if (FAIL_CODE == enq_success)
    {
        fprintf(stderr, "client_enqueue failed\n");
        goto EXIT;
    }
    add_client_success = SUCCESS_CODE;
EXIT:
    return add_client_success;
} /* llist_add_client() */

int llist_reset_timeout(llist_t * llist, int sock)
{
    int ret_code = FAIL_CODE;
    if (NULL == llist)
    {
        fprintf(stderr, "bad data *llist_reset_timeout*");
        goto EXIT;
    }
    // If our timeout occurs we need to iterate through the list and close the
    // socket and reset it to the default value of 0
    pthread_mutex_lock(&llist->llist_lock);
    {
        struct node_t * node = llist->head;
        while (node)
        {
            if (node->client->client_sock == sock)
            {
                node->client->is_logged_in = false;
                node->client->session_id   = DEFAULT_SESSION_ID;
                ret_code                   = SUCCESS_CODE;
                break;
            }
            node = node->next;
        }
    }
    pthread_mutex_unlock(&llist->llist_lock);
EXIT:
    return ret_code;
} /*llist_reset_timeout()*/

int llist_close_sock(llist_t * p_llist, int sock)
{
    int ret_code = FAIL_CODE;
    if (NULL == p_llist)
    {
        fprintf(stderr, "bad data *llist_reset_timeout*");
        goto EXIT;
    }
    // when a client disconnects I want to ensure that their socket gets closed
    // if our server recieves a SIGINT or SIGTERM we need to iterate through
    pthread_mutex_lock(&p_llist->llist_lock);
    {
        struct node_t * node = p_llist->head;
        while (node)
        {
            if (node->client->client_sock == sock)
            {
                // when we find a match for our socket
                // we will close it and reset it to the default value
                if ((SOCK_MIN < node->client->client_sock) &&
                    (SOCK_MAX > node->client->client_sock))
                {
                    close(node->client->client_sock);
                    ret_code = SUCCESS_CODE;
                }
                // if the socket is already closed we will just reset it to the
                // to the default value of 0
                node->client->client_sock = SOCK_MIN;
                break;
            }
            node = node->next;
        }
    }
    pthread_mutex_unlock(&p_llist->llist_lock);
EXIT:
    return ret_code;
} /*llist_reset_timeout()*/

int llist_delete_client(llist_t * p_llist, char * p_username)
{
    // Used for the delete_account function of our program
    int ret_code    = FAIL_CODE;
    int match_found = 0;
    if (NULL == p_llist)
    {
        fprintf(stderr, "bad data *llist_delete_client*");
        goto EXIT;
    }
    if (NULL == p_username)
    {
        fprintf(stderr, "bad data *llist_delete_client*");
        goto EXIT;
    }
    pthread_mutex_lock(&p_llist->llist_lock);
    {
        // Iterate through the linked list and find the client node to delete
        struct node_t * node = p_llist->head;
        // The prev_node is needed so we can copy the next pointer to the node
        // after the node being deleted
        struct node_t * prev_node = NULL;
        while (node)
        {
            int match = strncmp(p_username, node->client->name, strlen(p_username));
            if (0 == match)
            {
                // We found the client node to delete
                if (prev_node)
                {
                    // The client node is not the head of the linked list
                    prev_node->next = node->next;
                }
                else
                {
                    // The client node is the head of the linked list
                    p_llist->head = node->next;
                }
                if (node == p_llist->tail)
                {
                    // The client node is the tail of the linked list
                    p_llist->tail = prev_node;
                }
                client_delete(node->client);
                free(node->client);
                node->client = NULL;
                free(node);
                node        = NULL;
                match_found = MATCH_FOUND;
                break;
            }
            // after we have checked the current node we need to update the
            // prev_node and node pointers
            prev_node = node;
            node      = node->next;
        }
    }
    pthread_mutex_unlock(&p_llist->llist_lock);
    if (0 == match_found)
    {
        fprintf(stderr, "No match found for %s\n", p_username);
        goto EXIT;
    }
    ret_code = SUCCESS_CODE;
EXIT:
    return ret_code;
} /*llist_delete_client()*/

char * llist_get_client_name(llist_t * p_llist, int session_id)
{
    char * username = NULL;
    if (NULL == p_llist)
    {
        fprintf(stderr, "bad data *llist_get_client_name*");
        goto EXIT;
    }
    if ((DEFAULT_SESSION_ID > session_id) || (MAX_SESSION_ID < session_id))
    {
        fprintf(stderr, "bad data *llist_get_client_name*");
        goto EXIT;
    }
    username = calloc(1, MAX_USERNAME);
    if (NULL == username)
    {
        fprintf(stderr, "calloc error\n");
        goto EXIT;
    }

    pthread_mutex_lock(&p_llist->llist_lock);
    {
        // iterate throught the llist and if our int pointer matches
        struct node_t * node = p_llist->head;
        while (node)
        {
            // After I get the username from our client
            // I want to compare it to the username we are looking for
            if (session_id == node->client->session_id)
            {
                strncpy(username, node->client->name, strlen(node->client->name));
                break;
            }
            node = node->next;
        }
    }
    pthread_mutex_unlock(&p_llist->llist_lock);
EXIT:
    return username;
} /* llist_get_client_name() */

/*** end of file ***/
