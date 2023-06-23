/** @file threadpool.c
 *
 * @brief This file contains the innerworkings of the threadpool. It is
 * responsible for creation an destruction of the threadpool, handling the
 * queue, and signaling threads there is work to be done and when to shutdown.
 *
 */

#include "thread_pool.h"
#include <signal.h>

#define POOL_SIZE_MIN 1
#define FAIL_CODE     -1
#define SUCCESS_CODE  1

extern volatile sig_atomic_t shutdown_flag;

threadpool_t * thpool_init(int pool_size)
{
    threadpool_t * pool = NULL;

    if (POOL_SIZE_MIN > pool_size)
    {
        fprintf(stderr, "Pool size too small\n");
        goto EXIT;
    }
    if (MAX_CONNECTIONS < pool_size)
    {
        fprintf(stderr, "Pool size too large\n");
        goto EXIT;
    }
    pool = calloc(1, sizeof(threadpool_t));
    if (NULL == pool)
    {
        fprintf(stderr, "Could not allocate memory for thread pool\n");
        goto EXIT;
    }

    pool->queue_size = 0;
    pool->head       = NULL;
    pool->tail       = NULL;
    pool->shutdown   = false;

    // int hash_success = create_tpool_hashtable(pool);
    // if (FAIL_CODE == hash_success)
    // {
    //     fprintf(stderr, "Could not create hash table\n");
    //     goto PATH_ERROR;
    // }

    pool->pool_size = pool_size;
    pool->threads   = calloc(pool_size, sizeof(pthread_t));
    if (NULL == pool->threads)
    {
        fprintf(stderr, "Could not allocate memory for threads\n");
        goto PATH_ERROR;
    }
    if (pthread_mutex_init(&(pool->lock), NULL) != 0)
    {
        fprintf(stderr, "Could not initialize mutex\n");
        goto THREAD_ERROR;
    }
    if (pthread_mutex_init(&(pool->file_lock), NULL) != 0)
    {
        fprintf(stderr, "Could not initialize mutex\n");
        goto THREAD_ERROR;
    }
    if (pthread_cond_init(&(pool->not_empty), NULL) != 0)
    {
        fprintf(stderr, "Could not initialize cond_t\n");
        goto THREAD_ERROR;
    }
    if (pthread_cond_init(&(pool->empty), NULL) != 0)
    {
        fprintf(stderr, "Could not initialize cond_t\n");
        goto THREAD_ERROR;
    }
    for (int i = 0; i < pool_size; i++)
    {
        if (pthread_create(&(pool->threads[i]), NULL, thread_function, (void *)pool) != 0)
        {
            fprintf(stderr, "Could not create thread pool\n");
            thpool_destroy(pool);
            goto EXIT;
        }
    }
    goto EXIT;
THREAD_ERROR:
    free(pool->threads);
    pool->threads = NULL;
PATH_ERROR:
    free(pool);
    pool = NULL;
EXIT:
    return pool;
} /* thpool_init() */

int enqueue_job(threadpool_t * p_pool, int socket)
{
    int enqueue_success = FAIL_CODE;
    if (0 > socket)
    {
        fprintf(stderr, "invalid args\n");
        goto EXIT;
    }
    if (NULL == p_pool)
    {
        fprintf(stderr, "pool is null\n");
        goto EXIT;
    }
    job_t * newjob = calloc(1, sizeof(job_t));
    if (NULL == newjob)
    {
        fprintf(stderr, "Could not allocate memory for new job\n");
        goto EXIT;
    }
    pthread_mutex_lock(&(p_pool->lock));
    newjob->socket = socket;
    newjob->next   = NULL;
    if (NULL == p_pool->tail)
    {
        p_pool->head = newjob;
    }
    else
    {
        p_pool->tail->next = newjob;
    }
    p_pool->tail = newjob;
    p_pool->queue_size++;
    pthread_mutex_unlock(&(p_pool->lock));
    pthread_cond_signal(&(p_pool->not_empty));
    enqueue_success = SUCCESS_CODE;
EXIT:
    return enqueue_success;
} /* enqueue_job() */

int dequeue_all(threadpool_t * p_pool)
{
    int dequeue_all_success = FAIL_CODE;
    if (NULL == p_pool)
    {
        fprintf(stderr, "Invalid arguments to dequeue_all\n");
        goto EXIT;
    }
    pthread_mutex_lock(&(p_pool->lock));
    for (int i = 0; i < p_pool->queue_size; i++)
    {
        job_t * job  = p_pool->head;
        p_pool->head = p_pool->head->next;
        close(job->socket);
        free(job);
        job = NULL;
    }
    p_pool->queue_size = 0;
    p_pool->head       = NULL;
    p_pool->tail       = NULL;
    pthread_mutex_unlock(&(p_pool->lock));
    dequeue_all_success = SUCCESS_CODE;
EXIT:
    return dequeue_all_success;
} /* dequeue_all() */

void * thread_function(void * arg)
{
    if (NULL == arg)
    {
        fprintf(stderr, "Invalid arguments to thread_function\n");
        return NULL;
    }
    threadpool_t * p_pool = (threadpool_t *)arg;

    job_t * job = NULL;

    while (!shutdown_flag)
    {
        p_pool->queue_size = p_pool->queue_size;
        pthread_mutex_lock(&(p_pool->lock));
        while (0 == p_pool->queue_size)
        {
            pthread_cond_wait(&(p_pool->not_empty), &(p_pool->lock));
            if (p_pool->shutdown)
            {
                pthread_mutex_unlock(&(p_pool->lock));
                goto EXIT;
            }
        }
        job = p_pool->head;
        p_pool->queue_size--;
        if (p_pool->queue_size == 0)
        {
            p_pool->head = NULL;
            p_pool->tail = NULL;
        }
        else
        {
            p_pool->head = job->next;
        }
        pthread_mutex_unlock(&(p_pool->lock));

        execute_job(job, p_pool);
        job = NULL;
    }
EXIT:
    return NULL;
} /* thread_function() */

int thpool_destroy(threadpool_t * p_pool)
{
    int err_code = FAIL_CODE;
    if (NULL == p_pool)
    {
        fprintf(stderr, "Invalid arguments to destroy_tp_pool\n");
        goto EXIT;
    }

    p_pool->shutdown = true;

    // hash_table_print(p_pool->hash_table);

    // write hash table to file
    pthread_mutex_lock(&(p_pool->file_lock));
    // execute any file io or data base io here
    pthread_mutex_unlock(&(p_pool->file_lock));

    // destroy hash function

    int dq_success = dequeue_all(p_pool);
    if (FAIL_CODE == dq_success)
    {
        fprintf(stderr, "dequeue all failed\n");
    }

    pthread_mutex_lock(&(p_pool->lock));
    while (p_pool->queue_size != 0)
    {
        pthread_cond_wait(&(p_pool->empty), &(p_pool->lock));
    }
    pthread_cond_broadcast(&(p_pool->not_empty));
    pthread_mutex_unlock(&(p_pool->lock));
    for (int i = 0; i < p_pool->pool_size; i++)
    {
        pthread_cancel(p_pool->threads[i]);
        pthread_join(p_pool->threads[i], NULL);
    }
    free(p_pool->threads);
    p_pool->threads = NULL;
    pthread_mutex_destroy(&(p_pool->file_lock));
    pthread_mutex_destroy(&(p_pool->lock));
    pthread_cond_destroy(&(p_pool->not_empty));
    pthread_cond_destroy(&(p_pool->empty));
    free(p_pool);
    p_pool   = NULL;
    err_code = SUCCESS_CODE;
EXIT:
    return err_code;
} /* thpool_destroy() */

/*** end of file ***/
